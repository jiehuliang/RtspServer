#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#include "common.h"

#include <memory>
#include <functional>

class EventLoop;
class Channel;

class Acceptor{
    public:
        DISALLOW_COPY_AND_MOVE(Acceptor);
        Acceptor(EventLoop *loop, const char * ip, const int port);
        ~Acceptor();

        void set_newconnection_callback(std::function<void(int)> const &callback);
        
        // ����socket
        void Create();

        // ��ip��ַ��
        void Bind(const char *ip, const int port);
        
        // ����Socket
        void Listen();

        // ��������
        void AcceptConnection();

    private:
        EventLoop *loop_;
        int listenfd_;
        std::unique_ptr<Channel> channel_;
        std::function<void(int)> new_connection_callback_;
};
#endif // ACCEPTOR_H
