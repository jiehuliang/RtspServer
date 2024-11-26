#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H
#include "common.h"
#include <functional>
#include <memory>
#include <string>
class Buffer;
class TcpConnection
{
public:
    enum ConnectionState
    {
        Invalid = 1,
        Connected,
        Disconected
    };

    DISALLOW_COPY_AND_MOVE(TcpConnection);

    
    TcpConnection(EventLoop *loop, int connfd, int connid);
    ~TcpConnection();

     // �ر�ʱ�Ļص�����
    void set_close_callback(std::function<void(int)> const &fn);   
    // ���ܵ���Ϣ�Ļص�����                                  
    void set_message_callback(std::function<void(TcpConnection *)> const &fn); 


    // �趨send buf
    void set_send_buf(const char *str); 
    Buffer *read_buf();
    Buffer *send_buf();

    void Read(); // ������
    void Write(); // д����
    void Send(const std::string &msg); // �����Ϣ
    void Send(const char *msg, int len); // �����Ϣ
    void Send(const char *msg);


    void HandleMessage(); // �����յ���Ϣʱ�����лص�

    // ��TcpConnection����ر�����ʱ�����лص����ͷ���Ӧ��socket.
    void HandleClose(); 


    ConnectionState state() const;
    EventLoop *loop() const;
    int fd() const;
    int id() const;

private:
    // �����Ӱ󶨵�Socket
    int connfd_;
    int connid_;
    // ����״̬
    ConnectionState state_;

    EventLoop *loop_;

    std::unique_ptr<Channel> channel_;
    std::unique_ptr<Buffer> read_buf_;
    std::unique_ptr<Buffer> send_buf_;

    std::function<void(int)> on_close_;
    std::function<void(TcpConnection *)> on_message_;

    void ReadNonBlocking();
    void WriteNonBlocking();
};
#endif // TCPCONNECTION_H