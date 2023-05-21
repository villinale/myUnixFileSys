/*
 * @Author: yingxin wang
 * @Date: 2023-05-12 08:12:28
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-21 16:33:35
 * @Description: FileSystem类中在命令行可以调用的函数
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
	// fatherBuf->b_addr = directory2Char(fatherDir);
	this->bufManager->bwrite(directory2Char(fatherDir), POSITION_BLOCK + fatherBuf->b_blkno, sizeof(fatherDir));
	// this->bufManager->Bwrite(fatherBuf);

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

	// 将新文件夹写入其父亲的目录项中
	fatherDir->mkdir(name.c_str(), newinode->i_number);
	fatherInode->i_size += sizeof(Directory) / NUM_SUB_DIR; // 父亲的大小增加一个目录项
	fatherBuf->b_addr = directory2Char(fatherDir);

	// 统一写回：父目录inode，新目录inode，父目录数据块、新目录数据块
	fatherInode->WriteI();
	newinode->WriteI();
	this->bufManager->bwrite(directory2Char(fatherDir), POSITION_BLOCK + fatherBuf->b_blkno, sizeof(fatherDir));
	this->bufManager->bwrite(directory2Char(newDir), POSITION_BLOCK + newBuf->b_blkno, sizeof(newDir));

	// this->bufManager->Bwrite(fatherBuf);
	// this->bufManager->Bwrite(newBuf);
	// 除FS里的rootInode和curInode外释放所有Inode
	if (fatherInode != this->rootDirInode && fatherInode != this->curDirInode)
		this->IPut(fatherInode);
	this->IPut(newinode);

	return 0;
}

void FileSystem::exit()
{
	// 将superblock写回磁盘
	this->bufManager->bwrite((const char *)this->spb, POSITION_SUPERBLOCK, sizeof(SuperBlock));
	// TODO:将userTable写回磁盘

	// 将所有标记延迟写的内容都写回磁盘
	this->bufManager->SaveAll();
}

File *FileSystem::fopen(string path)
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
	File *pFile = this->FAlloc();
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

	return pFile;
}

/// @brief 写文件
/// @param buffer 写入的内容
/// @param count 写入的字节数
/// @param fp 文件指针
void FileSystem::fwrite(const char *buffer, int count, File *fp)
{
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
		// 计算本次写入的物理位置
		int startaddr = blkno * SIZE_BLOCK + startpos % SIZE_BLOCK;
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