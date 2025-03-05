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

	std::shared_ptr<Buffer> buffer;
	buffer->Append((char*)rtcp.get(), rtcp->getSize());
	return std::move(buffer);
}