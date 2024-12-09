#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <set>
#include <vector>
#include <memory>
#include <utility>
#include <functional>
#include "common.h"
#include "TimeStamp.h"

class Timer;
class EventLoop;
class Channel;
class TimerQueue
{
public:
	DISALLOW_COPY_AND_MOVE(TimerQueue)
	TimerQueue(EventLoop* loop);
	~TimerQueue();

	void CreateTimerfd(); //����timerfd
    void ReadTimerFd(); // ��ȡtimerfd�¼�
	void HandleRead(); //timerfd�ɶ�ʱ����

	void ResetTimerFd(Timer* timer); //��������timerfd��ʱʱ�䣬��ע�µĶ�ʱ����
	void ResetTimers(); //�������ظ����ԵĶ�ʱ�����¼������

	bool Insert(Timer* timer); //����ʱ����������
	void AddTimer(TimeStamp timestamp, std::function<void()>const& cb, double interval); //���һ����ʱ����

private:
	using  Entry =  std::pair<TimeStamp, Timer*>;

	EventLoop* loop_;
	int timerfd_;
	std::unique_ptr<Channel> channel_;

	std::set<Entry> timers_; //��ʱ������
	std::vector<Entry> active_timers_; //����Ķ�ʱ��
};

#endif // TIMERQUEUE_H