#include "../h/header.h"

void fun()
{
	cout << "User:" << sizeof(User) << endl;
	cout << "DiskInode:" << sizeof(DiskInode) << endl;
	cout << "Inode:" << sizeof(Inode) << endl;
	cout << "SuperBlock:" << sizeof(SuperBlock) << endl;
	cout << "Directory:" << sizeof(Directory) << endl;
}

int main()
{
	fun();


	return 0;

}