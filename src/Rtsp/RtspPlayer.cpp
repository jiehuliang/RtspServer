#include "RtspPlayer.h"

#include "Event/EventLoop.h"
#include "NetWork/TcpClient.h"
#include "NetWork/TcpConnection.h"
#include "HooLog/HooLog.h"
#include "Util/Buffer.h"
#include "Http/HttpContext.h"
#include "Util/CurrentThread.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <functional>

RtspClient::RtspClient(EventLoop* loop, const char* ip, const int port){
    client_ = std::unique_ptr<TcpClient>(new TcpClient(loop,ip,port));
    client_->set_connectio_callback(std::bind(&RtspClient::onConnection, this, std::placeholders::_1));
    client_->set_message_callback(std::bind(&RtspClient::onMessage, this, std::placeholders::_1));
};
RtspClient::~RtspClient() {};

void RtspClient::Start() {
    client_->Start();
}

void RtspClient::Write(const std::string& message) {
    client_->Write(message);
}

void RtspClient::Close() {
    client_->Close();
}

void RtspClient::onConnection(const std::shared_ptr<TcpConnection>& conn) {
    // 获取接收连接的Ip地址和port端口
    int clnt_fd = conn->fd();
    struct sockaddr_in peeraddr;
    socklen_t peer_addrlength = sizeof(peeraddr);
    getpeername(clnt_fd, (struct sockaddr*)&peeraddr, &peer_addrlength);

    LOG_DEBUG << "ThreadId:" << CurrentThread::tid()
        << " RtspClient::OnNewConnection : new connection "
        << "[fd#" << clnt_fd << "]"
        << " from " << inet_ntoa(peeraddr.sin_addr) << ":" << ntohs(peeraddr.sin_port);

    Write("hello !!!!!!");
};

void RtspClient::onMessage(const std::shared_ptr<TcpConnection>& conn) {
    // std::cout << CurrentThread::tid() << " EchoServer::onMessage" << std::endl;
    if (conn->state() == TcpConnection::ConnectionState::Connected)
    {
        LOG_DEBUG << "ThreadId:" << CurrentThread::tid() << " Message from clent " << conn->read_buf()->RetrieveAllAsString();
        //conn->Send(conn->read_buf()->beginread(), conn->read_buf()->readablebytes());
        //conn->HandleClose();
    }
}