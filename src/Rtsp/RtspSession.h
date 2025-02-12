#ifndef RTSP_SESSION_H
#define RTSP_SESSION_H
#include <string>
#include <memory>
#include <map>
#include "RtspMediaStream.h"
#include "Rtsp.h"
class HttpRequest;
class TcpConnection;

class RtspSession {
	using RtspRequest = HttpRequest;
	using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
public:
	RtspSession();
	~RtspSession();
	void onWholeRtspPacket(const TcpConnectionPtr& conn,const RtspRequest& request);
	void handleOptions(const TcpConnectionPtr& conn, const RtspRequest& request);
	void handleDescribe(const TcpConnectionPtr& conn, const RtspRequest& request);
	void handleSetup(const TcpConnectionPtr& conn, const RtspRequest& request);
	void handlePlay(const TcpConnectionPtr& conn, const RtspRequest& request);
	void handleTeardown(const TcpConnectionPtr& conn, const RtspRequest& request);
	void handlePause(const TcpConnectionPtr& conn, const RtspRequest& request);

	std::string getRtspResponse(const std::string& res_code, const std::initializer_list<std::string>& headers,
		const std::string& sdp = "", const std::string& protocol = "RTSP/1.0");
	std::string getRtspResponse(const std::string& res_code, const std::map<std::string, std::string>& headers = std::map<std::string, std::string>(),
		const std::string& sdp = "", const std::string& protocol = "RTSP/1.0");

	void parse(const std::string& url_in);

private:
	//�յ���seq���ظ�ʱһ��
	int _cseq = 0;
	//���ĵ�������
	uint64_t _bytes_usage = 0;
	//ContentBase
	std::string _content_base;
	//Session��
	std::string _sessionid;

	//�������url��Ϣ
	std::string _schema;
	std::string _host;
	uint16_t _port = 0;
	std::string _vhost;
	std::string _app;
	std::string _streamid;
	std::map<std::string, std::string> _param_strs;

	std::shared_ptr<RtspMediaStream> _stream;

	//�����������ͻ��˲��õ�rtp���䷽ʽ
	eRtpType _rtp_type = eRtpType::RTP_Invalid;
};

#endif // RTSP_SESSION_H