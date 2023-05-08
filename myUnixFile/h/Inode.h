#pragma once
#include "header.h"

class Inode
{
public:
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

	/* static const member */
	static const unsigned int IALLOC = 0x8000;					/* 文件被使用 */
	static const unsigned int IFMT = 0x6000;					/* 文件类型掩码 */
	static const unsigned int IFDIR = 0x4000;					/* 文件类型：目录文件 */
	static const unsigned int IFCHR = 0x2000;					/* 字符设备特殊类型文件 */
	static const unsigned int IFBLK = 0x6000;					/* 块设备特殊类型文件，为0表示常规数据文件 */
	static const unsigned int ILARG = 0x1000;					/* 文件长度类型：大型或巨型文件 */
	static const unsigned int ISUID = 0x800;					/* 执行时文件时将用户的有效用户ID修改为文件所有者的User ID */
	static const unsigned int ISGID = 0x400;					/* 执行时文件时将用户的有效组ID修改为文件所有者的Group ID */
	static const unsigned int ISVTX = 0x200;					/* 使用后仍然位于交换区上的正文段 */
	static const unsigned int IREAD = 0x100;					/* 对文件的读权限 */
	static const unsigned int IWRITE = 0x80;					/* 对文件的写权限 */
	static const unsigned int IEXEC = 0x40;						/* 对文件的执行权限 */
	static const unsigned int IRWXU = (IREAD | IWRITE | IEXEC); /* 文件主对文件的读、写、执行权限 */
	static const unsigned int IRWXG = ((IRWXU) >> 3);			/* 文件主同组用户对文件的读、写、执行权限 */
	static const unsigned int IRWXO = ((IRWXU) >> 6);			/* 其他用户对文件的读、写、执行权限 */

	static const int BLOCK_SIZE = 512;									 /* 文件逻辑块大小: 512字节 */
	static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int); /* 每个间接索引表(或索引块)包含的物理盘块号 */

	static const int SMALL_FILE_BLOCK = 6;							/* 小型文件：直接索引表最多可寻址的逻辑块号 */
	static const int LARGE_FILE_BLOCK = 128 * 2 + 6;				/* 大型文件：经一次间接索引表最多可寻址的逻辑块号 */
	static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6; /* 巨型文件：经二次间接索引最大可寻址文件逻辑块号 */

	static const int PIPSIZ = SMALL_FILE_BLOCK * BLOCK_SIZE;

	/* static member */
	static int rablock; /* 顺序读时，使用预读技术读入文件的下一字符块，rablock记录了下一逻辑块号
						经过bmap转换得到的物理盘块号。将rablock作为静态变量的原因：调用一次bmap的开销
						对当前块和预读块的逻辑块号进行转换，bmap返回当前块的物理盘块号，并且将预读块
						的物理盘块号保存在rablock中。 */
};