#include "Buffer.h"
#include <string>
#include <assert.h>
#include <cstring>

Buffer::Buffer()
	: buffer_(kInitalSize),
	read_index_(kPrePendIndex),
	write_index_(kPrePendIndex) {}

Buffer::~Buffer() {}

Buffer::Buffer(Buffer&& buf) 
	:buffer_(std::move(buf.buffer_)), 
	read_index_(buf.read_index_), 
	write_index_(buf.write_index_) {}

Buffer& Buffer::operator=(Buffer&& buf) {
	if (this != &buf) {
		buffer_.clear();
		read_index_ = kPrePendIndex;
		write_index_ = kPrePendIndex;

		buffer_ = std::move(buf.buffer_);
		read_index_ = buf.read_index_;
		write_index_ = buf.write_index_;
	}
	return *this;
}

char* Buffer::begin() { return &*buffer_.begin(); }
const char* Buffer::begin() const { return &*buffer_.begin(); }
char* Buffer::beginread() { return begin() + read_index_; }
const char* Buffer::beginread() const { return begin() + read_index_; }
char* Buffer::beginwrite() { return begin() + write_index_; }
const char* Buffer::beginwrite() const { return begin() + write_index_; }

void Buffer::AppendPrepend(const  char* message, int len) {
	if (len > prependablebytes()) {
		return;
	}
	std::copy(message, message + len, beginread() - len);
	read_index_ -= len;
}

void Buffer::Append(const char* message) {
	Append(message, static_cast<int>(strlen(message)));
}

void Buffer::Append(const  char* message, int len) {
	EnsureWriteableBytes(len);
	std::copy(message, message + len, beginwrite());
	write_index_ += len;
}

void Buffer::Append(const std::string& message) {
	Append(message.data(), static_cast<int>(message.size()));
}

int Buffer::readablebytes() const { return write_index_ - read_index_; }
int Buffer::writeablebytes() const { return static_cast<int>(buffer_.size()) - write_index_; }
int Buffer::prependablebytes() const { return read_index_; }

char* Buffer::Peek() { return beginread(); }
const char* Buffer::Peek() const { return beginread(); }

std::string Buffer::PeekAsString(int len) {
	return std::string(beginread(), beginread() + len);
}

std::string Buffer::PeekAllAsString() {
	return std::string(beginread(), beginwrite());
}

void Buffer::Retrieve(int len) {
	assert(readablebytes() >= len);
	if (len + read_index_ < write_index_) {
		//����������ݲ������ɶ��ռ䣬��ֻ�ø���read_index_
		read_index_ += len;
	}
	else {
		//����������ö��꣬��Ҫͬʱ����write_index_;
		RetrieveAll();
	}
}

void Buffer::RetrieveAll() {
	write_index_ = kPrePendIndex;
	read_index_ = write_index_;
}

void Buffer::RetrieveUtil(const char* end) {
	//��֤û�н����д����
	assert(beginwrite() >= end);
	read_index_ += static_cast<int>(end - beginread());
}

std::string Buffer::RetrieveAsString(int len) {
	assert(read_index_ + len <= write_index_);

	std::string ret = std::move(PeekAsString(len));
	Retrieve(len);
	return ret;
}

std::string  Buffer::RetrieveUtilAsString(const char* end) {
	assert(beginwrite() >= end);
	std::string ret = std::move(PeekAsString(static_cast<int>(end - beginread())));
	RetrieveUtil(end);
	return ret;
}

std::string Buffer::RetrieveAllAsString() {
	assert(readablebytes() > 0);
	std::string ret = std::move(PeekAllAsString());
	RetrieveAll();
	return ret;
}

void Buffer::EnsureWriteableBytes(int len) {
	if (writeablebytes() >= len)
		return;
	if (writeablebytes() + prependablebytes() >= kPrePendIndex + len) {
		//�����ʱwriteable��prependable��ʣ��ռ䳬��д�ĳ��ȣ����Ƚ��������ݸ��Ƶ���ʼλ�ã�
		//�����϶����µ�read_index_����ʹǰ��û�����õĿռ������ϡ�
		std::copy(beginread(), beginwrite(), begin() + kPrePendIndex);
		write_index_ = kPrePendIndex + readablebytes();
		read_index_ = kPrePendIndex;
	}
	else {
		buffer_.resize(write_index_ + len);
	}
}