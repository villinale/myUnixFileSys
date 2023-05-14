#include "../h/header.h"
#include "../h/Utility.h"

void test()
{
	cout << "User:" << sizeof(UserTable) << endl;
	cout << "DiskInode:" << sizeof(DiskInode) << endl;
	cout << "Inode:" << sizeof(Inode) << endl;
	cout << "SuperBlock:" << sizeof(SuperBlock) << endl;
	cout << "Directory:" << sizeof(Directory) << endl;

	Directory dir;

	for (int i = 0; i < NUM_SUB_DIR; i++)
	{
		dir.d_inodenumber[i] = i;
		strcpy(dir.d_filename[i], "test");
	}

	for (int i = 0; i < NUM_SUB_DIR; i++)
		cout << dir.d_inodenumber[i] << " " << dir.d_filename[i] << endl;

	char *ch = directory2Char(&dir);
	cout << ch << endl;

	Directory *dirPtr = char2Directory(ch);
	for (int i = 0; i < NUM_SUB_DIR; i++)
		cout << dirPtr->d_inodenumber[i] << " " << dirPtr->d_filename[i] << endl;
}

int main()
{
	test();
	
	return 0;
}