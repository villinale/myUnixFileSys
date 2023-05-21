#include "../h/header.h"
#include "../h/Utility.h"

FileSystem fs;

int main()
{
	cout << "欢迎来到myUnixFile系统!" << endl;
	cout << "       /\\_/\\ " << endl;
	cout << "     ( > o < )" << endl;
	cout << "      \\_____/" << endl;
	cout << "初始root用户名是root，密码也是root" << endl;
	cout << "初始普通用户名是unix，密码也是1" << endl;
	cout << "自己试试看吧!" << endl;

	fstream fd;
	fd.open(DISK_PATH, ios::in);
	if (!fd.is_open())
	{
		printf("文件系统不存在，正在进行初始化\n");
		fs.fformat();
	}
	else
	{
		cout << "文件系统已存在，正在加载" << endl;
		//fs.init();
		fs.fformat();
		fs.exit();
	}

	fs.login();
	fs.fun();

	return 0;
}