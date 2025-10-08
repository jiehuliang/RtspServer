#include "Rtsp/RtspPlayer.h"
#include "Event/EventLoop.h"
#include "Thread/EventLoopThreadPool.h"
#include "HooLog/HooLog.h"
#include <iostream>
#include <csignal>

void ignoreBrokenPipe(int signum) {
}

int main(int argc, char* argv[]) {
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
	setLogLevel(loglevel::DEBUG);

	int size = std::thread::hardware_concurrency() - 1;
	std::unique_ptr<EventLoopThreadPool> event_loop_thread_pool = std::unique_ptr<EventLoopThreadPool>(new EventLoopThreadPool(nullptr));
	event_loop_thread_pool->SetThreadNums(size);
	event_loop_thread_pool->start();
	std::shared_ptr<RtspClient> rtsp_client = std::make_shared<RtspClient>(event_loop_thread_pool->nextloop(),"172.31.2.240",port);
	rtsp_client->Start();
	while (1) {
		//rtsp_client->Write("hello !!!!!!");
		sleep(1);

	}
}