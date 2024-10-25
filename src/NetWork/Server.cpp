#include "Server.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"
#include "Acceptor.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>



#define READ_BUFFER 1024

Server::Server(EventLoop *_loop):loop(_loop) {
    acceptor = new Acceptor(loop);
	std::function<void(Socket*)> cb = std::bind(&Server::newConnection,this,std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server() {
    delete acceptor;
}

void Server::handleEvent(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {    //����ʹ�÷�����IO����ȡ�ͻ���buffer��һ�ζ�ȡbuf��С���ݣ�ֱ��ȫ����ȡ���
        memset(&buf,0, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0) {
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        }
        else if (bytes_read == -1 && errno == EINTR) {  //�ͻ��������жϡ�������ȡ
            printf("continue reading");
            continue;
        }
        else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {//������IO�����������ʾ����ȫ����ȡ���
            printf("finish reading once, errno: %d\n", errno);
            break;
        }
        else if (bytes_read == 0) {  //EOF���ͻ��˶Ͽ�����
            printf("EOF, client fd %d disconnected\n", sockfd);
            close(sockfd);   //�ر�socket���Զ����ļ���������epoll�����Ƴ�
            break;
        }
    }
}

void Server::newConnection(Socket* serv_sock) {
    InetAddress* clnt_addr = new InetAddress();      //�ᷢ���ڴ�й¶��û��delete
    Socket* clnt_sock = new Socket(serv_sock->accept(clnt_addr));       //�ᷢ���ڴ�й¶��û��delete
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getfd(), inet_ntoa(clnt_addr->addr.sin_addr), ntohs(clnt_addr->addr.sin_port));
    clnt_sock->setnonblocking();
    Channel* clnt_channel = new Channel(loop,clnt_sock->getfd());
    std::function<void()> cb = std::bind(&Server::handleEvent,this,clnt_sock->getfd());
    clnt_channel->setCallback(cb);
    clnt_channel->enableReading();
}