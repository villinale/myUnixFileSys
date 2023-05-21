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
	
	//fs.init();
	fs.exit();
}

int main()
{
	test();

	fstream fd;
	fd.open(DISK_PATH, ios::out | ios::in | ios::binary);

	SuperBlock* sp;
	char* ch = new char[1000];
	fd.seekg(0, ios::beg);
	fd.read(ch, sizeof(SuperBlock));
	sp = reinterpret_cast<SuperBlock *>(ch);
	
	return 0;
}