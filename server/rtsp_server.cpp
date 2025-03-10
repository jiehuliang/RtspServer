#include "RtspServer.h"
#include "EventLoop.h"
#include <iostream>
#include <csignal>

// 信号处理函数
void ignoreBrokenPipe(int signum) {
    // 什么也不做，忽略 Broken pipe 信号
}

int main(int argc, char* argv[]) {

    // 设置信号处理函数，忽略 SIGPIPE 信号
    signal(SIGPIPE, ignoreBrokenPipe);
    
    int port;
    if (argc <= 1)
    {
        port = 1234;
    }
    else if (argc == 2) {
        port = atoi(argv[1]);
    }
    else {
        printf("error");
        exit(0);
    }
	int size = std::thread::hardware_concurrency() - 1;
	EventLoop* loop = new EventLoop();
	RtspServer* rtspServer = new RtspServer(loop, "172.31.2.240", port, true);
	rtspServer->SetThreadNums(size);
    rtspServer->start();
}