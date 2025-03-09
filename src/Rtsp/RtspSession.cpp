#include "RtspSession.h"
#include "HttpRequest.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include"common.h"
#include "Logging.h"

#include <cinttypes>
#include <iomanip>
#include <atomic>
#include <memory>
#include <arpa/inet.h>


RtspSession::RtspSession() {
}

RtspSession::~RtspSession() {
}

void RtspSession::onWholeRtspPacket(const RtspRequest& request) {
	_cseq = atoi(request.GetRequestValue("CSeq").c_str());
	if (_content_base.empty() && request.method() != "GET") {
		_content_base = request.url();
		_param_strs = request.request_params();
		parse(request.url());
		_schema = "rtsp";
	}
	if (request.method() == "OPTIONS")
		handleOptions(request);
	else if (request.method() == "DESCRIBE")
		handleDescribe(request);
	else if (request.method() == "SETUP")
		handleSetup(request);
	else if (request.method() == "PLAY")
		handlePlay(request);
	else
		Send(getRtspResponse("403 Forbidden"));
}

void RtspSession::handleOptions(const RtspRequest& request) {
	auto resp = getRtspResponse("200 OK", { "Public" , "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, ANNOUNCE, RECORD" });
	Send(resp);
}
void RtspSession::handleDescribe(const RtspRequest& request) {

	_stream = std::make_shared<RtspMediaStream>(_streamid);

	Track::Ptr& trackRef = _stream->getMediaTrack();
	trackRef->_seq = 1;
	trackRef->_ssrc = generateRandomInt<uint32_t>();
	trackRef->_time_stamp = 0;
	trackRef->_type = 0;
	trackRef->_samplerate = 90000;
	trackRef->_pt = 96;
	trackRef->_interleaved = 2 * trackRef->_type;

	auto ready = _stream->createFromEs(trackRef->_pt, trackRef->_samplerate);
	_sessionid = makeRandStr(12);
	if (!ready) {
		Send(getRtspResponse("404 Stream Not Found", { "Connection","Close" }));
		return;
	}

	_rtcp_context.clear();
	_rtcp_context.emplace_back(std::make_shared<RtcpContextForSend>());
	
	Send(getRtspResponse("200 OK",
		{ "Content-Base", _content_base + "/",
		 "x-Accept-Retransmit","our-retransmit",
		 "x-Accept-Dynamic-Rate","1"
		}, _stream->getSdp()));

}
void RtspSession::handleSetup(const RtspRequest& request) {
	Track::Ptr& trackRef = _stream->getMediaTrack();
	//trackRef->_inited = true; 

	if (_rtp_type == eRtpType::RTP_Invalid) {
		auto transport = request.GetRequestValue("Transport");
		if (transport.find("TCP") != std::string::npos) {
			_rtp_type = eRtpType::RTP_TCP;
		}
		else if (transport.find("multicast") != std::string::npos) {
			_rtp_type = eRtpType::RTP_MULTICAST;
		}
		else {
			_rtp_type = eRtpType::RTP_UDP;
		}
	}

	switch (_rtp_type) {
	case eRtpType::RTP_TCP: {
		Send(getRtspResponse("200 OK",
			{ "Transport",(std::string("RTP/AVP/TCP;unicast;") + "interleaved=") +
							std::to_string((int)trackRef->_interleaved) + "-" +
							std::to_string((int)trackRef->_interleaved + 1) + ";" +
							"ssrc=" + trackRef->getSSRC(),
				"x-Transport-Options","late-tolerance=1.400000",
				"x-Dynamic-Rate","1"
			}));
	}
		break;
	case eRtpType::RTP_UDP: {}
		break;
	case eRtpType::RTP_MULTICAST: {}
		break;
	}

}

void RtspSession::handlePlay(const RtspRequest& request) {
	if(_stream->getMediaTrack() == nullptr || request.GetRequestValue("Session") != _sessionid) {
		Send(getRtspResponse("454 Session Not Found", { "Connection","Close" }));
		return;
	}
	if (!_stream) {
		Send(getRtspResponse("404 Stream Not Found", { "Connection","Close" }));
		return;
	}
	auto Scale = request.GetRequestValue("Scale");
	auto Range = request.GetRequestValue("Range");
	std::map<std::string, std::string> resMap;
	if (!Scale.empty()) {
	}
	if (!Range.empty()) {
	}
	Track::Ptr& trackRef = _stream->getMediaTrack();
	std::string rtp_info;
	rtp_info = "url=" + _content_base + ";" +
		"seq=" + std::to_string(trackRef->_seq) + ";" +
		"rtptime=" + std::to_string((int)(trackRef->_time_stamp * (trackRef->_samplerate / 1000))) + ",";
	rtp_info.pop_back();
	resMap.emplace("RTP-Info", rtp_info);
	resMap.emplace("Range", std::string("npt=") + std::to_string(trackRef->_time_stamp / 1000.0));
	Send(getRtspResponse("200 OK", resMap));
	std::weak_ptr<RtspSession> rtsp_weak_self = std::dynamic_pointer_cast<RtspSession>(shared_from_this());
	auto play = [rtsp_weak_self](const RtpPacket::Ptr& packet) {
		auto strong_self = rtsp_weak_self.lock();
		if (!strong_self) {
			//对象已销毁
			return false;
		}
		strong_self->updateRtcpContext(packet);
		//LOG_INFO << "rtp seq: " << packet->getSeq() << " ,TimeStamp: " << packet->getStamp();
		strong_self->Send(packet);
		};
	_stream->setEncoderSendCB(play);
	auto stream = getStream();
	std::weak_ptr<RtspMediaStream> stream_weak_self = std::dynamic_pointer_cast<RtspMediaStream>(stream);
	auto conn = conn_weak_self.lock();
	if (!conn) {
		//对象已销毁
		return ;
	}
	_timer = conn->loop()->RunEvery(500, [stream_weak_self]() {
		auto strong_self = stream_weak_self.lock();
		if (!strong_self) {
			//对象已销毁
			return false;
		}
		strong_self->readFrame();
		return true;
	},TimeUnit::MILLISECONDS);

}

void RtspSession::handleTeardown(const RtspRequest& request) {
	Send(getRtspResponse("200 OK"));
}

void RtspSession::handlePause(const RtspRequest& request) {

}

void RtspSession::updateRtcpContext(const RtpPacket::Ptr& rtp) {
	auto& rtcp_ctx = _rtcp_context[0];
	rtcp_ctx->onRtp(rtp->getSeq(), rtp->getStamp(),/*rtp->ntp_stamp*/TimeStamp::Now().microseconds(), rtp->sample_rate, rtp->getData()->readablebytes() - RtpPacket::RtpTcpHeaderSize);
	auto& ticker = _rtcp_send_ticker;
	if (TimeStamp::Now() > TimeStamp::AddTime(ticker, 5, TimeUnit::SECONDS) || _send_sr_rtcp) {
		_rtcp_send_ticker = TimeStamp::Now();
		//确保在发送rtp前，先发送一次sender report rtcp(用于播放器同步音视频)
		_send_sr_rtcp = false;

		static auto send_rtcp = [](RtspSession* thiz,std::shared_ptr<Buffer> rtcp) {
			auto& track = thiz->_stream->getMediaTrack();
			//还需添加rtp over tcp header
			std::shared_ptr<Buffer> rtp_tcp = std::make_shared<Buffer>();
			std::string capacity(RtpPacket::RtpTcpHeaderSize, 0);
			rtp_tcp->Append(capacity.c_str(), RtpPacket::RtpTcpHeaderSize);
			auto ptr = rtp_tcp->Peek();
			ptr[0] = '$';
			ptr[1] = track->_interleaved + 1;
			ptr[2] = ((uint16_t)(rtcp->readablebytes()) >> 8) & 0xFF;
			ptr[3] = (uint16_t)(rtcp->readablebytes()) & 0xFF;
			rtcp->AppendPrepend(rtp_tcp->Peek(), rtp_tcp->readablebytes());
			thiz->Send(rtcp->RetrieveAllAsString());
		};

		auto ssrc = rtp->getSSRC();
		auto rtcp = rtcp_ctx->createRtcpSR(ssrc);
	
		auto rtcp_sdes = RtcpSdes::create({ "RtspServer" });
		rtcp_sdes->chunks.type = (uint8_t)SDES_TYPE::RTCP_SDES_CNAME;
		rtcp_sdes->chunks.ssrc = htonl(ssrc);
		send_rtcp(this,rtcp);
		send_rtcp(this, RtcpHeader::toBuffer(rtcp_sdes));
	}
}

void RtspSession::onRtpPacket(const char* data, size_t len) {
	uint8_t interleaved = data[1];
	if (interleaved % 2 != 0) {
		onRtcpPacket(data + RtpPacket::RtpTcpHeaderSize, len - RtpPacket::RtpTcpHeaderSize);
	}
}

void RtspSession::onRtcpPacket(const char* data, size_t len) {
	auto rtcp_arr = RtcpHeader::loadFromBytes((char*)data, len);
	for (auto &rtcp : rtcp_arr) {
		_rtcp_context[0]->onRtcp(rtcp);
	}
}

static std::string dateStr() {
	char buf[64];
	time_t tt = time(NULL);
	strftime(buf, sizeof buf, "%a, %b %d %Y %H:%M:%S GMT", gmtime(&tt));
	return buf;
}

std::string RtspSession::getRtspResponse(const std::string& res_code, const std::initializer_list<std::string>& header,
	const std::string& sdp, const std::string& protocol) {
	std::string key;
	std::map<std::string,std::string> header_map;
	int i = 0;
	for (auto& val : header) {
		if (++i % 2 == 0) {
			header_map.emplace(key, val);
		}
		else {
			key = val;
		}
	}
	return getRtspResponse(res_code, header_map, sdp, protocol);
}

std::string RtspSession::getRtspResponse(const std::string& res_code, const std::map<std::string, std::string>& headers,
	const std::string& sdp, const std::string& protocol) {
	auto header = headers;
	header.emplace("CSeq", std::to_string(_cseq));
	if (!_sessionid.empty()) {
		header.emplace("Session", _sessionid);
	}
	header.emplace("Server", "RtspServer Study");
	header.emplace("Date", dateStr());

	if (!sdp.empty()) {
		header.emplace("Content-Length", std::to_string(sdp.size()));
		header.emplace("Content-Type", "application/sdp");
	}
	std::string message;
	message += protocol + " " + res_code + "\r\n";
	for (auto& pr : header) {
		message +=  pr.first + ": " + pr.second + "\r\n";
	}
	message += "\r\n";
	if (!sdp.empty()) {
		message += sdp;
	}
	return message;
}

bool RtspSession::Send(const RtpPacket::Ptr& rtp) {
	auto strong_self = conn_weak_self.lock();
	if (!strong_self) {
		//对象已销毁
		return false;
	}
	strong_self->Send(rtp->getData()->RetrieveAllAsString());
}

bool RtspSession::Send(const std::string& msg) {
	auto strong_self = conn_weak_self.lock();
	if (!strong_self) {
		//对象已销毁
		return false;
	}
	strong_self->Send(msg);
}

void RtspSession::parse(const std::string& url_in) {
	std::string url = url_in;
	auto schema_pos = url.find("://");
	if (schema_pos != std::string::npos) {
		_schema = url.substr(0, schema_pos);
		url = url.substr(schema_pos + 3);
	}

	auto pos = url.find("/");
	auto host_pos = url.substr(0,pos).rfind(":");
	if (host_pos == std::string::npos) {
		_host = url.substr(0, pos);
		_vhost = _host;
		return;
	}
	_host = url.substr(0, host_pos);
	sscanf(url.substr(0, pos).data() + host_pos + 1, "%" SCNu16, &_port);
	_vhost = _host;

	url = url.substr(pos + 1);
	pos = url.find("/");
	if (pos == std::string::npos) {
		_app = url;
		return;
	}
	_app = url.substr(0, pos);

	url = url.substr(pos + 1);
	if (url.back() == '/') {
		url.pop_back();
	}
	_streamid = url;
}

std::shared_ptr<RtspMediaStream> RtspSession::getStream() {
	return _stream;
}

void RtspSession::setConnWeakPtr(const std::weak_ptr<TcpConnection>& conn_weak) {
	conn_weak_self = conn_weak;
}
