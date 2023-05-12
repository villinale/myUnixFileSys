#pragma once
#define _CRT_SECURE_NO_WARNINGS

#define ROOT_ID 0
#define ROOT_GID 0

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <iomanip>
#include "errno.h"
using namespace std;

// 文件卷名称
static const string DISK_PATH = "myDisk.img";
// 文件逻辑块大小: 512字节
static const int SIZE_BLOCK = 512;

// SuperBlock中能够管理的最大空闲Inode与空闲数据盘块的数量
static const int NUM_FREE_BLOCK_GROUP = 100;
// SuperBlock开始的位置（以block为单位）
static const unsigned int POSITION_SUPERBLOCK = 0;

// DiskInode数量
static const int NUM_DISKINODE = 256;
// DiskInode中可以使用的最大物理块数量
static const int NUM_I_ADDR = 10;
// DiskInode大小（以字节为单位）
static const int SIZE_DISKINODE = 64;
// DiskInode开始的位置（以block为单位）
static const unsigned int POSITION_DISKINODE = 2;

// Block数量
static const int NUM_BLOCK = 1000000;
// Block开始的位置（以block为单位）
static const unsigned int POSITION_BLOCK = int(POSITION_DISKINODE + SIZE_DISKINODE * NUM_DISKINODE / SIZE_BLOCK);

// 规定：User内容最多占一个BLOCK，即:NUM_USER*(NUM_USER_NAME+NUM_USER_PASSWORD)<=BLOCK_SIZE
// 规定：User内容在数据区的第二个Block中
// User中最多用户数
static const int NUM_USER = 8;
// User用户名称的最大长度
static const unsigned int NUM_USER_NAME = (SIZE_BLOCK / NUM_USER - sizeof(short) * 2) / 2;
// User用户密码的最大长度
static const unsigned int NUM_USER_PASSWORD = (SIZE_BLOCK / NUM_USER - sizeof(short) * 2) / 2;
// User开始的位置（以block为单位）
static const unsigned int POSITION_USER = POSITION_BLOCK + 1;

// BufferManager缓存控制块、缓冲区的数量
static const int NUM_BUF = 15;
// BufferManager缓冲区大小。 以字节为单位
static const int SIZE_BUFFER = 512;

// 规定：根目录在数据区的第一个Block中
// Directory中一个目录下子目录文件名最大长度
static const int NUM_FILE_NAME = 28;
// Directory中一个目录下最多子目录个数
static const int NUM_SUB_DIR = SIZE_BLOCK / (NUM_FILE_NAME + sizeof(int));
// Directory开始的位置（以block为单位）
static const unsigned int POSITION_DIRECTORY = POSITION_BLOCK;

/*
 * 用户User类的定义
 * 由于有多个用户，但是没有实现多用户多进程文件读写，还是一种“伪并发”
 */
class User
{
public:
	short u_id[NUM_USER];						  // 用户id
	short u_gid[NUM_USER];						  // 用户所在组id
	char u_name[NUM_USER][NUM_USER_NAME];		  // 用户名
	char u_password[NUM_USER][NUM_USER_PASSWORD]; // 用户密码
	User();

	// 添加用户
	void AddUser(const short id, const char *name, const char *password, const short givengid);
	// 删除用户
	void DeleteUser(const short id, const char *name);
};

/*
 * 内存索引节点INode类的定义
 * 系统中每一个打开的文件、当前访问目录、挂载的子文件系统都对应唯一的内存inode。
 * 由于只有一个设备，所以不需要存储设备设备号，仅需要i_number来确定位置
 */
class Inode
{
public:
	/* i_type中标志位 */
	enum INodeType
	{
		IFILE = 0x1,
		IDIRECTORY = 0x2,
	};

	/* i_flag中标志位 */
	enum INodeFlag
	{
		ILOCK = 0x1,  /* 索引节点上锁 */
		IUPD = 0x2,	  /* 内存inode被修改过，需要更新相应外存inode */
		IACC = 0x4,	  /* 内存inode被访问过，需要修改最近一次访问时间 */
		IMOUNT = 0x8, /* 内存inode用于挂载子文件系统 */
		IWANT = 0x10, /* 有进程正在等待该内存inode被解锁，清ILOCK标志时，要唤醒这种进程 */
		ITEXT = 0x20  /* 内存inode对应进程图像的正文段 */
	};

	/* i_mode中标志位 */
	enum INodeMode
	{
		OWNER_R = 0x400,
		OWNER_W = 0x200,
		OWNER_X = 0x100,
		GROUP_R = 0x40,
		GROUP_W = 0x20,
		GROUP_X = 0x10,
		OTHER_R = 0x4,
		OTHER_W = 0x2,
		OTHER_X = 0x1,
	};

public:
	unsigned short i_number; /* 在inode区中的编号 */

	unsigned short i_uid; /* 文件所有者的用户标识数 */
	unsigned short i_gid; /* 文件所有者的组标识数 */

	unsigned short i_mode; /* 文件权限，定义见enum INodeMode */
	unsigned short i_type; /* 文件类型，定义见enum INodeType */
	unsigned short i_flag; /* 状态的标志位，定义见enum INodeFlag */

	unsigned short i_count; /* 引用计数 */
	unsigned short i_nlink; /* 文件联结计数，即该文件在目录树中不同路径名的数量 */

	unsigned int i_size;			 /* 文件大小，字节为单位 */
	unsigned int i_addr[NUM_I_ADDR]; /* 用于文件逻辑块号和物理块号转换的基本索引表 */

	unsigned int i_atime;
	unsigned int i_mtime;
};

/*
 * 外存索引节点(DiskINode)的定义
 * 外存Inode位于文件存储设备上的外存Inode区中。每个文件有唯一对应
 * 的外存Inode，其作用是记录了该文件对应的控制信息。
 * 外存Inode中许多字段和内存Inode中字段相对应。外存INode对象长度为64字节，
 * 每个磁盘块可以存放512/64 = 8个外存Inode
 */
class DiskInode
{
public:
	unsigned int d_mode; /* 状态的标志位，定义见enum INodeFlag */
	int d_nlink;		 /* 文件联结计数，即该文件在目录树中不同路径名的数量 */

	short d_uid; /* 文件所有者的用户标识数 */
	short d_gid; /* 文件所有者的组标识数 */

	int d_size;				/* 文件大小，字节为单位 */
	int d_addr[NUM_I_ADDR]; /* 用于文件逻辑块好和物理块好转换的基本索引表 */

	int d_atime; /* 最后访问时间 */
	int d_mtime; /* 最后修改时间 */
};

/*
 * 文件系统存储资源管理块(Super Block)的定义。
 */
class SuperBlock
{
	unsigned int s_isize;						/* Inode区占用的盘块数 */
	unsigned int s_ninode;						/* 直接管理的空闲内存Inode数量 */
	unsigned int s_inode[NUM_FREE_BLOCK_GROUP]; /* 直接管理的空闲内存Inode索引表 */

	unsigned int s_fsize;					   /* 盘块总数 */
	unsigned int s_nfree;					   /* 直接管理的空闲盘块数量 */
	unsigned int s_free[NUM_FREE_BLOCK_GROUP]; /* 直接管理的空闲盘块索引表 */
};

/*
 * 打开文件控制块File类。
 * 该结构记录了进程打开文件的读、写请求类型，文件读写位置等等。
 */
class File
{
public:
	enum FileFlags
	{
		FREAD = 0x1, /* 读请求类型 */
		FWRITE = 0x2 /* 写请求类型 */
	};

	Inode *f_inode;		   /* 指向打开文件的内存Inode指针 */
	unsigned int f_offset; /* 文件读写位置指针 */
	unsigned short f_uid;  /* 文件所有者的用户标识数 */
	unsigned int f_flag;   /* 对打开文件的读、写操作要求,定义见enum FileFlags */
};

/*
 * 目录Directory类
 * 该结构实现了树形带交叉勾连的目录结构
 * 一个Directory类就一个BLOCK大小
 */
struct Directory
{
	unsigned int d_inodenumber[NUM_SUB_DIR];	 // 子目录Inode号
	char d_filename[NUM_SUB_DIR][NUM_FILE_NAME]; // 子目录文件名
};

/*
 * 缓存控制块buf定义
 * 记录了相应缓存的使用情况等信息
 */
class Buf
{
public:
	enum BufFlag /* b_flags中标志位 */
	{
		// B_ERROR = 0x8,	 // I/O因出错而终止
		// B_BUSY = 0x10,	 // 相应缓存正在使用中
		// B_WANTED = 0x20, // 有进程正在等待使用该buf管理的资源，清B_BUSY标志时，要唤醒这种进程
		// B_ASYNC = 0x40, // 异步I/O，不需要等待其结束
		B_NONE = 0x0,	// 写操作。将缓存中的信息写到内存上去
		B_WRITE = 0x1,	// 写操作。将缓存中的信息写到内存上去
		B_READ = 0x2,	// 读操作。从内存读取信息到缓存中
		B_DONE = 0x4,	// I/O操作结束
		B_DELWRI = 0x80 // 延迟写，在相应缓存要移做他用时，再将其内容写到相应内存上
	};

public:
	unsigned int b_flags; /* 缓存控制块标志位,定义见enum BufFlag */
	/* 缓存控制块队列勾连指针 */
	Buf *b_forw;	//当前缓存控制块的前驱节点,将Buf插入NODEV队列或某一设备队列
	Buf *b_back;	//当前缓存控制块的后继节点,将Buf插入NODEV队列或某一设备队列
	Buf *av_forw;	//上一个空闲缓存控制块的指针,将Buf插入自由队列或某一I/O请求队列
	Buf *av_back;	//下一个空闲缓存控制块的指针,将Buf插入自由队列或某一I/O请求队列

	unsigned int b_wcount; // 需传送的字节数
	char *b_addr; // 指向该缓存控制块所管理的缓冲区的首地址
	unsigned int b_blkno;  // 内存逻辑块号

	Buf();
};

/*
 * 缓存控制块管理类BufferManager定义
 */
class BufferManager
{
private:
	Buf bFreeList;								// 自由缓存队列控制块,其实实现的是一个双向链表
	Buf SwBuf;									// 进程图像传送请求块
	Buf devtab;									// 由于只有一个设备，所以只有一个磁盘设备表
	Buf m_Buf[NUM_BUF];							// 缓存控制块数组
	char Buffer[NUM_BUF][SIZE_BUFFER]; // 缓冲区数组
public:
	//构造函数
	BufferManager();
	Buf* GetBlk(int blkno);
	void Bwrite(Buf* bp);
	Buf* Bread(int blkno);
};

class FileSystem
{
protected:
	BufferManager buf;
	SuperBlock spb;
	User user;
	short curId;
public:
	FileSystem();

	short getCurUserID();

};