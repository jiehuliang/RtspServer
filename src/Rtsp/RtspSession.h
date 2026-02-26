#ifndef RTSP_SESSION_H
#define RTSP_SESSION_H
#include <string>
#include <memory>
#include <map>
#include <cstdio>
#include "RtspMediaStream.h"
#include "Rtsp.h"
#include "NetWork/TcpConnection.h"
#include "Timer/Timer.h"
#include "Rtcp/RtcpContext.h"
class HttpRequest;
class TcpConnection;

class RtspSession :public SessionBase , public std::enable_shared_from_this<RtspSession>{
	using RtspRequest = HttpRequest;
	using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
public:
	RtspSession();
	~RtspSession();
	void onWholeRtspPacket(const RtspRequest& request);
	void handleOptions(const RtspRequest& request);
	void handleDescribe(const RtspRequest& request);
	void handleSetup(const RtspRequest& request);
	void handlePlay(const RtspRequest& request);
	void handleTeardown(const RtspRequest& request);
	void handlePause(const RtspRequest& request);

	//rtcp
	void updateRtcpContext(const RtpPacket::Ptr& rtp);
	//rtp
	void onRtpPacket(const char* data,size_t len);

	void onRtcpPacket(const char* data, size_t len);

	std::string getRtspResponse(const std::string& res_code, const std::initializer_list<std::string>& headers,
		const std::string& sdp = "", const std::string& protocol = "RTSP/1.0");
	std::string getRtspResponse(const std::string& res_code, const std::map<std::string, std::string>& headers = std::map<std::string, std::string>(),
		const std::string& sdp = "", const std::string& protocol = "RTSP/1.0");

	bool Send(const RtpPacket::Ptr& rtp);
	bool Send(const std::string &msg);

	void parse(const std::string& url_in);

	std::shared_ptr<RtspMediaStream> getStream();

	void setConnWeakPtr(const std::weak_ptr<TcpConnection>& conn_weak);

private:
	std::weak_ptr<TcpConnection> conn_weak_self;


	int _cseq = 0;

	uint64_t _bytes_usage = 0;
	//ContentBase
	std::string _content_base;
	//Session id
	std::string _sessionid;


	std::string _schema;
	std::string _host;
	uint16_t _port = 0;
	std::string _vhost;
	std::string _app;
	std::string _streamid;
	std::map<std::string, std::string> _param_strs;

	std::shared_ptr<RtspMediaStream> _stream;


	eRtpType _rtp_type = eRtpType::RTP_Invalid;


	Timer::TimerPtr _timer;

	bool _send_sr_rtcp = true;
	TimeStamp _rtcp_send_ticker = TimeStamp::Now();
	std::vector<std::shared_ptr<RtcpContext>> _rtcp_context;
};

#endif // RTSP_SESSION_H