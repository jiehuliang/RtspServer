#include "RtcpContext.h"
#include "Timer/TimeStamp.h"
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
				// 发送sr到收到rr之间的时间戳增量
            	// Timestamp increment between sending sr and receiving rr
				auto ms_inc = TimeStamp::Now().microseconds() - it->second;
				// rtp接收端收到sr包后，回复rr包的延时，已转换为毫秒 
            	// Delay of the rtp receiver replying to the rr packet after receiving the sr packet, converted to milliseconds
				auto delay_ms = (uint64_t)item->delay_since_last_sr * 1000 / 65536;
				auto rtt = (int)(ms_inc - delay_ms);
				if (rtt >= 0) {
					// rtt不可能小于0 
                	// RTT cannot be less than 0
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

    // 记录上次发送的sender report信息，用于后续统计rtt  [AUTO-TRANSLATED:1d22d2c8]
    // Record the last sent sender report information for subsequent RTT statistics
	auto last_sr_lsr = ((ntohl(rtcp->ntpmsw) & 0xFFFF << 16) | ((ntohl(rtcp->ntplsw) >> 16) & 0xFFFF));
	// 删除最早的sr rtcp  [AUTO-TRANSLATED:2457e08d]
    // Delete the earliest sr rtcp
	_sender_report_ntp[last_sr_lsr] = TimeStamp::Now().microseconds();
	if (_sender_report_ntp.size() >= 5) {
		_sender_report_ntp.erase(_sender_report_ntp.begin());
	}

	return RtcpHeader::toBuffer(rtcp);
}