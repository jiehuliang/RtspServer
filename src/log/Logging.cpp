#include "Logging.h"
#include "CurrentThread.h"

//Ϊ��ʵ�ֶ��߳�����־ʱ���ʽ����Ч�ʣ�����������_thread������
//���ڻ��浱ǰ�̴߳�����ʱ���ַ�������һ����־��¼������
_thread char t_time[64];		//��ǰ�̴߳����ڵ�ʱ���ַ��� ����:��:�� ʱ:��:�롱
_thread time_t t_lastsecond;	//��ǰ�߳���һ�μ�¼��־������

//����һ����֪���ȵ��ַ���������buffer��
class Template {
public:
	Template(const char**  str, unsigned int len)
		: str_(str),
		  len_(len) {}
	const char* str_;
	const unsigned int len_;
};

