/*
 * @Author: yingxin wang
 * @Date: 2023-05-12 08:12:28
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-15 16:40:40
 * @Description: FileSystem类，相当于FileManager
 */
#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

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
	fstream fd(DISK_PATH, ios::out);
	fd.close();
	fd.open(DISK_PATH, ios::out | ios::in | ios::binary);
	// 如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open())
	{
		cout << "无法打开一级文件myDisk.img" << endl;
		throw(errno);
	}

	// 先对缓存相关内容进行初始化
	this->bufManager = new BufferManager();
	this->spb = new SuperBlock();
	this->userTable = new UserTable();

	// 才能对superblock进行初始化，因为会调用函数
	this->spb->Init();

	// 分配一个空闲的外存Inode来存放根目录
	Inode *pInode = this->IAlloc();
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

/// @brief 从外存读取指定外存Inode到内存中
/// @param inumber 外存Inode编号
/// @return Inode* 内存Inode拷贝
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
		cout << "内存InodeTable已满" << endl;
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

/// @brief 根据文件路径查找对应的Inode
/// @param path 文件路径
/// @return Inode* 返回对应的Inode，如果没有找到，返回NULL
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
		if (ipaths == paths.size()) // 这种情况说明找到了对应的文件或目录
			break;
		else if (ipaths >= paths.size())
			return NULL;

		// 如果现有的Inode是目录文件才正确,因为在这里面的循环才会找到文件/目录
		// 一旦找到文件/目录不会进入这个循环
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
		else if (mode == FileMode::WRITE) // 写权力前提是有读权力
			return (pInode->i_mode & Inode::INodeMode::OWNER_R) && (pInode->i_mode & Inode::INodeMode::OWNER_W);
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
		else if (mode == FileMode::WRITE) // 写权力前提是有读权力
			return (pInode->i_mode & Inode::INodeMode::GROUP_R) && (pInode->i_mode & Inode::INodeMode::GROUP_W);
		else if (mode == FileMode::READ)
			return pInode->i_mode & Inode::INodeMode::GROUP_R;
		else
			return 0;
	}

	// 如果是其他用户
	if (mode == FileMode::EXC)
		return pInode->i_mode & Inode::INodeMode::OTHER_X;
	else if (mode == FileMode::WRITE) // 写权力前提是有读权力
		return (pInode->i_mode & Inode::INodeMode::GROUP_R) && (pInode->i_mode & Inode::INodeMode::OTHER_W);
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
int FileSystem::fopen(string path)
{
	if (path.empty())
	{
		cout << "路径无效!" << endl;
		throw(EINVAL);
		return -1;
	}
	Inode *pInode;

	// 找到相应的Inode
	pInode = this->NameI(path);

	// 没有找到相应的Inode
	if (NULL == pInode)
	{
		cout << "没有找到对应的文件或目录!" << endl;
		throw(ENOENT);
		return -1;
	}

	// 如果找到，判断是否有权限打开文件
	if (this->Access(pInode, FileMode::EXC) == 0)
	{
		cout << "没有权限打开文件!" << endl;
		throw(EACCES);
		return -1;
	}

	// 当有权限打开文件时，在openFileTable中添加文件
	int fd = this->AddFileinFileTable(pInode);

	// 如果添加失败，说明打开文件过多
	if (fd == -1)
	{
		cout << "已打开过多文件!" << endl;
		throw(ENFILE);
		return -1;
	}
	// 添加成功并返回文件描述符
	return fd;
}

/// @brief 分配一个空闲的外存Inode
/// @return Inode* 返回分配到的内存Inode，如果分配失败，返回NULL
Inode *FileSystem::IAlloc()
{
	Buf *pBuf;
	Inode *pNode;
	int ino = 0;				  // 分配到的空闲外存Inode编号
	SuperBlock *ispb = this->spb; // 获取副本，防止有操作失误

	// SuperBlock直接管理的空闲Inode索引表已空
	if (ispb->s_ninode <= 0)
	{
		// 依次读入磁盘Inode区中的磁盘块，搜索其中空闲外存Inode，记入空闲Inode索引表
		for (int i = 0; i < ispb->s_isize; i++)
		{
			pBuf = this->bufManager->Bread(POSITION_DISKINODE + i / NUM_INODE_PER_BLOCK);

			// 获取缓冲区首址
			int *p = (int *)pBuf->b_addr;

			// 检查该缓冲区中每个外存Inode的i_mode != 0，表示已经被占用
			for (int j = 0; j < NUM_INODE_PER_BLOCK; j++)
			{
				ino++;
				int mode = *(p + j * sizeof(DiskInode) / sizeof(int));

				// 该外存Inode已被占用，不能记入空闲Inode索引表
				if (mode != 0)
				{
					continue;
				}

				/*
				 * 如果外存inode的i_mode==0，此时并不能确定
				 * 该inode是空闲的，因为有可能是内存inode没有写到
				 * 磁盘上,所以要继续搜索内存inode中是否有相应的项
				 */
				if (this->IsLoaded(ino) == -1)
				{
					// 该外存Inode没有对应的内存拷贝，将其记入空闲Inode索引表
					ispb->s_inode[spb->s_ninode++] = ino;

					/* 如果空闲索引表已经装满，则不继续搜索 */
					if (ispb->s_ninode >= 100)
					{
						break;
					}
				}
			}

			// 如果空闲索引表已经装满，则不继续搜索
			if (ispb->s_ninode >= 100)
			{
				break;
			}
		}
	}

	// 如果这样了还没有可用外存Inode，返回NULL
	if (ispb->s_ninode <= 0)
	{
		cout << "磁盘上外存Inode区已满!" << endl;
		throw(ENOSPC);
		return NULL;
	}

	// 现在分配内存Inode
	int inumber = ispb->s_inode[--ispb->s_ninode];
	pNode = IGet(inumber);
	if (NULL == pNode) // 不做修改操作
		return NULL;

	if (0 == pNode->i_mode)
	{
		pNode->Clean();
		return pNode;
	}
}

/// @brief 创建文件
/// @param path 文件路径
/// @return int 创建成功为0，否则为-1
int FileSystem::fcreate(string path)
{
	vector<string> paths = stringSplit(path, '/');
	string name = paths[paths.size() - 1];
	if (name.size() > NUM_FILE_NAME)
	{
		cout << "文件名过长!" << endl;
		throw(ENAMETOOLONG);
		return -1;
	}

	Inode *pInode;

	// 从路径中删除文件名
	path.erase(path.size() - name.size(), name.size());
	// 找到想要创建的文件的父文件夹相应的Inode
	pInode = this->NameI(path);

	// 没有找到相应的Inode
	if (pInode == NULL)
	{
		cout << "没有找到对应的文件或目录!" << endl;
		throw(ENOENT);
		return -1;
	}

	// 如果找到，判断创建的文件的父文件夹是不是文件夹类型
	if (pInode->i_mode & Inode::INodeMode::IDIR)
	{
		cout << "不是一个正确的目录项!" << endl;
		throw(ENOTDIR);
		return -1;
	}

	// 如果找到，判断是否有权限写文件
	if (this->Access(pInode, FileMode::WRITE) == 0)
	{
		cout << "没有权限写文件!" << endl;
		throw(EACCES);
		return -1;
	}

	bool isFull = true;
	int iinDir = 0;
	// 当有权限写文件时，判断是否有重名文件而且查看是否有空闲的子目录可以写
	// 计算要读的物理盘块号
	// 由于目录文件只占一个盘块，所以只有一项不为空
	int blkno = pInode->Bmap(0);
	// 读取磁盘的数据
	Buf *pbuf = this->bufManager->Bread(blkno);
	// 将数据转为目录结构
	Directory *dirPtr = char2Directory(pbuf->b_addr);
	// 循环查找目录中的每个元素
	for (int i = 0; i < NUM_SUB_DIR; i++)
	{
		// 如果找到对应子目录
		if (name == dirPtr->d_filename[i])
		{
			cout << "文件已存在!" << endl;
			throw(EEXIST);
			return -1;
		}
		if (isFull && dirPtr->d_inodenumber[i] == 0)
		{
			isFull = false;
			iinDir = i;
		}
	}

	// 如果目录已满
	if (isFull)
	{
		cout << "目录已满!" << endl;
		throw(ENOSPC);
		return -1;
	}

	// 这才开始创建新的文件
	// 分配一个新的内存Inode
	Inode *newinode = this->IAlloc();
	newinode->i_mode = Inode::INodeMode::IFILE |
					   Inode::INodeMode::OWNER_R | Inode::INodeMode::OWNER_W | Inode::INodeMode::OWNER_X |
					   Inode::INodeMode::GROUP_R | Inode::INodeMode::GROUP_X |
					   Inode::INodeMode::OTHER_R | Inode::INodeMode::OTHER_X;
	newinode->i_nlink = 1;
	newinode->i_uid = this->curId;
	newinode->i_gid = this->userTable->GetGId(this->curId);
	newinode->i_size = 0;
	newinode->i_mtime = unsigned int(time(NULL));
	newinode->i_atime = unsigned int(time(NULL));

	// 将新的Inode写回磁盘中
	newinode->WriteI();

	// 将文件写入目录项中
	dirPtr->d_inodenumber[iinDir] = newinode->i_number;
	strcpy(dirPtr->d_filename[iinDir], name.c_str());
	// 将父目录写回磁盘中，因为一个目录项就是一个盘块大小，直接修改了b_addr，其实并不安全
	pbuf->b_addr = directory2Char(dirPtr);
	this->bufManager->Bwrite(pbuf);

	return 0;
}