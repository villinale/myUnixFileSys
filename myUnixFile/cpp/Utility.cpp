#include "../h/header.h"
#include "../h/Utility.h"

/*
* ����һЩ������͹��ߺ���
*/

//ȫ�ֳ�ʼ��
void Utility::Init()
{
	fstream fd(DISK_PATH, ios::out);
	fd.close();

	fd.open(DISK_PATH, ios::out | ios::in | ios::binary);
	if (!fd.is_open()) {
		cout << "�޷���һ���ļ�myDisk.img" << endl;
		throw(ENOENT);
	}

	AddUser(ROOT_ID, "root", "root", ROOT_GID);

}

//��User��
void Utility::ReadUser(User& user)
{
	fstream fd;
	fd.open(DISK_PATH, ios::out | ios::in | ios::binary);
	if (!fd.is_open()) {
		cout << "�޷���һ���ļ�myDisk.img" << endl;
		throw(ENOENT);
	}

	fd.seekg(USER_POSITION, ios::beg);
	fd.read((char*)&user, sizeof(user));
	fd.close();
}

//дUser��
void Utility::WriteUser(User user)
{
	fstream fd;
	fd.open(DISK_PATH, ios::out | ios::in | ios::binary);
	if (!fd.is_open()) {
		cout << "�޷���һ���ļ�myDisk.img" << endl;
		throw(ENOENT);
	}

	fd.seekg(USER_POSITION, ios::beg);
	fd.write((char*)&user, sizeof(user));
	fd.close();
}