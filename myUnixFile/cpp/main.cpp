#include "../h/header.h"
#include "../h/Utility.h"

FileSystem fs;

void test()
{
	cout << "User:" << sizeof(UserTable) << endl;
	cout << "DiskInode:" << sizeof(DiskInode) << endl;
	cout << "Inode:" << sizeof(Inode) << endl;
	cout << "SuperBlock:" << sizeof(SuperBlock) << endl;
	cout << "Directory:" << sizeof(Directory) << endl;
	
	fs.init();
}

int main()
{
	test();
	
	return 0;
}