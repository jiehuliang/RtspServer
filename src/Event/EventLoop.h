#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "common.h"

#include <memory>
#include <mutex>
#include <thread>
#include <functional>
#include <vector>
class Epoller;
class EventLoop
{
public:
    DISALLOW_COPY_AND_MOVE(EventLoop);
    EventLoop();
    ~EventLoop();
    
    void Loop();
    void UpdateChannel(Channel *ch);
    void DeleteChannel(Channel *ch);


    // ���ж����е�����
    void DoToDoList();

    // ��������ӵ������С���loop���polling������
    void QueueOneFunc(std::function<void()> fn); 

    // ����ɴ�����Loop���̵߳��ã�������ִ��fn����
    // ���򣬽�fn���뵽�����У��ȴ�֮������
    void RunOneFunc(std::function<void()> fn);
    
    // �жϵ��øú������ǲ��ǵ�ǰ���̣߳����ǲ��Ǵ�����ǰLoop���̡߳�
    bool IsInLoopThread();

    void HandleRead();

private:
    std::unique_ptr<Epoller> poller_;
    std::vector<std::function<void()>> to_do_list_;
    std::mutex mutex_;

    int wakeup_fd_;
    std::unique_ptr<Channel> wakeup_channel_;

    bool calling_functors_;
    pid_t tid_;
};

#endif // EVENTLOOP_H
