#ifndef RTSP_PLAYER_H
#define RTSP_PLAYER_H

#include "NetWork/TcpConnection.h"
#include "Rtsp.h"

#include<map>

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
    void sendRtspRequest(const std::string &cmd, const std::string &url ,const std::initializer_list<std::string> &header = {});
    void sendRtspRequest(const std::string &cmd, const std::string &url ,const std::map<std::string, std::string>& headers);


private:
    std::weak_ptr<TcpConnection> conn_weak_self;

    std::string _url;
    std::string _base_url;
    std::string _rtsp_username;
    std::string _rtsp_password;


    int _cseq;
    std::string _session_id;
    std::string kServerName;
};


#endif //RTSP_PLAYER_H