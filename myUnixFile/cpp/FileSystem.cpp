/*
 * @Author: yingxin wang
 * @Date: 2023-05-12 08:12:28
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-12 22:26:41
 * @Description: FileSystem类，相当于FileManager
 */
#include "../h/header.h"
#include "../h/errno.h"

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

Inode *FileSystem::NameI(string path)
{
	Inode *pInode;
	Directory dir;
	vector<string> paths = stringSplit(path, '/');

	// 第一个字符为/表示绝对路径
	if (path[0] == '/') // 从根目录开始查找
		pInode = this->rootDirInode;
	else // 相对路径的查找
		pInode = this->curDirInode;

	while (true)
	{
	}
}

Buf *FileSystem::Alloc(short dev)
{
	int blkno; // 分配到的空闲物理磁盘块编号

	// 从索引表栈顶获取空闲磁盘块编号
	blkno = this->spb->s_free[--spb->s_nfree];

	// 若获取磁盘块编号为零，则表示已分配尽所有的空闲磁盘块，抛出异常
	if (0 == blkno)
	{
		spb->s_nfree = 0;
		throw(ENOSPC);
		return NULL;
	}

	// 栈已空，新分配到空闲磁盘块中记录下一组空闲磁盘块的编号,
	// 将下一组空闲磁盘块的编号读入SuperBlock的空闲磁盘块索引表s_free[] 中。
	if (spb->s_nfree <= 0)
	{
		//读入该空闲磁盘块 
		Buf *pBuf = this->bufManager->Bread(blkno);

		// 从该磁盘块的0字节开始记录，共占据4(s_nfree)+400(s_free[100])个字节 
		int *p = (int *)pBuf->b_addr;

		// 首先读出空闲盘块数s_nfree 
		spb->s_nfree = *p++;

		// 读取缓存中后续位置的数据，写入到SuperBlock空闲盘块索引表s_free[100]中 
		Utility::DWordCopy(p, spb->s_free, 100);

		// 缓存使用完毕，释放以便被其它进程使用 
		this->m_BufferManager->Brelse(pBuf);

		// 解除对空闲磁盘块索引表的锁，唤醒因为等待锁而睡眠的进程 
		spb->s_flock = 0;
		Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&spb->s_flock);
	}

	/* 普通情况下成功分配到一空闲磁盘块 */
	pBuf = this->m_BufferManager->GetBlk(dev, blkno); /* 为该磁盘块申请缓存 */
	this->m_BufferManager->ClrBuf(pBuf);			  /* 清空缓存中的数据 */
	spb->s_fmod = 1;								  /* 设置SuperBlock被修改标志 */

	return pBuf;
}

/// @brief 打开文件，由于没有系统调用，所以直接返回文件描述符
/// @param path 文件路径
/// @return int 文件描述符，文件描述符是从0开始的，所以返回值为-1表示打开失败
int FileSystem::Open(string path)
{
	Inode *pInode;

	// 找到相应的Inode
	// pInode = this->NameI(path);
	// // 没有找到相应的Inode
	// if (NULL == pInode)
	// {
	// 	return;
	// }
	// this->Open1(pInode, u.u_arg[1], 0);
	return 0;
}