#include "../h/header.h"
#include "../h/Utility.h"

FileSystem fs;

int main()
{
	cout << "��ӭ����myUnixFileϵͳ!" << endl;
	cout << "       /\\_/\\ " << endl;
	cout << "     ( > o < )" << endl;
	cout << "      \\_____/" << endl;
	cout << "��ʼroot�û�����root������Ҳ��root" << endl;
	cout << "��ʼ��ͨ�û�����unix������Ҳ��1" << endl;
	cout << "�Լ����Կ���!" << endl
		<< endl;
	/*
	fstream fd;
	fd.open(DISK_PATH, ios::in);
	if (!fd.is_open())
	{
		printf("�ļ�ϵͳ�����ڣ����ڽ��г�ʼ��\n\n");
		// ������Ϊʲôcout��printf�����ã���ΪVS�ı��뵼��һЩ�����ַ���cout����ᱨ��
	}
	else
	{
		cout << "�ļ�ϵͳ�Ѵ��ڣ����ڼ���" << endl;
		// fs.fformat();
		fs.init();
	}*/

	fs.fformat();
	fs.exit();
	fs.fun();

	return 0;
}