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
				//����sr ���յ�rr֮���ʱ�������,ϵͳʱ��ɻ��˲��ȶ����Ժ���Ҫ���ɳ�������ʱ��
				auto ms_inc = TimeStamp::Now().microseconds() - it->second;
				//rtp���ն��յ�sr���󣬻ظ�rr������ʱ����ת��Ϊ����
				auto delay_ms = (uint64_t)item->delay_since_last_sr * 1000 / 65536;
				auto rtt = (int)(ms_inc - delay_ms);
				if (rtt >= 0) {
					//rtt ������С��0
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

	//��¼�ϴη��͵�sender report��Ϣ�����ں���ͳ��rtt
	auto last_sr_lsr = ((ntohl(rtcp->ntpmsw) & 0xFFFF << 16) | ((ntohl(rtcp->ntplsw) >> 16) & 0xFFFF));
	//��ʱʹ��ϵͳʱ�����ΪLSR��������Ҫ���ɳ�������ʱ�䣬���ȶ�
	_sender_report_ntp[last_sr_lsr] = TimeStamp::Now().microseconds();
	if (_sender_report_ntp.size() >= 5) {
		_sender_report_ntp.erase(_sender_report_ntp.begin());
	}

	return RtcpHeader::toBuffer(rtcp);
}