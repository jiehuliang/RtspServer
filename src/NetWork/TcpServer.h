#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "common.h"
#include <functional>
#include <map>
#include <vector>
#include <memory>
class EventLoop;
class TcpConnection;
class Acceptor;
class EventLoopThreadPool;
class InetAddress;
class TcpServer
{
public:
    DISALLOW_COPY_AND_MOVE(TcpServer);
    TcpServer(EventLoop *loop, const char *ip, const int port);
    ~TcpServer();

    void Start();

    void set_connection_callback(std::function < void(const std::shared_ptr<TcpConnection> &)> const &fn);
    void set_message_callback(std::function < void(const std::shared_ptr<TcpConnection> &)> const &fn);

    inline void HandleClose(const std::shared_ptr<TcpConnection> &);
    // ����һ�����ķ�װ���Ա�֤erase��������`main_reactor_`�������ġ�
    inline void HandleCloseInLoop(const std::shared_ptr<TcpConnection> &);

    // ���յ���Ϣ���Ĳ�����
    inline void HandleNewConnection(int fd);
    
    // �����߳�����
    void SetThreadNums(int thread_nums);

private:
    EventLoop *main_reactor_;
    int next_conn_id_;

    std::unique_ptr<EventLoopThreadPool> thread_pool_;

    std::unique_ptr<Acceptor> acceptor_;
    std::map<int, std::shared_ptr<TcpConnection>> connectionsMap_;

    std::function<void(const std::shared_ptr<TcpConnection> &)> on_connect_;
    std::function<void(const std::shared_ptr<TcpConnection> &)> on_message_;

};

#endif // TCPSERVER_H
