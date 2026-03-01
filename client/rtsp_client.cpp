#include "Rtsp/RtspClient.h"
#include "Event/EventLoop.h"
#include "Thread/EventLoopThread.h"
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

	std::unique_ptr<EventLoopThread> ptr = std::unique_ptr<EventLoopThread>(new EventLoopThread());
	ptr->StartLoop();
	std::shared_ptr<RtspClient> rtsp_client = std::make_shared<RtspClient>(ptr,"172.31.2.240",port);
	rtsp_client->Start();
	while (1) {
		//rtsp_client->Write("hello !!!!!!");
		sleep(1);

	}
}