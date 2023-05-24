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
	cout << "自己试试看吧!" << endl
		<< endl;
	/*
	fstream fd;
	fd.open(DISK_PATH, ios::in);
	if (!fd.is_open())
	{
		printf("文件系统不存在，正在进行初始化\n\n");
		// 你问我为什么cout和printf混着用，因为VS的编码导致一些中文字符用cout输出会报错
	}
	else
	{
		cout << "文件系统已存在，正在加载" << endl;
		// fs.fformat();
		fs.init();
	}*/

	fs.fformat();
	fs.exit();
	fs.fun();

	return 0;
}