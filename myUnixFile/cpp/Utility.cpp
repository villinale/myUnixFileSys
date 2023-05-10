#include "../h/header.h"
#include "../h/Utility.h"

/*
* 定义一些工具类和工具函数
*/

//全局初始化
void Utility::Init()
{
	fstream fd(DISK_PATH, ios::out);
	fd.close();

	fd.open(DISK_PATH, ios::out | ios::in | ios::binary);
	if (!fd.is_open()) {
		cout << "无法打开一级文件myDisk.img" << endl;
		throw(ENOENT);
	}

	AddUser(ROOT_ID, "root", "root", ROOT_GID);

}

//读User块
void Utility::ReadUser(User& user)
{
	fstream fd;
	fd.open(DISK_PATH, ios::out | ios::in | ios::binary);
	if (!fd.is_open()) {
		cout << "无法打开一级文件myDisk.img" << endl;
		throw(ENOENT);
	}

	fd.seekg(USER_POSITION, ios::beg);
	fd.read((char*)&user, sizeof(user));
	fd.close();
}

//写User块
void Utility::WriteUser(User user)
{
	fstream fd;
	fd.open(DISK_PATH, ios::out | ios::in | ios::binary);
	if (!fd.is_open()) {
		cout << "无法打开一级文件myDisk.img" << endl;
		throw(ENOENT);
	}

	fd.seekg(USER_POSITION, ios::beg);
	fd.write((char*)&user, sizeof(user));
	fd.close();
}