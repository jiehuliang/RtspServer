#include "Logging.h"
#include "CurrentThread.h"

//Ϊ��ʵ�ֶ��߳�����־ʱ���ʽ����Ч�ʣ�����������_thread������
//���ڻ��浱ǰ�̴߳�����ʱ���ַ�������һ����־��¼������
__thread char t_time[64];		//��ǰ�̴߳����ڵ�ʱ���ַ��� ����:��:�� ʱ:��:�롱
__thread time_t t_lastsecond;	//��ǰ�߳���һ�μ�¼��־������

//����һ����֪���ȵ��ַ���������buffer��
class Template {
public:
	Template(const char*  str, unsigned int len)
		: str_(str),
		  len_(len) {}
	const char* str_;
	const unsigned int len_;
};

//�����������ʹLogStream���Դ���Template���͵�����
inline LogStream& operator<<(LogStream& s, Template v) {
	s.append(v.str_, v.len_);
	return s; 
}

//�����������ʹLogStream���Դ���SourceFile���͵�����
inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v) {
	s.append(v.data_, v.size_);
	return s;
}

Logger::SourceFile::SourceFile(const char* data) : data_(data), size_(static_cast<int>(strlen(data_))) 
{
	const char* forward_slash = strrchr(data, '/');
	if (forward_slash) 
	{
		data_ = forward_slash + 1;
		size_ -= static_cast<int>((data_ - data));
	}
}

Logger::Impl::Impl(Logger::LogLevel level, const Logger::SourceFile& source, int line) 
	: level_(level),
	  sourcefile_(source),
	  line_(line){

	FormattedTime();
	CurrentThread::tid();

	stream_ << Template(CurrentThread::tidString(), CurrentThread::tidStringLength());
	stream_ << Template(loglevel(), 6);
}

void Logger::Impl::FormattedTime() {
	//��ʽ�����ʱ��
	TimeStamp now = TimeStamp::Now();
	time_t seconds = static_cast<time_t>(now.microseconds() / kMicrosecond2Second);
	int microseconds = static_cast<int>(now.microseconds() % kMicrosecond2Second);

	//�����־��¼��ʱ�䣬�������ͬһ�룬�����ʱ�䡣

	if (t_lastsecond != seconds) 
	{
		struct tm tm_time;
		localtime_r(&seconds, &tm_time);
		snprintf(t_time, sizeof(t_time), "%4d%02d%02d-%02d:%02d:%02d",
				tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
				tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
		t_lastsecond = seconds;
	}
	Fmt us(".%06dZ ", microseconds);
	stream_ << Template(t_time, 17) << Template(us.data(), 9);
}

void Logger::Impl::Finish() {
	stream_ << " - " << sourcefile_.data_ << ":" << line_ << "\n";
}

LogStream& Logger::Impl::stream() { return stream_; }

const char* Logger::Impl::loglevel() const {
	switch (level_) {
		case DEBUG:
			return "DEBUG ";
		case INFO:
			return "INFO  ";
		case WARN:
			return "WARN  ";
		case ERROR:
			return "ERROR ";
		case FATAL:
			return "FATAL ";
	}
	return nullptr;
}

void defaultOutput(const char* msg, int len) {
	fwrite(msg, 1, len, stdout);	// Ĭ��д����stdout
}

void defaultFlush() {
	fflush(stdout);		// Ĭ��flush��stdout
}

//����Ĭ��ֵ
Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
Logger::LogLevel g_logLevel = Logger::LogLevel::INFO;

Logger::Logger(const char* file_, int line, Logger::LogLevel level)
	: impl_(level, file_, line) {};

Logger::~Logger() {
	impl_.Finish();	// ����Դ����λ�ú�����
	const LogStream::Buffer& buf(stream().buffer());	// ��ȡ������
	g_output(buf.data(), buf.len());	 // Ĭ�������stdout

	//����־����ΪFATALʱ��flush�豸����������ֹ����
	if (impl_.level_ == FATAL) {
		g_flush();
		abort();
	}
}

LogStream& Logger::stream() { return impl_.stream(); }

void Logger::setOutput(Logger::OutputFunc func) {
	g_output = func;
}

void Logger::setFlush(Logger::FlushFunc func) {
	g_flush = func;
}

void Logger::setLogLevel(Logger::LogLevel level) {
	g_logLevel = level;
}