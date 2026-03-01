#ifndef RTSP_PLAYER_H
#define RTSP_PLAYER_H

#include "NetWork/TcpConnection.h"
#include "Rtsp.h"

class HttpRequest;
class TcpConnection;

class RtspPlayer : public SessionBase{
    using RtspResponse = HttpRequest;
	using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
public:
	RtspPlayer();
	~RtspPlayer();
    void setConnWeakPtr(const std::weak_ptr<TcpConnection>& conn_weak);

    void onPlay();

	void onWholeRtspPacket(const RtspResponse& request);

    //rtcp
	void updateRtcpContext(const RtpPacket::Ptr& rtp);
	//rtp
	void onRtpPacket(const char* data,size_t len);

	void onRtcpPacket(const char* data, size_t len);

private:
    void handleResSETUP(const RtspResponse& response, unsigned int track_idx);
    void handleResDESCRIBE(const RtspResponse& response);
    bool handleAuthenticationFailure(const std::string &wwwAuthenticateParamsStr);
    void handleResPAUSE(const RtspResponse& response, int type);
    bool handleResponse(const std::string &cmd, const RtspResponse& response);

    void sendOptions();
    void sendSetup(unsigned int track_idx);
    void sendPause(int type , uint32_t ms);
    void sendDescribe();
    void sendTeardown();
    void sendKeepAlive();
    void sendRtspRequest(const std::string &cmd, const std::string &url ,const StrCaseMap &header = StrCaseMap());
    void sendRtspRequest(const std::string &cmd, const std::string &url ,const std::initializer_list<std::string> &header);

private:
    std::weak_ptr<TcpConnection> conn_weak_self;
};


#endif //RTSP_PLAYER_H