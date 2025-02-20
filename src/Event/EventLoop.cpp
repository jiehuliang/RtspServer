#include "EventLoop.h"

#include "Channel.h"
#include "Epoller.h"

#include "TimerQueue.h"
#include "TimeStamp.h"
#include <memory>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include "CurrentThread.h"
#include <sys/eventfd.h>
#include <assert.h>


EventLoop::EventLoop() : tid_(CurrentThread::tid()) { 
    // ��Loop����������˲�ͬ���̣߳���ȡִ�иú������߳�
	poller_ = std::unique_ptr<Epoller>(new Epoller());

    timer_queue_ = std::unique_ptr<TimerQueue>(new TimerQueue(this));

    wakeup_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    wakeup_channel_ = std::unique_ptr<Channel>(new Channel(wakeup_fd_, this));
    calling_functors_ = false;
    wakeup_channel_->set_read_callback(std::bind(&EventLoop::HandleRead, this));
    wakeup_channel_->EnableRead();
}

EventLoop::~EventLoop() {
    DeleteChannel(wakeup_channel_.get());
    ::close(wakeup_fd_);
}

void EventLoop::Loop(){
    while (true)
    {
        for (Channel *active_ch : poller_->Poll()){
            active_ch->HandleEvent();
        }
        DoToDoList();
    }
}

void EventLoop::UpdateChannel(Channel *ch) { poller_->UpdateChannel(ch); }
void EventLoop::DeleteChannel(Channel *ch) { poller_->DeleteChannel(ch); }

bool EventLoop::IsInLoopThread(){
    return CurrentThread::tid() == tid_;
}

void EventLoop::RunOneFunc(std::function<void()> cb){
    if(IsInLoopThread()){
        cb();
    }else{
        QueueOneFunc(cb);
    }
}

void EventLoop::QueueOneFunc(std::function<void()> cb){
    {
        // ��������֤�߳�ͬ��
        std::unique_lock<std::mutex> lock(mutex_);
        to_do_list_.emplace_back(std::move(cb));
    }

    // ������õ�ǰ�����Ĳ����ǵ�ǰ��ǰEventLoop��Ӧ�ĵ��̣߳����份�ѡ���Ҫ���ڹر�TcpConnection
    // ���ڹر��������ɶ�Ӧ`TcpConnection`������ģ����ǹر����ӵĲ���Ӧ����main_reactor������(Ϊ���ͷ�ConnectionMap�������е�TcpConnection)
    if (!IsInLoopThread() || calling_functors_) {
        uint64_t write_one_byte = 1;  
        ssize_t write_size = ::write(wakeup_fd_, &write_one_byte, sizeof(write_one_byte));
        (void) write_size;
        assert(write_size == sizeof(write_one_byte));
    } 
}

void EventLoop::DoToDoList(){
    // ��ʱ�Ѿ�epoll_wait���������ܴ���������epoll_wait�Ŀ����ԡ�
    calling_functors_ = true;

    std::vector < std::function<void()>> functors;
    {
        // ���� ��֤�߳�ͬ��
        std::unique_lock<std::mutex> lock(mutex_); 
        functors.swap(to_do_list_);
    }
    for(const auto& func: functors){
        func();
    }

    calling_functors_ = false;
}

void EventLoop::HandleRead(){
    // ���ڻ���EventLoop
    uint64_t read_one_byte = 1;
    ssize_t read_size = ::read(wakeup_fd_, &read_one_byte, sizeof(read_one_byte));
    (void) read_size;
    assert(read_size == sizeof(read_one_byte));
    return;
}

void EventLoop::RunAt(TimeStamp timestamp, std::function<bool()>const& cb) {
    timer_queue_->AddTimer(timestamp,std::move(cb),0.0);
}

void EventLoop::RunAfter(double wait_time, std::function<bool()>const& cb, TimeUnit unit) {
    TimeStamp timestamp(TimeStamp::AddTime(TimeStamp::Now(), wait_time, unit));
    timer_queue_->AddTimer(timestamp, std::move(cb), 0.0, unit);
}

Timer::TimerPtr EventLoop::RunEvery(double interval, std::function<bool()>const& cb, TimeUnit unit) {
    TimeStamp timestamp(TimeStamp::AddTime(TimeStamp::Now(), interval, unit));
    return timer_queue_->AddTimer(timestamp, std::move(cb), interval, unit);
}