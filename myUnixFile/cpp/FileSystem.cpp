/*
 * @Author: yingxin wang
 * @Date: 2023-05-12 08:12:28
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-14 16:36:12
 * @Description: FileSystem类，相当于FileManager
 */
#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

vector<string> stringSplit(const string &strIn, char delim)
{
	char *str = const_cast<char *>(strIn.c_str());
	string s;
	s.append(1, delim);
	vector<string> elems;
	char *splitted = strtok(str, s.c_str());
	while (splitted != NULL)
	{
		elems.push_back(string(splitted));
		splitted = strtok(NULL, s.c_str());
	}
	return elems;
}

FileSystem::FileSystem()
{
	this->init();
}

short FileSystem::getCurUserID()
{
	return this->curId;
}

void FileSystem::init()
{
}

/// @brief 判断指定外存Inode是否已经加载到内存中
/// @param inumber 外存Inode编号
/// @return int 如果已经加载，返回内存Inode在inodeTable的编号，否则返回-1
int FileSystem::IsLoaded(int inumber)
{
	// 寻找指定外存Inode的内存inode拷贝
	for (int i = 0; i < NUM_INODE; i++)
		if (this->inodeTable[i].i_number == inumber && this->inodeTable[i].i_count != 0)
			return i;

	return -1;
}

Inode *FileSystem::IGet(int inumber)
{
	Inode *pInode = NULL;
	// 在inodeTable中查找指定外存Inode的内存inode拷贝
	int iInTable = this->IsLoaded(inumber);

	// 如果找到了，直接返回内存inode拷贝，引用计数加1
	if (iInTable != -1)
	{
		pInode = &this->inodeTable[iInTable];
		pInode->i_count++;
		return pInode;
	}

	// 没有找到，从外存读取
	for (int i = 0; i < NUM_INODE; i++)
		// 如果该内存Inode引用计数为零，则该Inode表示空闲，可以使用
		if (this->inodeTable[i].i_count == 0)
			pInode = &(this->inodeTable[i]);

	// 如果内存InodeTable已满，抛出异常
	if (pInode == NULL)
	{
		cout << "内存InodeTable已满！" << endl;
		throw(ENFILE);
		return pInode;
	}

	// 如果内存InodeTabl没满，从外存读取指定外存Inode到内存中
	pInode->i_number = inumber;
	pInode->i_count++;

	// 将该外存Inode读入缓冲区
	Buf *pBuf = this->bufManager->Bread(POSITION_DISKINODE + inumber / NUM_INODE_PER_BLOCK);
	// 将缓冲区中的外存Inode信息拷贝到新分配的内存Inode中
	pInode->ICopy(pBuf, inumber);
	return pInode;
}

Inode *FileSystem::NameI(string path)
{
	Inode *pInode;
	Buf *pbuf;
	Directory dir;
	vector<string> paths = stringSplit(path, '/'); // 所以要求文件夹和文件的名中不能出现"/"
	int ipaths = 0;
	bool isFind = false;

	// 第一个字符为/表示绝对路径
	if (path[0] == '/') // 从根目录开始查找
		pInode = this->rootDirInode;
	else // 相对路径的查找
		pInode = this->curDirInode;

	while (true)
	{
		isFind = false;
		if (ipaths == (paths.size() - 1)) // 这种情况说明找到了对应的文件或目录
			break;
		else if (ipaths >= paths.size())
			return NULL;

		// 如果现有的Inode是目录文件才正确
		if (pInode->i_mode & Inode::INodeMode::IDIR)
		{
			// 计算要读的物理盘块号
			// 由于目录文件只占一个盘块，所以只有一项不为空
			int blkno = pInode->Bmap(0);
			// 读取磁盘的数据
			pbuf = this->bufManager->Bread(blkno);

			// 将数据转为目录结构
			Directory *dirPtr = char2Directory(pbuf->b_addr);

			// 循环查找目录中的每个元素
			for (int i = 0; i < NUM_SUB_DIR; i++)
			{
				// 如果找到对应子目录
				if (paths[ipaths] == dirPtr->d_filename[i])
				{
					ipaths++;
					isFind = true;
					pInode = this->IGet(dirPtr->d_inodenumber[i]);
					break;
				}
			}

			// 如果没有找到对应的文件或目录
			if (!isFind)
				return NULL;
		}
		else // 不是目录文件是错误的
			return NULL;
	}

	// 到这个部分说明找到了对应的文件或者目录
	return pInode;
}

/// @brief 查找pInode是否有给定mode的权限
/// @param pInode 要查找的Inode
/// @param mode   要查找的权限，定义间见FileSystem::FileMode
/// @return       如果有权限，返回1，否则返回0
int FileSystem::Access(Inode *pInode, unsigned int mode)
{
	// 如果是超级用户，直接返回
	if (this->curId == ROOT_ID)
		return 1;

	// 如果是文件所有者
	if (this->curId == pInode->i_uid)
	{
		if (mode == FileMode::EXC)
			return pInode->i_mode & Inode::INodeMode::OWNER_X;
		else if (mode == FileMode::WRITE)
			return pInode->i_mode & Inode::INodeMode::OWNER_W;
		else if (mode == FileMode::READ)
			return pInode->i_mode & Inode::INodeMode::OWNER_R;
		else
			return 0;
	}

	// 如果是文件所有者所在的组
	if (this->curId == pInode->i_gid)
	{
		if (mode == FileMode::EXC)
			return pInode->i_mode & Inode::INodeMode::GROUP_X;
		else if (mode == FileMode::WRITE)
			return pInode->i_mode & Inode::INodeMode::GROUP_W;
		else if (mode == FileMode::READ)
			return pInode->i_mode & Inode::INodeMode::GROUP_R;
		else
			return 0;
	}

	// 如果是其他用户
	if (mode == FileMode::EXC)
		return pInode->i_mode & Inode::INodeMode::OTHER_X;
	else if (mode == FileMode::WRITE)
		return pInode->i_mode & Inode::INodeMode::OTHER_W;
	else if (mode == FileMode::READ)
		return pInode->i_mode & Inode::INodeMode::OTHER_R;
	else
		return 0;
}

/// @brief 在openFileTable中添加一个文件
/// @param pInode 要添加的文件的Inode
/// @return 如果添加成功，返回文件描述符，否则返回-1
int FileSystem::AddFileinFileTable(Inode *pInode)
{
	File *fp = new File;
	fp->f_uid = this->curId;
	fp->f_gid = this->userTable->GetGId(this->curId);
	fp->f_inode = pInode;
	fp->f_offset = 0;

	for (int i = 0; i < NUM_FILE; i++)
	{
		// 进程打开文件描述符表中找到空闲项，则返回之
		if (this->openFileTable[i].f_inode == NULL)
		{
			this->openFileTable[i] = *fp;
			return i;
		}
	}
	return -1;
}

/// @brief 打开文件，由于没有系统调用，所以直接返回文件描述符
/// @param path 文件路径
/// @return int 文件描述符，文件描述符是从0开始的，所以返回值为-1表示打开失败
int FileSystem::Open(string path)
{
	if (path.empty())
	{
		cout << "路径无效！" << endl;
		throw(EINVAL);
		return -1;
	}
	Inode *pInode;

	// 找到相应的Inode
	pInode = this->NameI(path);

	// 没有找到相应的Inode
	if (NULL == pInode)
	{
		cout << "没有找到对应的文件或目录！" << endl;
		throw(ENOENT);
		return -1;
	}

	// 如果找到，判断是否有权限打开文件
	if (this->Access(pInode, FileMode::EXC) == 0)
	{
		cout << "没有权限打开文件！" << endl;
		throw(EACCES);
		return -1;
	}

	// 当有权限打开文件时，在openFileTable中添加文件
	int fd = this->AddFileinFileTable(pInode);

	// 如果添加失败，说明打开文件过多
	if (fd == -1)
	{
		cout << "已打开过多文件！" << endl;
		throw(ENFILE);
		return -1;
	}
	// 添加成功并返回文件描述符
	return fd;
}