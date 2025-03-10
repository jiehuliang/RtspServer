#ifndef RtspServer_H
#define RtspServer_H

#include "Util/common.h"
#include <memory>

class EventLoop;
class TcpServer;
class TcpConnection;

// �Զ��رյ�ʱ�䣬����Ϊ��λ
#define AUTOCLOSETIMEOUT 100

class RtspServer{
public:
	RtspServer(EventLoop* loop,const char* ip,const int port,bool auto_close_conn);
	~RtspServer();

	void OnConnection(const std::shared_ptr<TcpConnection>& conn);

	void OnMessage(const std::shared_ptr<TcpConnection>& conn);

	void SetThreadNums(int num);

	void start();

private:
	EventLoop* loop_;
	std::unique_ptr<TcpServer> server_;

	// �Ƿ��Զ��ر����ӡ�
	bool auto_close_conn_;
};

#endif // RTSPSERVER_H