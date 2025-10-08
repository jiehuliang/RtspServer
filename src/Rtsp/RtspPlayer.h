#ifndef RTSP_PLAYER_H
#define RTSP_PLAYER_H

#include "Util/common.h"
#include <memory>

class TcpConnection;
class TcpClient;
class RtspClient {
public:
	RtspClient(EventLoop* loop, const char* ip, const int port);
	~RtspClient();

	void onConnection(const std::shared_ptr<TcpConnection>& conn);

	void onMessage(const std::shared_ptr<TcpConnection>& conn);

	void Start();

	void Write(const std::string& message);

	void Close();

private:
	std::unique_ptr <TcpClient> client_;
};


#endif // RTSP_PLAYER_H