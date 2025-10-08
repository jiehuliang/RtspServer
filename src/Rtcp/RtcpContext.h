#ifndef RTCP_CONTEXT_H
#define RTCP_CONTEXT_H
#include "Rtcp.h"
#include "Util/Buffer.h"
#include "HooLog/HooLog.h"
#include <map>

class RtcpContext {
public:
	using Ptr = std::shared_ptr<RtcpContext>;
	virtual ~RtcpContext() = default;


	virtual void onRtp(uint16_t seq, uint32_t stamp, uint64_t ntp_stamp_ms, uint32_t sample_rate, size_t bytes);

	virtual void onRtcp(RtcpHeader* rtcp) = 0;

	virtual std::shared_ptr<Buffer> createRtcpSR(uint32_t rtcp_ssrc) {
		LOG_ERROR << "没有实现, rtp接收者尝试发送sr包";
	};

protected:
	//收到或发送的rtp的字节数
	size_t _bytes = 0;
	//收到或发送的rtp的个数
	size_t _packets = 0;
	//上次的rtp时间戳,毫秒
	uint32_t _last_rtp_stamp = 0;
	uint64_t _last_ntp_stamp_ms = 0;
};


class RtcpContextForSend : public RtcpContext {
public:
	std::shared_ptr<Buffer> createRtcpSR(uint32_t rtcp_ssrc) override;

	void onRtcp(RtcpHeader* rtcp) override;

	uint32_t getRtt(uint32_t ssrc) const;

private:
	std::map<uint32_t/*ssrc*/, uint32_t/*rtt*/> _rtt;//Round-Trip Time
	std::map<uint32_t/*last_sr_lsr*/, uint32_t/*ntp stamp*/> _sender_report_ntp;
};

#endif //RTCP_CONTEXT_H