#include "RtspMediaStream.h"
#include "Log/Logging.h"
#include "Util/File.h"
#include "Util/base64.h"
#include "H264.h"

RtspMediaStream::RtspMediaStream(std::string url):url_(url) {
	_media_track = std::make_shared<Track>();
}

RtspMediaStream::~RtspMediaStream() {}

bool RtspMediaStream::createFromEs(int payload_type, int time_base) {
	stream_ = File::loadFile(url_);
	if (stream_.empty()) {
		LOG_ERROR << "Failed to load file: " << url_.c_str();
		return false;
	}
	data_ = stream_.c_str();
	size_ = stream_.size();
	int got_sps_pps = 0;
	H264Nalu sps;
	H264Nalu pps;

	while (size_ > 0 && got_sps_pps != 0x3) {
		H264Nalu nalu;
		int nalu_len = nalu.get_annexb_nalu(data_, size_);
		size_ -= nalu_len;
		data_ += nalu_len;

		if (nalu.nal_unit_type == 7) {
			sps = nalu;
			got_sps_pps = got_sps_pps | 0x1;
			continue;
		}
		if (nalu.nal_unit_type == 8) {
			pps = nalu;
			got_sps_pps = got_sps_pps | 0x2;
			continue;
		}
	}
	sdp_ = h264_sdp_create(sps.buffer.c_str(), sps.len, pps.buffer.c_str(), pps.len, payload_type, time_base);
	encoder_ = std::make_shared<H264RtpEncoder>(_media_track->_ssrc, _media_track->_interleaved, _media_track->_pt, _media_track->_samplerate);
	createTimeStamp_ = TimeStamp::Now();
	data_ = stream_.c_str();
	size_ = stream_.size();
	return true;
}

std::string RtspMediaStream::h264_sdp_create(const char* sps, const int sps_len,
	const char* pps, const int pps_len,int payload_type,int time_base) {

	char video_sdp[1024] = { 0 };

	uint32_t profile_level_id = 0;
	if (sps_len >= 4) { // sanity check
		profile_level_id = sps[1] << 16;
		profile_level_id |= sps[2] << 8;
		profile_level_id |= sps[3];    // profile_idc|constraint_setN_flag|level_idc
	}

	char str_sps[100];
	memset(str_sps, 0, 100);
	av_base64_encode(str_sps, 100, (uint8_t*)sps, sps_len);

	char str_pps[100];
	memset(str_pps, 0, 100);
	av_base64_encode(str_pps, 100, (uint8_t*)pps, pps_len);

	/**
	 Single NAI Unit Mode = 0. // Single NAI mode (Only nals from 1-23 are allowed)
	 Non Interleaved Mode = 1��// Non-interleaved Mode: 1-23��24 (STAP-A)��28 (FU-A) are allowed
	 Interleaved Mode = 2,  // 25 (STAP-B)��26 (MTAP16)��27 (MTAP24)��28 (EU-A)��and 29 (EU-B) are allowed.
	 Single NAI Unit Mode = 0. // Single NAI mode (Only nals from 1-23 are allowed)
	 Non Interleaved Mode = 1��// Non-interleaved Mode: 1-23��24 (STAP-A)��28 (FU-A) are allowed
	 Interleaved Mode = 2,  // 25 (STAP-B)��26 (MTAP16)��27 (MTAP24)��28 (EU-A)��and 29 (EU-B) are allowed.
	 *
	 **/
	int h264_stap_a = 1;

	char demo[] =
		"m=video %d RTP/AVP %d\n"
		"a=rtpmap:%d H264/%d\n"
		"a=fmtp:%d profile-level-id=%06X; packetization-mode=%d; sprop-parameter-sets=%s,%s\n";

	snprintf(video_sdp, sizeof(video_sdp), demo, 0, payload_type,
		payload_type, time_base,
		payload_type, profile_level_id, h264_stap_a, str_sps, str_pps);

	std::string title_sdp = "v=0\r\n";
	title_sdp += "o=- 0 0 IN IP4 0.0.0.0\r\n";
	title_sdp += "s=Streamed by RtspServer Study\r\n";
	title_sdp += "c=IN IP4 0.0.0.0\r\n";
	title_sdp += "t=0 0\r\n";
	title_sdp += "a=range:npt=now-\r\n";
	auto sdp = title_sdp + std::string(video_sdp);

	LOG_INFO << "\nvideo sdp:\n" << sdp;

	return sdp;
}

void RtspMediaStream::readFrame() {
	auto nowTime = TimeStamp::Now().microseconds() - createTimeStamp_.microseconds();
	while (_media_track->_time_stamp * 1000 < nowTime - 50000) {
		H264Nalu::Ptr nalu = std::make_shared<H264Nalu>();
		int nalu_len = nalu->get_annexb_nalu(data_, size_);
		size_ -= nalu_len;
		data_ += nalu_len;
		nalu->_dts = _media_track->_time_stamp * 90;
		nalu->_pts = nalu->_dts;
		encoder_->inputFrame(std::move(nalu));
		if (nalu->decodeAble()) {
			_media_track->_time_stamp += 1000 / 25;
		}
		if (size_ == 0) {
			data_ = stream_.c_str();
			size_ = stream_.size();
		}
	}
}

Track::Ptr& RtspMediaStream::getMediaTrack(){
	return _media_track;
}



std::string RtspMediaStream::getSdp() {
	return sdp_;
}

void RtspMediaStream::setEncoderSendCB(const std::function<void(const RtpPacket::Ptr& rtp)>& cb) {
	encoder_->set_send_cb(cb);
}