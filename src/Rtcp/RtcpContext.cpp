#include "RtcpContext.h"
#include "TimeStamp.h"
#include <arpa/inet.h>


void RtcpContext::onRtp(uint16_t seq, uint32_t stamp, uint64_t ntp_stamp_ms, uint32_t sample_rate, size_t bytes) {
	++_packets;
	_bytes += bytes;
	_last_rtp_stamp = stamp;
	_last_ntp_stamp_ms = ntp_stamp_ms;
}

void RtcpContextForSend::onRtcp(RtcpHeader* rtcp) {
	switch ((RTCP_TYPE)rtcp->pt) {
		case RTCP_TYPE::RTCP_RR: {
			auto rtcp_rr = (RtcpRR*)rtcp;
			for (auto item : rtcp_rr->getItemList()) {
				if (!item->last_sr_stamp) {
					continue;
				}
				auto it = _sender_report_ntp.find(item->last_sr_stamp);
				if (it == _sender_report_ntp.end()) {
					continue;
				}
				//发送sr 到收到rr之间的时间戳增量,系统时间可回退不稳定，以后需要换成程序启动时间
				auto ms_inc = TimeStamp::Now().microseconds() - it->second;
				//rtp接收端收到sr包后，回复rr包的延时，已转换为毫秒
				auto delay_ms = (uint64_t)item->delay_since_last_sr * 1000 / 65536;
				auto rtt = (int)(ms_inc - delay_ms);
				if (rtt >= 0) {
					//rtt 不可能小于0
					_rtt[item->ssrc] = rtt;
				}
			}
			break;
		}
		default:break;
	}
}



std::shared_ptr<Buffer> RtcpContextForSend::createRtcpSR(uint32_t rtcp_ssrc) {
	auto rtcp = RtcpSR::create(0);
	rtcp->setNtpStamp(_last_ntp_stamp_ms);
	rtcp->rtpts = htonl(_last_rtp_stamp);
	rtcp->ssrc = htonl(rtcp_ssrc);
	rtcp->packet_count = htonl((uint32_t)_packets);
	rtcp->octet_count = htonl((uint32_t) _bytes);

	//记录上次发送的sender report信息，用于后续统计rtt
	auto last_sr_lsr = ((ntohl(rtcp->ntpmsw) & 0xFFFF << 16) | ((ntohl(rtcp->ntplsw) >> 16) & 0xFFFF));
	//暂时使用系统时间戳作为LSR，后续需要换成程序启动时间，更稳定
	_sender_report_ntp[last_sr_lsr] = TimeStamp::Now().microseconds();
	if (_sender_report_ntp.size() >= 5) {
		_sender_report_ntp.erase(_sender_report_ntp.begin());
	}

	return RtcpHeader::toBuffer(rtcp);
}