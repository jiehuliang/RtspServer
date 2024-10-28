#include "Connection.h"
#include "Socket.h"
#include "Channel.h"
#include "Buffer.h"
#include "util.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define READ_BUFFER 1024

Connection::Connection(EventLoop* _loop, Socket* _sock) :loop(_loop), sock(_sock), channel(nullptr), inBuffer(new std::string()), readBuffer(nullptr) {
	channel = new Channel(loop, sock->getfd());
	std::function<void()> cb = std::bind(&Connection::echo,this, sock->getfd());
	channel->setCallback(cb);
	channel->enableReading();
    readBuffer = new Buffer();
}

Connection::~Connection() {
	delete channel;
	delete sock;
}

void Connection::echo(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {    //����ʹ�÷�����IO����ȡ�ͻ���buffer��һ�ζ�ȡbuf��С���ݣ�ֱ��ȫ����ȡ���
        memset(&buf, 0, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0) {
            printf("message from client fd %d: %s\n", sockfd, buf);
            readBuffer->append(buf, bytes_read);
        }
        else if (bytes_read == -1 && errno == EINTR) {  //�ͻ��������жϡ�������ȡ
            printf("continue reading");
            continue;
        }
        else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {//������IO�����������ʾ����ȫ����ȡ���
            printf("finish reading once, errno: %d\n", errno);
            errif(write(sockfd, readBuffer->c_str(), readBuffer->size()) == -1, "socket write error");
            readBuffer->clear();
            break;
        }
        else if (bytes_read == 0) {  //EOF���ͻ��˶Ͽ�����
            printf("EOF, client fd %d disconnected\n", sockfd);
            deleteConnectionCallback(sock);
            break;
        }
    }
}

    void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> _cb) {
        deleteConnectionCallback = _cb;
    }
