#include "RtspServer.h"
#include "RtspSession.h"
#include "Event/EventLoop.h"
#include "NetWork/TcpServer.h"
#include "NetWork/TcpConnection.h"
#include "Log/Logging.h"
#include "NetWork/Buffer.h"
#include "Http/HttpContext.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <functional>

RtspServer::RtspServer(EventLoop* loop,const char* ip,const int port,bool auto_close_conn):loop_(loop),auto_close_conn_(auto_close_conn) {
	server_ = std::unique_ptr<TcpServer>(new TcpServer(loop,ip,port));
	server_->set_connection_callback(std::bind(&RtspServer::OnConnection, this, std::placeholders::_1));
	server_->set_message_callback(std::bind(&RtspServer::OnMessage,this,std::placeholders::_1));
	LOG_INFO << "RtspServer Listening on [ " << ip << ":" << port << " ]";
}

RtspServer::~RtspServer() {

}

void RtspServer::OnConnection(const std::shared_ptr<TcpConnection>& conn) {
	auto rtsp_session = std::make_shared<RtspSession>();
	rtsp_session->setConnWeakPtr(conn);
	conn->set_session(rtsp_session);

	int clnt_fd = conn->fd();
	struct sockaddr_in peeraddr;
	socklen_t peer_addrlength = sizeof(peeraddr);
	getpeername(clnt_fd,(struct sockaddr *)&peeraddr,&peer_addrlength);
	LOG_INFO << "RtspServer::OnNewConnection : Add connection "
		<< "[ fd#" << clnt_fd << "-id#" << conn->id() << " ]"
		<< " from " << inet_ntoa(peeraddr.sin_addr) << ":" << ntohs(peeraddr.sin_port);
}

void RtspServer::OnMessage(const std::shared_ptr<TcpConnection>& conn) {
	if (conn->state() == TcpConnection::ConnectionState::Connected) {
		auto ptr = conn->read_buf()->Peek();

		HttpContext* context = conn->context();
		if (ptr[0] != '$') {
			auto str = conn->read_buf()->RetrieveAllAsString();
			if (!context->ParaseRequest(str))
			{
				LOG_INFO << "RtspServer::onMessage : Receive non Rtsp message ";
				//conn->Send("Rtsp/1.0 400 Bad Request\r\n\r\n");
				//conn->HandleClose();
			}
			if (context->GetCompleteRequest())
			{
				((RtspSession*)conn->session())->onWholeRtspPacket(*context->request());
				context->ResetContextStatus();
			}
		}
		else {
			//这是rtp包
			if (conn->read_buf()->readablebytes() < 4) {
				//数据不够
				return ;
			}
			if (ptr[1] != 0x00 && ptr[1] != 0x01 && ptr[1] != 0x02 && ptr[1] != 0x03) {
				//数据错误
				return ;
			}
			uint16_t length = (((uint8_t*)ptr)[2] << 8) | ((uint8_t*)ptr)[3];
			if (conn->read_buf()->readablebytes() < length) {
				return;
			}
			auto str = conn->read_buf()->RetrieveAsString(RtpPacket::RtpTcpHeaderSize + length);
			((RtspSession*)conn->session())->onRtpPacket(str.c_str(), str.size());
		}

	}
}



void RtspServer::SetThreadNums(int num) {
	server_->SetThreadNums(num);
}

void RtspServer::start(){
	server_->Start();
}