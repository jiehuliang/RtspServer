#include "AsyncLogging.h"
#include  "LogFile.h"

#include <vector>
#include <memory>
#include <functional>
#include <chrono>

AsyncLogging::AsyncLogging(const char* filepath) 
	:running_(false),
	filepath_(filepath),
	latch_(1){
	current_ = std::unique_ptr<Buffer>(new Buffer());
	next_ = std::unique_ptr<Buffer>(new Buffer());
}

AsyncLogging::~AsyncLogging() {
	if (running_) {
		Stop();
	}
}

void AsyncLogging::Start() {
	running_ = true;
	thread_ = std::thread(std::bind(&AsyncLogging::ThreadFunc, this));

	//�ȴ��߳��������
	latch_.wait();
}

void AsyncLogging::Stop() {
	running_ = false;
	//���Ѻ���߳�
	conv_.notify_one();
	thread_.join();
}

void AsyncLogging::Flush() {
	fflush(stdout);
}

void AsyncLogging::Append(const char* data, int len) {
	std::unique_lock<std::mutex> lock(mutex_);
	if (current_->avail() >= len) {
		current_->append(data, len);
	}
	else{
		// �����ǰ����û�пռ䣬�ͽ���ǰ������뵽�����б���
		buffers_.push_back(std::move(current_));
		if (next_) {
			current_ = std::move(next_);
		}
		else {
			current_.reset(new Buffer());
		}
		//���µĻ�����д����Ϣ��
		current_->append(data, len);
	}
	// ���Ѻ���߳�
	conv_.notify_one();
}

void AsyncLogging::ThreadFunc() {
	//�����ɹ����������߳�
	latch_.notify();

	std::unique_ptr<Buffer> new_current = std::unique_ptr<Buffer>(new Buffer());
	std::unique_ptr<Buffer> new_next = std::unique_ptr<Buffer>(new Buffer());

	std::unique_ptr<LogFile> logfile = std::unique_ptr<LogFile>(new LogFile());

	new_current->bzero();
	new_next->bzero();

	std::vector<std::unique_ptr<Buffer>> active_buffers;

	while (running_) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (buffers_.empty()) {
			//��������л�û�������Ļ���������ȴ�Ƭ��
			conv_.wait_until(lock, std::chrono::system_clock::now() +
				BufferWriteTimeout * std::chrono::milliseconds(1000),
				[] {return false; });
		}

		//ֱ�ӽ���ǰ���������뵽�����������У�
		buffers_.push_back(std::move(current_));
		active_buffers.swap(buffers_);

		current_ = std::move(new_current);

		if (!next_) {
			next_ = std::move(new_next);
		}

		//д����־�ļ�
		for (const auto& buffer : active_buffers) {
			logfile->Write(buffer->data(), buffer->len());
		}

		if (logfile->writtenbytes() >= FileMaximumSize) {
			//ָ��һ���µ���־�ļ�
			logfile.reset(new LogFile(filepath_));
		}

		if (active_buffers.size() > 2) {
			//��ס���������ں���
			active_buffers.resize(2);
		}

		if (!new_current) {
			new_current = std::move(active_buffers.back());
			active_buffers.pop_back();
			new_current->bzero();
		}
		if (!new_next) {
			new_next = std::move(active_buffers.back());
			active_buffers.pop_back();
			new_next->bzero();
		}

		active_buffers.clear();
	}
}