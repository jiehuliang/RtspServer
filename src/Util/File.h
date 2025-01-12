#ifndef FILE_H
#define FILE_H

#include <string>

class File {
public:
	//����·��
	//Create path
	static bool create_path(const std::string& file, unsigned int mod);

	//�½��ļ���Ŀ¼�ļ����Զ�����
	//Create a new file,and the directory folder will be generated automatically
	static FILE* create_file(const std::string& file, const std::string& mode);

	/**
	 * �����ļ�������string
	 * @param path ���ص��ļ�·��
	 * @return �ļ�����
	 * Load file content to string
	 * @param path The path of the file to load
	 * @return The file content
	 */
	static std::string loadFile(const std::string& path);

	/**
	 * �����������ļ�
	 * @param data �ļ�����
	 * @param path ������ļ�·��
	 * @return �Ƿ񱣴�ɹ�
	 * Save content to file
	 * @param data The file content
	 * @param path The path to save the file
	 * @return Whether the save was successful
	 */
	static bool saveFile(const std::string& data, const std::string& path);


private:
	File();
	~File();
};

#endif //FILE_H	