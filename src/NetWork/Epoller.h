#ifndef EPOLLER_H
#define EPOLLER_H
#include "common.h"

#include <vector>
#include <sys/epoll.h>

class Channel;
class Epoller
{
public:
    DISALLOW_COPY_AND_MOVE(Epoller);

    Epoller();
    ~Epoller();

    // ���¼�����channel
    void UpdateChannel(Channel *ch) const;
    // ɾ��������ͨ��
    void DeleteChannel(Channel *ch) const;

    // ���ص�����epoll_wait��ͨ���¼�
    std::vector<Channel *> Poll(long timeout = -1) const;

    private:
        int fd_;
        struct epoll_event *events_;
};
#endif // EPOLLER_H
