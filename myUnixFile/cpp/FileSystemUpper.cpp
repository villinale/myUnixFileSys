/*
 * @Author: yingxin wang
 * @Date: 2023-05-12 08:12:28
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-24 19:35:59
 * @Description: FileSystem类中最顶层的各类函数，被Outter文件中的函数调用
 */
#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

/// @brief 创建文件
/// @param path 文件路径
/// @return int 创建成功为0，否则为-1
int FileSystem::fcreate(string path)
{
	vector<string> paths = stringSplit(path, '/');
	if (paths.size() == 0)
	{
		cout << "路径无效!" << endl;
		throw(EINVAL);
		return -1;
	}
	string name = paths[paths.size() - 1];
	if (name.size() > NUM_FILE_NAME)
	{
		cout << "文件名过长!" << endl;
		throw(ENAMETOOLONG);
		return -1;
	}
	else if (name.size() == 0)
	{
		cout << "文件名不能为空!" << endl;
		throw(EINVAL);
		return -1;
	}

	Inode *fatherInode;

	// 从路径中删除文件名
	path.erase(path.size() - name.size(), name.size());
	// 找到想要创建的文件的父文件夹相应的Inode
	fatherInode = this->NameI(path);

	// 没有找到相应的Inode
	if (fatherInode == NULL)
	{
		cout << "没有找到对应的文件或目录!" << endl;
		throw(ENOENT);
		return -1;
	}

	// 如果找到，判断创建的文件的父文件夹是不是文件夹类型
	if (!(fatherInode->i_mode & Inode::INodeMode::IDIR))
	{
		cout << "不是一个正确的目录项!" << endl;
		throw(ENOTDIR);
		return -1;
	}

	// 如果找到，判断是否有权限写文件
	if (this->Access(fatherInode, FileMode::WRITE) == 0)
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
	int blkno = fatherInode->Bmap(0);
	// 读取磁盘的数据
	Buf *fatherBuf = this->bufManager->Bread(blkno);
	// 将数据转为目录结构
	Directory *fatherDir = char2Directory(fatherBuf->b_addr);
	// 循环查找目录中的每个元素
	for (int i = 0; i < NUM_SUB_DIR; i++)
	{
		// 如果找到对应子目录
		if (name == fatherDir->d_filename[i])
		{
			cout << "文件已存在!" << endl;
			throw(EEXIST);
			return -1;
		}
		if (isFull && fatherDir->d_inodenumber[i] == 0)
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
	fatherDir->mkdir(name.c_str(), newinode->i_number);
	// 将父目录写回磁盘中，因为一个目录项就是一个盘块大小，直接修改了b_addr，其实并不安全
	fatherBuf->b_addr = directory2Char(fatherDir);
	this->bufManager->Bwrite(fatherBuf);
	// this->bufManager->bwrite(directory2Char(fatherDir), POSITION_BLOCK + fatherBuf->b_blkno, sizeof(fatherDir));

	// 释放所有Inode
	if (fatherInode != this->rootDirInode && fatherInode != this->curDirInode)
		this->IPut(fatherInode);
	this->IPut(newinode);

	return 0;
}

/// @brief 创建文件夹
/// @param path 文件夹路径
/// @return int 创建成功为0，否则为-1
int FileSystem::mkdir(string path)
{
	vector<string> paths = stringSplit(path, '/');
	if (paths.size() == 0)
	{
		cout << "路径无效!" << endl;
		throw(EINVAL);
		return -1;
	}

	string name = paths[paths.size() - 1];
	if (name.size() > NUM_FILE_NAME)
	{
		cout << "新建目录名过长!" << endl;
		throw(ENAMETOOLONG);
		return -1;
	}
	else if (name.size() == 0)
	{
		cout << "新建目录名不能为空!" << endl;
		throw(EINVAL);
		return -1;
	}

	Inode *fatherInode;

	// 从路径中删除文件夹名
	path.erase(path.size() - name.size(), name.size());
	// 找到想要创建的文件夹名的父文件夹相应的Inode
	fatherInode = this->NameI(path);

	// 没有找到相应的Inode
	if (fatherInode == NULL)
	{
		cout << "没有找到对应的文件或目录!" << endl;
		throw(ENOENT);
		return -1;
	}

	// 如果找到，判断创建的文件夹的父文件夹是不是文件夹类型
	if (!(fatherInode->i_mode & Inode::INodeMode::IDIR))
	{
		cout << "不是一个正确的目录项!" << endl;
		throw(ENOTDIR);
		return -1;
	}

	// 如果找到，判断是否有权限写文件
	if (this->Access(fatherInode, FileMode::WRITE) == 0)
	{
		cout << "没有权限写文件!" << endl;
		throw(EACCES);
		return -1;
	}

	bool isFull = true;
	// 当有权限写文件时，判断是否有重名文件而且查看是否有空闲的子目录可以写
	// 计算要读的物理盘块号
	// 由于目录文件只占一个盘块，所以只有一项不为空
	int blkno = fatherInode->Bmap(0);
	// 读取磁盘的数据
	Buf *fatherBuf = this->bufManager->Bread(blkno);
	// 将数据转为目录结构
	Directory *fatherDir = char2Directory(fatherBuf->b_addr);
	// 循环查找目录中的每个元素
	for (int i = 0; i < NUM_SUB_DIR; i++)
	{
		// 如果找到对应子目录
		if (name == fatherDir->d_filename[i])
		{
			cout << "文件已存在!" << endl;
			throw(EEXIST);
			return -1;
		}
		if (isFull && fatherDir->d_inodenumber[i] == 0)
		{
			isFull = false;
		}
	}

	// 如果目录已满
	if (isFull)
	{
		cout << "目录已满!" << endl;
		throw(ENOSPC);
		return -1;
	}

	// 这才开始创建新的文件夹
	// 分配一个新的内存Inode
	Inode *newinode = this->IAlloc();
	newinode->i_mode = Inode::INodeMode::IDIR |
					   Inode::INodeMode::OWNER_R | Inode::INodeMode::OWNER_W | Inode::INodeMode::OWNER_X |
					   Inode::INodeMode::GROUP_R | Inode::INodeMode::GROUP_X |
					   Inode::INodeMode::OTHER_R | Inode::INodeMode::OTHER_X;
	newinode->i_nlink = 1;
	newinode->i_uid = this->curId;
	newinode->i_gid = this->userTable->GetGId(this->curId);
	newinode->i_size = 0;
	newinode->i_mtime = unsigned int(time(NULL));
	newinode->i_atime = unsigned int(time(NULL));
	// 给新文件夹添加两个目录项
	Directory *newDir = new Directory();
	newDir->mkdir(".", newinode->i_number);		// 创建自己
	newDir->mkdir("..", fatherInode->i_number); // 创建父亲
	// 跟新文件夹分配数据盘块号
	Buf *newBuf = this->Alloc();
	newBuf->b_addr = directory2Char(newDir);
	newinode->i_size = sizeof(Directory) / NUM_SUB_DIR * 2; // 新文件夹大小是两个目录项
	newinode->i_addr[0] = newBuf->b_blkno;
	// delete newDir;

	// 将新文件夹写入其父亲的目录项中
	fatherDir->mkdir(name.c_str(), newinode->i_number);
	fatherInode->i_size += sizeof(Directory) / NUM_SUB_DIR; // 父亲的大小增加一个目录项
	fatherBuf->b_addr = directory2Char(fatherDir);

	// 统一写回：父目录inode，新目录inode，父目录数据块、新目录数据块
	fatherInode->WriteI();
	newinode->WriteI();
	// this->bufManager->bwrite(directory2Char(fatherDir), POSITION_BLOCK + fatherBuf->b_blkno, sizeof(fatherDir));
	// this->bufManager->bwrite(directory2Char(newDir), POSITION_BLOCK + newBuf->b_blkno, sizeof(newDir));

	this->bufManager->Bwrite(fatherBuf);
	this->bufManager->Bwrite(newBuf);
	// 除FS里的rootInode和curInode外释放所有Inode
	if (fatherInode != this->rootDirInode && fatherInode != this->curDirInode)
		this->IPut(fatherInode);
	this->IPut(newinode);

	return 0;
}

/// @brief 退出系统
void FileSystem::exit()
{
	// 将superblock写回磁盘
	this->WriteSpb();
	// TODO:将userTable写回磁盘

	// 将所有标记延迟写的内容都写回磁盘
	this->bufManager->SaveAll();
}

/// @brief 初始化文件系统
void FileSystem::fformat()
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
	fd.close();

	// 先对用户进行初始化
	this->userTable = new UserTable();
	this->userTable->AddRoot(); // 添加root用户
	this->curId = ROOT_ID;
	this->curName = "root";
	this->userTable->AddUser(this->curId, "unix", "1", ROOT_GID + 1); // 添加unix用户

	// 对缓存相关内容进行初始化
	this->bufManager = new BufferManager();
	this->spb = new SuperBlock();

	// 才能对superblock进行初始化，因为会调用函数
	this->spb->Init();
	// 将superblock写回磁盘
	this->WriteSpb();

	// 现在对目录进行初始化
	// 分配一个空闲的外存Inode来索引根目录
	this->rootDirInode = this->IAlloc();
	this->rootDirInode->i_uid = ROOT_ID;
	this->rootDirInode->i_gid = this->userTable->GetGId(ROOT_ID);
	this->rootDirInode->i_mode = Inode::INodeMode::IDIR |
								 Inode::INodeMode::OWNER_R | Inode::INodeMode::OWNER_W | Inode::INodeMode::OWNER_X |
								 Inode::INodeMode::GROUP_R | Inode::INodeMode::GROUP_X |
								 Inode::INodeMode::OTHER_R | Inode::INodeMode::OTHER_X;
	this->rootDirInode->i_nlink = 1;
	this->rootDirInode->i_size = 0;
	this->rootDirInode->i_mtime = unsigned int(time(NULL));
	this->rootDirInode->i_atime = unsigned int(time(NULL));
	this->curDirInode = this->rootDirInode;
	// 分配一个数据盘块存放根目录内容
	Directory *rootDir = new Directory();
	rootDir->mkdir(".", this->rootDirInode->i_number);	// 创建自己
	rootDir->mkdir("..", this->rootDirInode->i_number); // 创建父亲，根目录的父亲就是自己，这也是为什么不能直接调用mkdir函数的原因
	this->curDir = "/";

	// 跟root文件夹分配数据盘块号并且写回磁盘数据区中
	Buf *newBuf = this->Alloc();
	newBuf->b_addr = directory2Char(rootDir);
	this->bufManager->Bwrite(newBuf);
	// 给Inode写回数据区位置
	this->rootDirInode->i_size = sizeof(Directory) / NUM_SUB_DIR * 2;
	this->rootDirInode->i_addr[0] = newBuf->b_blkno;
	// delete rootDir; 没搞懂，还不能删了

	// 根据要求添加目录
	this->mkdir("/bin");
	this->mkdir("/etc");
	this->mkdir("/home");
	this->mkdir("/dev");
	// 将rootInode写回磁盘中
	this->rootDirInode->WriteI();

	// 创建并写入用户表
	this->fcreate("/etc/userTable.txt");
	int filoc = fopen("/etc/userTable.txt");
	File *userTableFile = &this->openFileTable[filoc];
	this->fwrite(userTable2Char(this->userTable), sizeof(UserTable), userTableFile); // 需要全部写入
	this->fclose(userTableFile);
}

/// @brief 打开文件
/// @param path 文件路径
/// @return File* 返回打开文件的指针
int FileSystem::fopen(string path)
{
	Inode *pinode = this->NameI(path);

	// 没有找到相应的Inode
	if (pinode == NULL)
	{
		cout << "没有找到对应的文件或目录!" << endl;
		throw(ENOENT);
		return NULL;
	}

	// 如果找到，判断所要找的文件是不是文件类型
	if (!(pinode->i_mode & Inode::INodeMode::IFILE))
	{
		cout << "不是一个正确的文件!" << endl;
		throw(ENOTDIR);
		return NULL;
	}

	// 如果找到，判断是否有权限打开文件
	if (this->Access(pinode, FileMode::EXC) == 0)
	{
		cout << "没有权限打开文件!" << endl;
		throw(EACCES);
		return NULL;
	}

	// 分配打开文件控制块File结构
	int fileloc = 0;
	File *pFile = this->FAlloc(fileloc);
	if (NULL == pFile)
	{
		cout << "打开太多文件!" << endl;
		throw(ENFILE);
		return NULL;
	}
	pFile->f_inode = pinode;
	pFile->f_offset = 0;
	pFile->f_uid = this->curId;
	pFile->f_gid = this->userTable->GetGId(this->curId);

	// 修改访问时间
	pinode->i_atime = unsigned int(time(NULL));

	return fileloc;
}

/// @brief 写文件
/// @param buffer 写入的内容
/// @param count 写入的字节数
/// @param fp 文件指针
void FileSystem::fwrite(const char *buffer, int count, File *fp)
{
	cout << *buffer << endl;
	if (fp == NULL)
	{
		cout << "文件指针为空!" << endl;
		throw(EBADF);
		return;
	}

	// 如果找到，判断是否有权限打开文件
	if (this->Access(fp->f_inode, FileMode::WRITE) == 0)
	{
		cout << "没有权限写文件!" << endl;
		throw(EACCES);
		return;
	}

	if (count + fp->f_offset > SIZE_BLOCK * NUM_I_ADDR)
	{
		cout << "写入文件太大!" << endl;
		throw(EFBIG);
		return;
	}
	// 获取文件的Inode
	Inode *pInode = fp->f_inode;

	// 写文件的三种情况：
	// 1. 写入的起始位置为逻辑块的起始地址；写入字节数为512-------异步写
	// 2. 非 写入的起始位置为逻辑块的起始地址；写入字节数为512----先Bread
	//  2.1 写到缓存末尾----------------------------------------异步写
	//	2.2 没有写到缓存末尾-------------------------------------延迟写

	int pos = 0; // 已经写入的字节数
	while (pos < count)
	{
		// 计算本次写入位置在文件中的位置
		int startpos = fp->f_offset + pos;
		// 计算本次写入物理盘块号，如果文件大小不够的话会在里面新分配物理盘块
		int blkno = pInode->Bmap(startpos % SIZE_BLOCK);
		// 计算本次写入的大小
		int size = SIZE_BLOCK - startpos % SIZE_BLOCK;
		if (size > count - pos)
			size = count - pos; // 修正写入的大小

		// 如果写入的起始位置为逻辑块的起始地址；写入字节数为512-------异步写
		if (startpos % SIZE_BLOCK == 0 && size == SIZE_BLOCK)
		{
			// 申请缓存
			Buf *pBuf = this->bufManager->GetBlk(blkno);
			// 将数据写入缓存
			memcpy(pBuf->b_addr, buffer + pos, size);
			// 将数据立即写入磁盘
			this->bufManager->Bwrite(pBuf);
		}
		else
		{ // 非 写入的起始位置为逻辑块的起始地址；写入字节数为512----先Bread
			// 申请缓存
			Buf *pBuf = this->bufManager->Bread(blkno);
			// 将数据写入缓存
			memcpy(pBuf->b_addr + startpos % SIZE_BLOCK, buffer + pos, size);

			// 写到缓存末尾---异步写
			if (startpos % SIZE_BLOCK + size == SIZE_BLOCK)
				this->bufManager->Bwrite(pBuf);
			else // 没有写到缓存末尾---延迟写
				this->bufManager->Bdwrite(pBuf);
		}

		pos += size;
		fp->f_offset += size;
	}

	if (pInode->i_size < fp->f_offset)
		pInode->i_size = fp->f_offset;
}

// 根据fd关闭文件
void FileSystem::fclose(File *fp)
{
	// 释放内存结点
	this->IPut(fp->f_inode);
	fp->Clean();
}

/// @brief 获取当前目录下的目录项
/// @return Directory 返回当前目录下的目录项
Directory FileSystem::getDir()
{
	// 如果不是目录文件
	if (this->curDirInode->i_mode & Inode::INodeMode::IFILE)
		return Directory();

	// 如果是目录文件
	int blkno = this->curDirInode->Bmap(0);
	// 读取磁盘的数据
	Buf *pbuf = this->bufManager->Bread(blkno);
	// 将数据转为目录结构
	Directory *dir = char2Directory(pbuf->b_addr);
	return *dir;
}

/// @brief 读文件到字符串中
/// @param fp 文件指针
/// @param buffer 读取内容索要存放的字符串
/// @param count  读取的字节数
void FileSystem::fread(File *fp, char *buffer, int count)
{
	if (fp == NULL)
	{
		cout << "文件指针为空!" << endl;
		throw(EBADF);
		return;
	}

	// 如果找到，判断是否有权限打开文件
	if (this->Access(fp->f_inode, FileMode::READ) == 0)
	{
		cout << "没有权限读文件!" << endl;
		throw(EACCES);
		return;
	}

	// 获取文件的Inode
	Inode *pInode = fp->f_inode;
	buffer = new char(count);
	int pos = 0; // 已经读取的字节数
	while (pos < count)
	{
		// 计算本读取位置在文件中的位置
		int startpos = fp->f_offset + pos;
		if (startpos >= pInode->i_size) // 读取位置超出文件大小
			break;
		// 计算本次读取物理盘块号，由于上一个判断,不会有读取位置超出文件大小的问题
		int blkno = pInode->Bmap(startpos % SIZE_BLOCK);
		// 计算本次读取的大小
		int size = SIZE_BLOCK - startpos % SIZE_BLOCK;
		if (size > count - pos)
			size = count - pos; // 修正读取的大小

		Buf *pBuf = this->bufManager->Bread(blkno);
		// TODO:如有大文件这里需要改
		memcpy(buffer + pos, pBuf->b_addr + startpos % SIZE_BLOCK, size);
		pos += size;
		fp->f_offset += size;
	}
}

FileSystem::~FileSystem()
{
	this->exit();
	delete this->bufManager;
	delete this->spb;
	delete this->userTable;
	delete this->curDirInode;
	delete this->rootDirInode;
}