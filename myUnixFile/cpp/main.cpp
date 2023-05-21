#include "../h/header.h"
#include "../h/Utility.h"

FileSystem fs;

int main()
{
	cout << "欢迎来到myUnixFile系统！" << endl;
	cout << "      /\\_/\\ " << endl;
	cout << "     ( >^_^< )" << endl;
	cout << "      \_____/" << endl;

	fstream fd;
	fd.open(DISK_PATH, ios::in);
	if (!fd.is_open())
	{
		cout << "文件系统不存在，正在进行初始化" << endl;
		fs.fformat();
	}
	else
	{
		cout << "文件系统已存在，正在加载" << endl;
		fs.init();
	}

	fs.login();
	cout << "输入help可以查看命令清单" << endl;
	fs.fun();

	return 0;
}