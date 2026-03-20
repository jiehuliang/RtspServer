#include "RtspPlayer.h"

#include "Http/HttpRequest.h"

RtspPlayer::RtspPlayer() {
}

RtspPlayer::~RtspPlayer() {
}

void RtspPlayer::setConnWeakPtr(const std::weak_ptr<TcpConnection>& conn_weak) {
	conn_weak_self = conn_weak;
}

void RtspPlayer::onPlay(){
    sendOptions();
}

void  RtspPlayer::onWholeRtspPacket(const RtspResponse& request){
	_cseq = atoi(request.GetRequestValue("CSeq").c_str());
}



void RtspPlayer::sendOptions(){
	sendRtspRequest("OPTIONS",_base_url);
}


void RtspPlayer::sendRtspRequest(const std::string &cmd, const std::string &url ,const std::initializer_list<std::string> &header){
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
	sendRtspRequest(cmd,url, header_map);
}

void RtspPlayer::sendRtspRequest(const std::string &cmd, const std::string &url ,const std::map<std::string, std::string>& headers){
    auto header = headers;
    header.emplace("CSeq", std::to_string(_cseq));
    header.emplace("User-Agent", kServerName);

    if (!_session_id.empty()) {
        header.emplace("Session", _session_id);
    }

	std::string message;
	message += cmd + " " + url + "RTSP/1.0" + "\r\n";
	for (auto& pr : header) {
		message +=  pr.first + ": " + pr.second + "\r\n";
	}


}



