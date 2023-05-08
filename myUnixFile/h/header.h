#pragma once
#define _CRT_SECURE_NO_WARNINGS
# include <iostream>
# include <fstream>
# include <string>
# include <time.h>
# include <iomanip>
#include<bitset>
using namespace std;

//Inode中可以使用的最大物理块数量 
static const int NUM_I_ADDR = 10;
//SuperBlock中能够管理的最大空闲Inode与空闲数据盘块的数量
static const int NUM_FREE_BLOCK_GROUP = 100;
//User中最多用户数
static const int NUM_USER = 5;
//User用户名称的最大长度
static const unsigned int NUM_USER_NAME = 16;
//User用户密码的最大长度
static const unsigned int NUM_USER_PASSWORD = 32;
//Directory中一个目录下最多子目录个数
static const int NUM_SUB_DIR = 10;
//BufferManager缓存控制块、缓冲区的数量
static const int NUM_BUF = 15;
//BufferManager缓冲区大小。 以字节为单位
static const int BUFFER_SIZE = 512;
//Directory中一个目录下子目录文件名最大长度
static const int NUM_FILE_NAME = 20;

//文件逻辑块大小: 512字节
static const int BLOCK_SIZE = 512;

/*
 * 用户User类的定义
 * 由于有多个用户，但是没有实现多用户多进程文件读写，还是一种“伪并发”
 */
struct User {
	unsigned short u_id[NUM_USER];//用户id
	unsigned short u_gid[NUM_USER];//用户所在组id
	char u_name[NUM_USER][NUM_USER_NAME];     //用户名
	char u_password[NUM_USER][NUM_USER_PASSWORD]; //用户密码
};

/*
 * 内存索引节点INode类的定义
 * 系统中每一个打开的文件、当前访问目录、挂载的子文件系统都对应唯一的内存inode。
 * 由于没有外存，所以不需要存储设备设备号，仅需要i_number来确定位置
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

	/* static member */
	static int rablock; /* 顺序读时，使用预读技术读入文件的下一字符块，rablock记录了下一逻辑块号
						经过bmap转换得到的物理盘块号。将rablock作为静态变量的原因：调用一次bmap的开销
						对当前块和预读块的逻辑块号进行转换，bmap返回当前块的物理盘块号，并且将预读块
						的物理盘块号保存在rablock中。 */

public:
	unsigned int i_number; /* 在inode区中的编号 */

	unsigned short i_uid; /* 文件所有者的用户标识数 */
	unsigned short i_gid; /* 文件所有者的组标识数 */

	unsigned int i_type; /* 文件类型，定义见enum INodeType */
	unsigned int i_flag; /* 状态的标志位，定义见enum INodeFlag */
	unsigned int i_mode; /* 文件权限，定义见enum INodeMode */

	unsigned int i_count;	/* 引用计数 */
	//int i_nlink; /* 文件联结计数，即该文件在目录树中不同路径名的数量 */

	unsigned int i_size;				/* 文件大小，字节为单位 */
	unsigned int i_addr[NUM_I_ADDR]; /* 用于文件逻辑块号和物理块号转换的基本索引表 */

	unsigned int i_lastr;	/* 存放最近一次读取文件的逻辑块号，用于判断是否需要预读 */
};


/*
 * 文件系统存储资源管理块(Super Block)的定义。
 */
class SuperBlock
{
	unsigned int	s_isize;		/* Inode区占用的盘块数 */
	unsigned int	s_ninode;		/* 直接管理的空闲内存Inode数量 */
	unsigned int	s_inode[NUM_FREE_BLOCK_GROUP];	/* 直接管理的空闲内存Inode索引表 */

	unsigned int	s_fsize;		/* 盘块总数 */
	unsigned int	s_nfree;		/* 直接管理的空闲盘块数量 */
	unsigned int	s_free[NUM_FREE_BLOCK_GROUP];	/* 直接管理的空闲盘块索引表 */
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
		FREAD = 0x1,			/* 读请求类型 */
		FWRITE = 0x2			/* 写请求类型 */
	};

	Inode* f_inode;			/* 指向打开文件的内存Inode指针 */
	unsigned int f_offset;			/* 文件读写位置指针 */
	unsigned short f_uid;			/* 文件所有者的用户标识数 */
	unsigned int f_flag;	/* 对打开文件的读、写操作要求,定义见enum FileFlags */
};


/*
* 目录Directory类
* 该结构实现了树形带交叉勾连的目录结构
* 每一个目录结构是一个
*/
struct Directory {
	unsigned int d_inodenumber[NUM_SUB_DIR];//子目录Inode号
	char d_filename[NUM_SUB_DIR][NUM_FILE_NAME];//子目录文件名
};

/*
 * 缓存控制块buf定义
 * 记录了相应缓存的使用情况等信息
 */
class Buf
{
public:
	enum BufFlag	/* b_flags中标志位 */
	{
		B_WRITE = 0x1,		/* 写操作。将缓存中的信息写到内存上去 */
		B_READ = 0x2,		/* 读操作。从内存读取信息到缓存中 */
		B_DONE = 0x4,		/* I/O操作结束 */
		B_ERROR = 0x8,		/* I/O因出错而终止 */
		B_BUSY = 0x10,		/* 相应缓存正在使用中 */
		B_WANTED = 0x20,	/* 有进程正在等待使用该buf管理的资源，清B_BUSY标志时，要唤醒这种进程 */
		B_ASYNC = 0x40,		/* 异步I/O，不需要等待其结束 */
		B_DELWRI = 0x80		/* 延迟写，在相应缓存要移做他用时，再将其内容写到相应内存上 */
	};

public:
	unsigned int b_flags;	/* 缓存控制块标志位,定义见enum BufFlag */

	/* 缓存控制块队列勾连指针 */
	Buf* b_forw;
	Buf* b_back;
	Buf* av_forw;
	Buf* av_back;

	unsigned int b_wcount;		/* 需传送的字节数 */
	unsigned char* b_addr;	/* 指向该缓存控制块所管理的缓冲区的首地址 */
	unsigned int b_blkno;		/* 内存逻辑块号 */
};

/*
 * 缓存控制块管理类BufferManager定义
 */
class BufferManager
{
public:
	Buf bFreeList;						/* 自由缓存队列控制块 */
	Buf SwBuf;							/* 进程图像传送请求块 */
	Buf m_Buf[NUM_BUF];					/* 缓存控制块数组 */
	unsigned char Buffer[NUM_BUF][BUFFER_SIZE];	/* 缓冲区数组 */
};
