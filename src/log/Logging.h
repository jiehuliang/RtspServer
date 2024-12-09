#ifndef LOGGING_H
#define LOGGING_H

#include <string.h>
#include "common.h"
#include "TimeStamp.h"
#include "LogStream.h"

class Logger {
public:
	DISALLOW_COPY_AND_MOVE(Logger);
	enum LogLevel 
	{
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL
	};

	//����������Դ�ļ���
	class SourceFile {
	public:
		SourceFile(const char* data);
		const char* data_;
		int size_;
	};

	//���캯������Ҫ�����ڹ���Impl
	Logger(const char* file_, int line, LogLevel level);
	~Logger();

	//������־�꣬����Impl�������
	LogStream& stream();

	//ȫ�ַ�����������־ȫ����־����flush���Ŀ�ĵ�
	static LogLevel logLevel();
	static void setLogLevel(LogLevel level);

	using OutputFunc = void(*)(const char* data, int  len);// ���庯��ָ��
	using FlushFunc = void(*)();
	//Ĭ��fwrite��stdout
	static void setOutput(OutputFunc);
	//Ĭ��flush��stdout
	static void setFlush(FlushFunc);

private:
	class Impl {
	public:
		DISALLOW_COPY_AND_MOVE(Impl);
		using LogLevel = Logger::LogLevel;
		Impl(Logger::LogLevel level, const SourceFile& source, int line);
		void FormattedTime();//��ʽ��ʱ����Ϣ
		void Finish();// ��ɸ�ʽ�������������Դ���ļ���Դ��λ��

		LogStream& stream();
		const char* loglevel() const;//��ȡLogLevel���ַ���
		LogLevel level_;
	private:
		Logger::SourceFile sourcefile_;//Դ��������
		int line_;//Դ��������
		LogStream stream_;//��־������ 
	};
	Impl impl_;
};

//ȫ�ֵ���־���𣬾�̬��Ա�������壬��̬��Ա����ʵ��
extern Logger::LogLevel g_logLevel;
inline Logger::LogLevel Logger::logLevel() {
	return g_logLevel;
}

//��־��
#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
	Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
	Logger(__FILE__, __LINE__, Logger::INFO).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()

#endif //LOGGING_H