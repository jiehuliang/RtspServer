#ifndef ASYNC_LOGGING_H
#define ASYNC_LOGGING_H

#include <vector>
#include <memory>

#include <thread>
#include "Latch.h"
#include "Logging.h"

static const double BufferWriteTimeout = 3.0;//�ȴ�д���ʱ��
static const int64_t FileMaximumSize = 1024 * 1024 * 1024;//�����ļ���������
class AsyncLogging {
public:
	
private:

};

#endif //ASYNC_LOGGING_H