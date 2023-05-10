#pragma once
#define _CRT_SECURE_NO_WARNINGS

#define ROOT_ID		0
#define ROOT_GID	0

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <iomanip>
#include "errno.h"
using namespace std;


//�ļ�������
static const string DISK_PATH = "myDisk.img";
//�ļ��߼����С: 512�ֽ�
static const int BLOCK_SIZE = 512;

//SuperBlock���ܹ������������Inode����������̿������
static const int NUM_FREE_BLOCK_GROUP = 100;
//SuperBlock��ʼ��λ�ã���blockΪ��λ��
static const unsigned int SUPERBLOCK_POSITION = 0;

//Inode����
static const int NUM_INODE = 256;
//Inode�п���ʹ�õ������������� 
static const int NUM_I_ADDR = 10;
//Inode��С�����ֽ�Ϊ��λ��
static const int INODE_SIZE = 64;
//Inode��ʼ��λ�ã���blockΪ��λ��
static const unsigned int INODE_POSITION = 2;

//Block����
static const int NUM_BLOCK = 1000000;
//Block��ʼ��λ�ã���blockΪ��λ��
static const unsigned int BLOCK_POSITION = int(INODE_POSITION + INODE_SIZE * NUM_INODE / BLOCK_SIZE);

//�涨��User�������ռһ��BLOCK����:NUM_USER*(NUM_USER_NAME+NUM_USER_PASSWORD)<=BLOCK_SIZE
//�涨��User�������������ĵڶ���Block��
//User������û���
static const int NUM_USER = 8;
//User�û����Ƶ���󳤶�
static const unsigned int NUM_USER_NAME = (BLOCK_SIZE / NUM_USER - sizeof(short) * 2) / 2;
//User�û��������󳤶�
static const unsigned int NUM_USER_PASSWORD = (BLOCK_SIZE / NUM_USER - sizeof(short) * 2) / 2;
//User��ʼ��λ�ã���blockΪ��λ��
static const unsigned int USER_POSITION = BLOCK_POSITION + 1;

//BufferManager������ƿ顢������������
static const int NUM_BUF = 15;
//BufferManager��������С�� ���ֽ�Ϊ��λ
static const int BUFFER_SIZE = 512;

//�涨����Ŀ¼���������ĵ�һ��Block��
//Directory��һ��Ŀ¼����Ŀ¼�ļ�����󳤶�
static const int NUM_FILE_NAME = 28;
//Directory��һ��Ŀ¼�������Ŀ¼����
static const int NUM_SUB_DIR = BLOCK_SIZE / (NUM_FILE_NAME + sizeof(int));
//Directory��ʼ��λ�ã���blockΪ��λ��
static const unsigned int DIRECTORY_POSITION = BLOCK_POSITION;

/*
 * �û�User��Ķ���
 * �����ж���û�������û��ʵ�ֶ��û�������ļ���д������һ�֡�α������
 */
class User {
public:
	short u_id[NUM_USER];//�û�id
	short u_gid[NUM_USER];//�û�������id
	char u_name[NUM_USER][NUM_USER_NAME];     //�û���
	char u_password[NUM_USER][NUM_USER_PASSWORD]; //�û�����
	User();
};
void AddUser(const short id, const char* name, const char* password, const short givengid);

/*
 * �ڴ������ڵ�INode��Ķ���
 * ϵͳ��ÿһ���򿪵��ļ�����ǰ����Ŀ¼�����ص����ļ�ϵͳ����ӦΨһ���ڴ�inode��
 * ����û����棬���Բ���Ҫ�洢�豸�豸�ţ�����Ҫi_number��ȷ��λ��
 */
class Inode
{
public:
	/* i_type�б�־λ */
	enum INodeType
	{
		IFILE = 0x1,
		IDIRECTORY = 0x2,
	};

	/* i_flag�б�־λ */
	enum INodeFlag
	{
		ILOCK = 0x1,  /* �����ڵ����� */
		IUPD = 0x2,	  /* �ڴ�inode���޸Ĺ�����Ҫ������Ӧ���inode */
		IACC = 0x4,	  /* �ڴ�inode�����ʹ�����Ҫ�޸����һ�η���ʱ�� */
		IMOUNT = 0x8, /* �ڴ�inode���ڹ������ļ�ϵͳ */
		IWANT = 0x10, /* �н������ڵȴ����ڴ�inode����������ILOCK��־ʱ��Ҫ�������ֽ��� */
		ITEXT = 0x20  /* �ڴ�inode��Ӧ����ͼ������Ķ� */
	};

	/* i_mode�б�־λ */
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
	static int rablock; /* ˳���ʱ��ʹ��Ԥ�����������ļ�����һ�ַ��飬rablock��¼����һ�߼����
						����bmapת���õ��������̿�š���rablock��Ϊ��̬������ԭ�򣺵���һ��bmap�Ŀ���
						�Ե�ǰ���Ԥ������߼���Ž���ת����bmap���ص�ǰ��������̿�ţ����ҽ�Ԥ����
						�������̿�ű�����rablock�С� */

public:
	unsigned short i_number; /* ��inode���еı�� */

	unsigned short i_uid; /* �ļ������ߵ��û���ʶ�� */
	unsigned short i_gid; /* �ļ������ߵ����ʶ�� */

	unsigned short i_type; /* �ļ����ͣ������enum INodeType */
	unsigned short i_flag; /* ״̬�ı�־λ�������enum INodeFlag */
	unsigned short i_mode; /* �ļ�Ȩ�ޣ������enum INodeMode */

	unsigned short i_count;	/* ���ü��� */
	unsigned short i_nlink; /* �ļ���������������ļ���Ŀ¼���в�ͬ·���������� */

	unsigned int i_size;				/* �ļ���С���ֽ�Ϊ��λ */
	unsigned int i_addr[NUM_I_ADDR]; /* �����ļ��߼���ź�������ת���Ļ��������� */

	unsigned int i_lastr;	/* ������һ�ζ�ȡ�ļ����߼���ţ������ж��Ƿ���ҪԤ�� */
};

/*
 * �ļ�ϵͳ�洢��Դ�����(Super Block)�Ķ��塣
 */
class SuperBlock
{
	unsigned int	s_isize;		/* Inode��ռ�õ��̿��� */
	unsigned int	s_ninode;		/* ֱ�ӹ���Ŀ����ڴ�Inode���� */
	unsigned int	s_inode[NUM_FREE_BLOCK_GROUP];	/* ֱ�ӹ���Ŀ����ڴ�Inode������ */

	unsigned int	s_fsize;		/* �̿����� */
	unsigned int	s_nfree;		/* ֱ�ӹ���Ŀ����̿����� */
	unsigned int	s_free[NUM_FREE_BLOCK_GROUP];	/* ֱ�ӹ���Ŀ����̿������� */
};


/*
 * ���ļ����ƿ�File�ࡣ
 * �ýṹ��¼�˽��̴��ļ��Ķ���д�������ͣ��ļ���дλ�õȵȡ�
 */
class File
{
public:
	enum FileFlags
	{
		FREAD = 0x1,			/* ���������� */
		FWRITE = 0x2			/* д�������� */
	};

	Inode* f_inode;			/* ָ����ļ����ڴ�Inodeָ�� */
	unsigned int f_offset;			/* �ļ���дλ��ָ�� */
	unsigned short f_uid;			/* �ļ������ߵ��û���ʶ�� */
	unsigned int f_flag;	/* �Դ��ļ��Ķ���д����Ҫ��,�����enum FileFlags */
};


/*
* Ŀ¼Directory��
* �ýṹʵ�������δ����湴����Ŀ¼�ṹ
* һ��Directory���һ��BLOCK��С
*/
struct Directory {
	unsigned int d_inodenumber[NUM_SUB_DIR];//��Ŀ¼Inode��
	char d_filename[NUM_SUB_DIR][NUM_FILE_NAME];//��Ŀ¼�ļ���
};

/*
 * ������ƿ�buf����
 * ��¼����Ӧ�����ʹ���������Ϣ
 */
class Buf
{
public:
	enum BufFlag	/* b_flags�б�־λ */
	{
		B_WRITE = 0x1,		/* д�������������е���Ϣд���ڴ���ȥ */
		B_READ = 0x2,		/* �����������ڴ��ȡ��Ϣ�������� */
		B_DONE = 0x4,		/* I/O�������� */
		B_ERROR = 0x8,		/* I/O��������ֹ */
		B_BUSY = 0x10,		/* ��Ӧ��������ʹ���� */
		B_WANTED = 0x20,	/* �н������ڵȴ�ʹ�ø�buf�������Դ����B_BUSY��־ʱ��Ҫ�������ֽ��� */
		B_ASYNC = 0x40,		/* �첽I/O������Ҫ�ȴ������ */
		B_DELWRI = 0x80		/* �ӳ�д������Ӧ����Ҫ��������ʱ���ٽ�������д����Ӧ�ڴ��� */
	};

public:
	unsigned int b_flags;	/* ������ƿ��־λ,�����enum BufFlag */

	/* ������ƿ���й���ָ�� */
	Buf* b_forw;
	Buf* b_back;
	Buf* av_forw;
	Buf* av_back;

	unsigned int b_wcount;		/* �贫�͵��ֽ��� */
	unsigned char* b_addr;	/* ָ��û�����ƿ�������Ļ��������׵�ַ */
	unsigned int b_blkno;		/* �ڴ��߼���� */
};

/*
 * ������ƿ������BufferManager����
 */
class BufferManager
{
public:
	Buf bFreeList;						/* ���ɻ�����п��ƿ� */
	Buf SwBuf;							/* ����ͼ��������� */
	Buf m_Buf[NUM_BUF];					/* ������ƿ����� */
	unsigned char Buffer[NUM_BUF][BUFFER_SIZE];	/* ���������� */
};
