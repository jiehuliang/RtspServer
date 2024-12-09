#ifndef LOG_STREAM_H
#define LOG_STREAM_H

#include "common.h"
#include <string>
#include <algorithm>
#include <cstring>
#include <assert.h>
#include <iostream>

static const int FixedBufferSize = 4096;
static const int kMaxNumericSize = 48;
class FixedBuffer {
public:
	FixedBuffer();
	~FixedBuffer();

	void append(const char* buf,int len);
	const char* data() const;
	int len() const;

	char* current();
	int avail() const; //ʣ��Ŀ��ÿռ�
	void add(int len);

	void reset();//���û�����
	const char* end() const;//��ȡĩ�˵�ַ
	void clear();
private:
	char data_[FixedBufferSize];
	char* cur_;
};

class LogStream {
public:
	DISALLOW_COPY_AND_MOVE(LogStream);
	using self = LogStream;
	using Buffer = FixedBuffer;

	LogStream();
	~LogStream();

	void append(const char* data, int len);
	const Buffer& buffer() const;
	void resetBuffer();

	self &operator<<(bool v);
	//�������ݵ��ַ�ת�������浽���������ڲ�������void formatInteger(T val);����
	self& operator<<(short num);
	self& operator<<(unsigned short num);
	self& operator<<(int num);
	self& operator<<(unsigned int num);
	self& operator<<(long num);
    self& operator<<(unsigned long num);
	self& operator<<(long long num);
	self& operator<<(unsigned long long num);

	//����������ת�������浽������
	self& operator<<(const float& num);
	self& operator<<(const double& num);

	self& operator<<(char v);
	//ԭ���ַ��������������
	self& operator<<(const char* str);

	//��׼�ַ���std::string�����������
	self& operator<<(const std::string& v);
private:
	template <typename T>
	void formatInteger(T value);

	Buffer buffer_;
};

template <typename T>
void LogStream::formatInteger(T value) {
	if (buffer_.avail() >= kMaxNumericSize) {
		char* buf = buffer_.current();
		char* now = buf;

		do {
			int remainder = value % 10;
			*(now++) = remainder + '0'; //�����ַ�'0'��ASCIIֵ���õ���Ӧ���ֵ��ַ���ʾ
			value /= 10;
		} while (value != 0);
		if (value < 0) {
			*(now++) = '-';
		}
		std::reverse(buf, now);//ʹ��std::reverse(buf, now)������ת������ַ����У�ʹ���Ϊ����
		buffer_.add(now - buf);
	}
}

class Fmt {
public:
	template<typename T>
	Fmt(const char* fmt, T val);

	const char* data() const { return buf_; }
	
	int length() const { return length_; }
private:
	char buf_[32];
	int length_;
};

template<typename T>
Fmt::Fmt(const char* fmt,T val) {
	static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");
	
	length_ = snprintf(buf_,sizeof(buf_),fmt,val);
	assert(static_cast<size_t>(length_) < sizeof(buf_));
}

inline LogStream& operator<<(LogStream& s,const Fmt& fmt) {
	s.append(fmt.data(),fmt.length());
	return s;
}

// Explicit instantiations

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);


#endif // LOG_STREAM_H