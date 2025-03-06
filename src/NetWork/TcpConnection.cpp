#include "TcpConnection.h"
#include "Buffer.h"
#include "Channel.h"
#include "common.h"
#include "EventLoop.h"
#include "HttpContext.h"
#include "TimeStamp.h"
#include "Logging.h"
#include <thread>
#include <memory>
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <sys/socket.h>
#include <signal.h>


TcpConnection::TcpConnection(EventLoop *loop, int connfd, int connid): connfd_(connfd), connid_(connid), loop_(loop){

    if (loop != nullptr)
    {
        channel_ = std::unique_ptr<Channel>(new Channel(connfd, loop));
        channel_->EnableET();
        channel_->set_read_callback(std::bind(&TcpConnection::HandleMessage, this));
        channel_->set_write_callback(std::bind(&TcpConnection::HandleWrite, this));
    }
    read_buf_ = std::unique_ptr<Buffer>(new Buffer());
    send_buf_ = std::unique_ptr<Buffer>(new Buffer());
    context_ = std::unique_ptr<HttpContext>(new HttpContext());
    session_ = std::unique_ptr<RtspSession>(new RtspSession());
}

TcpConnection::~TcpConnection(){
    //loop_->DeleteChannel(channel_.get());
    ::close(connfd_);
}

void TcpConnection::ConnectionEstablished(){
    state_ = ConnectionState::Connected;
    channel_->Tie(shared_from_this());
    channel_->EnableRead();
    if (on_connect_){
        on_connect_(shared_from_this());
    }
    session_->setConnWeakPtr(std::weak_ptr<TcpConnection>(shared_from_this()));
}

void TcpConnection::ConnectionDestructor(){
    //std::cout << CurrentThread::tid() << " TcpConnection::ConnectionDestructor" << std::endl;
    // ���ò���������������ֲ�ô����������ܣ���Ϊ������ǰ����ǰ`TcpConnection`�Ѿ��൱�ڹر��ˡ�
    // �Ѿ����Խ����loop���뿪
    loop_->DeleteChannel(channel_.get());
}

void TcpConnection::set_connection_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &fn){
    on_connect_ = std::move(fn);
}
void TcpConnection::set_close_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &fn) { 
    on_close_ = std::move(fn); 
}
void TcpConnection::set_message_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &fn) { 
    on_message_ = std::move(fn);
}


void TcpConnection::HandleClose() {
    //std::cout << std::this_thread::get_id() << " TcpConnection::HandleClose" << std::endl;
    if (state_ != ConnectionState::Disconected)
    {
        state_ = ConnectionState::Disconected;
        if(on_close_){
            on_close_(shared_from_this());
        }
    }
}

void TcpConnection::HandleMessage(){
    Read();
    if (on_message_)
    {
        on_message_(shared_from_this());
    }
}

void TcpConnection::HandleWrite() {
    //LOG_INFO << "TcpConnection::HandleWrite";
    WriteNonBlocking();
}


EventLoop *TcpConnection::loop() const { return loop_; }
int TcpConnection::fd() const { return connfd_; }
int TcpConnection::id() const { return connid_; }
TcpConnection::ConnectionState TcpConnection::state() const { return state_; }
Buffer *TcpConnection::read_buf(){ return read_buf_.get(); }
Buffer *TcpConnection::send_buf() { return send_buf_.get(); }

void TcpConnection::Send(const std::string &msg){
    Send(msg.data(), static_cast<int>(msg.size()));
}

void TcpConnection::Send(const char *msg){
    Send(msg, static_cast<int>(strlen(msg)));
} 

void TcpConnection::Send(const char *msg, int len){
    int remaining = len;
    int send_size = 0;

    
    //�����ʱsend_buf_��û�����ݣ�������ȳ��Է�������
    if (send_buf_->readablebytes() == 0) {
        //ǿ������ת��������remaining����
        send_size = static_cast<int>(write(connfd_, msg, len));
        // send_size = static_cast<int>(send(connfd_, msg, len, 0));
        if (send_size >= 0) {
            //˵�������˲�������
            remaining -= send_size;
        }
        else if ((send_size == -1) && 
            ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            //˵����ʱTCP�����������ģ�û�а취д�룬ʲô������
            send_size = 0;//˵��ʵ����û�з�������
        }
        else {
            LOG_ERROR << "TcpConnection::Send - TcpConnection Send Error";
            channel_->set_write_callback([]() {});
            HandleClose();
            return;
        }
    }
    //��ʣ������ݼ��뵽send_buf�У��ȴ��������͡�
    assert(remaining <= len);
    if (remaining > 0) {
        send_buf_->Append(msg + send_size, remaining);

        //������һ��ʱ
        //1.��û�м���д�¼����ڴ�ʱ�����˼���
        //2.������д�¼��������Ѿ������ˣ���ʱ�ٴμ�����ǿ�ƴ���һ�Σ�
        // ���ǿ�ƴ���ʧ�ܣ���Ȼ���Եȴ�����TCP��������д��
        channel_->EnableWrite();
    }
}

void TcpConnection::Read()
{
    ReadNonBlocking();
}

void TcpConnection::Write(){
    WriteNonBlocking();
}


void TcpConnection::ReadNonBlocking(){
    char buf[1024];
    while(true){
        memset(buf, 0, sizeof(buf));
        ssize_t bytes_read = read(connfd_, buf, sizeof(buf));
        if(bytes_read > 0){
            read_buf_->Append(buf, bytes_read);
        }else if(bytes_read == -1 && errno == EINTR){
            //std::cout << "continue reading" << std::endl;
            continue;
        }else if((bytes_read == -1) && (
            (errno == EAGAIN) || (errno == EWOULDBLOCK))){
            break;
        }else if (bytes_read == 0){//
            HandleClose();
            break;
        }else{
            HandleClose();
            break;
        }
    }
}

void TcpConnection::WriteNonBlocking(){
    int remaining = send_buf_->readablebytes();
    if (remaining == 0){
        return;
    }
    // int send_size = static_cast<int>(::write(connfd_, send_buf_->Peek(), remaining));
    int send_size = static_cast<int>(::send(connfd_, send_buf_->Peek(), remaining, 0));
    if ((send_size == -1) && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
        send_size = 0;
    }
    else if (send_size == -1) {
        LOG_ERROR << "TcpConnection::Send - TcpConnection Send Error";
        channel_->set_write_callback([]() {});
        HandleClose();
        //return;
    }

    remaining -= send_size;
    send_buf_->Retrieve(send_size);
}

HttpContext *TcpConnection::context() const { return context_.get(); }

RtspSession *TcpConnection::session() const { return session_.get(); }

TimeStamp TcpConnection::timestamp() const { return timestamp_; }

void TcpConnection::UpdateTimeStamp(TimeStamp now){
    timestamp_ = now;
}