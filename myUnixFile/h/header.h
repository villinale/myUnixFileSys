#pragma once
#ifndef HEADER_H
#define HEADER_H

#define _CRT_SECURE_NO_WARNINGS

#define ROOT_ID 0
#define ROOT_GID 0

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "errno.h"
using namespace std;

// �ļ�������
static const string DISK_PATH = "myDisk.img";
// �ļ��߼����С: 512�ֽ�
static const int SIZE_BLOCK = 512;
// �ļ����С
static const int SIZE_DISK = SIZE_BLOCK * 512;
// �ļ�����Block����
static const int NUM_BLOCK_ALL = SIZE_DISK / SIZE_BLOCK;

// SuperBlock���ܹ������������Inode����������̿������
static const int NUM_FREE_INODE = 100;
// SuperBlock��ʼ��λ�ã���blockΪ��λ��
static const unsigned int POSITION_SUPERBLOCK = 0;
// SuperBlockһ����������̿������
static const int NUM_FREE_BLOCK_GROUP = 100;

// DiskInode����
static const int NUM_DISKINODE = 256;
// DiskInode�п���ʹ�õ�������������
static const int NUM_I_ADDR = 10;
// DiskInode��С�����ֽ�Ϊ��λ��
static const int SIZE_DISKINODE = 64;
// DiskInode��ʼ��λ�ã���blockΪ��λ��
static const unsigned int POSITION_DISKINODE = 2;
// ÿ��Block��DiskInode������
static const int NUM_INODE_PER_BLOCK = SIZE_BLOCK / SIZE_DISKINODE;

// ���ݿ�Block����
static const int NUM_BLOCK = NUM_BLOCK_ALL - POSITION_DISKINODE - NUM_DISKINODE / NUM_INODE_PER_BLOCK;
// ���ݿ�Block��ʼ��λ�ã���blockΪ��λ��
static const unsigned int POSITION_BLOCK = int(POSITION_DISKINODE + SIZE_DISKINODE * NUM_DISKINODE / SIZE_BLOCK);

// �涨��User�������ռһ��BLOCK����:NUM_USER*(NUM_USER_NAME+NUM_USER_PASSWORD)<=BLOCK_SIZE
// �涨��User�������������ĵڶ���Block��
// User������û���
static const int NUM_USER = 8;
// User�û����Ƶ���󳤶�
static const unsigned int NUM_USER_NAME = (SIZE_BLOCK / NUM_USER - sizeof(short) * 2) / 2;
// User�û��������󳤶�
static const unsigned int NUM_USER_PASSWORD = (SIZE_BLOCK / NUM_USER - sizeof(short) * 2) / 2;
// User��ʼ��λ�ã���blockΪ��λ��
static const unsigned int POSITION_USER = POSITION_BLOCK + 1;

// BufferManager������ƿ顢������������
static const int NUM_BUF = 15;
// BufferManager��������С�� ���ֽ�Ϊ��λ
static const int SIZE_BUFFER = 512;

// �涨����Ŀ¼���������ĵ�һ��Block��
// Directory��һ��Ŀ¼����Ŀ¼�ļ�����󳤶�
static const int NUM_FILE_NAME = 28;
// Directory��һ��Ŀ¼�������Ŀ¼����
static const int NUM_SUB_DIR = SIZE_BLOCK / (NUM_FILE_NAME + sizeof(int));
// Directory��ʼ��λ�ã���blockΪ��λ��
static const unsigned int POSITION_DIRECTORY = POSITION_BLOCK;

// FileSystem:OpenFileTable�������ļ���
static const int NUM_FILE = 100;
// FileSystem:IndoeTable�����Inode��
static const int NUM_INODE = 100;

// ʵ���ַ����ָ�
vector<string> stringSplit(const string &strIn, char delim);

/*
 * �û�User��Ķ���
 * �����ж���û�������û��ʵ�ֶ��û�������ļ���д������һ�֡�α������
 */
class UserTable
{
public:
	unsigned short u_id[NUM_USER];				  // �û�id
	unsigned short u_gid[NUM_USER];				  // �û�������id
	char u_name[NUM_USER][NUM_USER_NAME];		  // �û���
	char u_password[NUM_USER][NUM_USER_PASSWORD]; // �û�����
	UserTable();

	// ����û�
	void AddUser(const short id, const char *name, const char *password, const short givengid);
	// ɾ���û�
	void DeleteUser(const short id, const char *name);

	unsigned short GetGId(const unsigned short id);
};

/*
 * �ļ�ϵͳ�洢��Դ�����(Super Block)�Ķ��塣
 */
class SuperBlock
{
public:
	unsigned int s_isize; /* Inode��ռ�õ��̿��� */
	unsigned int s_fsize; /* �̿����� */

	unsigned int s_ninode;				  /* ֱ�ӹ���Ŀ������Inode���� */
	unsigned int s_inode[NUM_FREE_INODE]; /* ֱ�ӹ���Ŀ������Inode������ */

	unsigned int s_nfree;					   /* ֱ�ӹ���Ŀ����̿����� */
	unsigned int s_free[NUM_FREE_BLOCK_GROUP]; /* ֱ�ӹ���Ŀ����̿������� */

	// ��ʼ��SuperBlock
	void Init();
};

/*
 * ��������ڵ�(DiskINode)�Ķ���
 * ���Inodeλ���ļ��洢�豸�ϵ����Inode���С�ÿ���ļ���Ψһ��Ӧ
 * �����Inode���������Ǽ�¼�˸��ļ���Ӧ�Ŀ�����Ϣ��
 * ���Inode������ֶκ��ڴ�Inode���ֶ����Ӧ�����INode���󳤶�Ϊ64�ֽڣ�
 * ÿ�����̿���Դ��512/64 = 8�����Inode
 */
class DiskInode
{
public:
	unsigned int d_mode; /* ״̬�ı�־λ�������enum INodeFlag */
	int d_nlink;		 /* �ļ���������������ļ���Ŀ¼���в�ͬ·���������� */

	short d_uid; /* �ļ������ߵ��û���ʶ�� */
	short d_gid; /* �ļ������ߵ����ʶ�� */

	int d_size;				/* �ļ���С���ֽ�Ϊ��λ */
	int d_addr[NUM_I_ADDR]; /* �����ļ��߼���ú�������ת���Ļ��������� */

	int d_atime; /* ������ʱ�� */
	int d_mtime; /* ����޸�ʱ�� */
};

/*
 * ������ƿ�buf����
 * ��¼����Ӧ�����ʹ���������Ϣ
 */
class Buf
{
public:
	enum BufFlag /* b_flags�б�־λ */
	{
		// B_ERROR = 0x8,	 // I/O��������ֹ
		// B_BUSY = 0x10,	 // ��Ӧ��������ʹ����
		// B_WANTED = 0x20, // �н������ڵȴ�ʹ�ø�buf�������Դ����B_BUSY��־ʱ��Ҫ�������ֽ���
		// B_ASYNC = 0x40, // �첽I/O������Ҫ�ȴ������
		B_NONE = 0x0,	// ��ʼ��
		B_WRITE = 0x1,	// д�������������е���Ϣд���ڴ���ȥ
		B_READ = 0x2,	// �����������ڴ��ȡ��Ϣ��������
		B_DONE = 0x4,	// I/O��������
		B_DELWRI = 0x80 // �ӳ�д������Ӧ����Ҫ��������ʱ���ٽ�������д����Ӧ�ڴ���
	};

public:
	unsigned int b_flags; /* ������ƿ��־λ,�����enum BufFlag */
	/* ������ƿ���й���ָ�� */
	Buf *b_forw;  // ��ǰ������ƿ��ǰ���ڵ�,��Buf����NODEV���л�ĳһ�豸����
	Buf *b_back;  // ��ǰ������ƿ�ĺ�̽ڵ�,��Buf����NODEV���л�ĳһ�豸����
	Buf *av_forw; // ��һ�����л�����ƿ��ָ��,��Buf�������ɶ��л�ĳһI/O�������
	Buf *av_back; // ��һ�����л�����ƿ��ָ��,��Buf�������ɶ��л�ĳһI/O�������

	unsigned int b_wcount; // �贫�͵��ֽ���
	char *b_addr;		   // ָ��û�����ƿ�������Ļ��������׵�ַ
	unsigned int b_blkno;  // �ڴ��߼����

	Buf();
};

/*
 * �ڴ������ڵ�INode��Ķ���
 * ϵͳ��ÿһ���򿪵��ļ�����ǰ����Ŀ¼�����ص����ļ�ϵͳ����ӦΨһ���ڴ�inode��
 * ����ֻ��һ���豸�����Բ���Ҫ�洢�豸�豸�ţ�����Ҫi_number��ȷ��λ��
 */
class Inode
{
public:
	/* i_mode�б�־λ */
	enum INodeMode
	{
		IDIR = 0x4000,	// �ļ����ͣ�Ŀ¼�ļ�
		IFILE = 0x2000, // �ļ����ͣ���ͨС�ļ�
		ILARG = 0x1000, // �ļ����ͣ����ļ�
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
	unsigned short i_uid; // �ļ������ߵ��û���ʶ��
	unsigned short i_gid; // �ļ������ߵ����ʶ��

	unsigned short i_mode; // �ļ�Ȩ�ޣ������enum INodeMode

	unsigned short i_count; // ���ü���
	unsigned short i_nlink; // �ļ���������������ļ���Ŀ¼���в�ͬ·����������

	unsigned int i_size;			 // �ļ���С���ֽ�Ϊ��λ
	unsigned int i_addr[NUM_I_ADDR]; // ָ�����ݿ����������ļ��߼���ź�������ת���Ļ���������

	unsigned int i_atime;
	unsigned int i_mtime;

	unsigned short i_number; // ��inode���еı��,�ŵ�����Ա��ڽ��ڴ�Inodeת��Ϊ���Inode

	// ���߼����lbnӳ�䵽�����̿��phyBlkno
	int Bmap(int lbn);

	// ���ݻ�������bp�����Inode��ȡ���ݵ��ڴ�Inode
	void ICopy(Buf *bp, int inumber);

	// ���ݹ�����ڴ�Inode�����ļ�Ȩ�ޣ�û���õ�
	unsigned short AssignMode(unsigned short id, unsigned short gid);

	// ���Inode����
	void Clean();

	// ���ڴ�Inode���µ������
	void WriteI();
};

/*
 * ���ļ����ƿ�File�ࡣ
 * �ļ������ߵ��û���ʶ�����ļ���дλ�õȵȡ�
 */
class File
{
public:
	Inode *f_inode;		   /* ָ����ļ����ڴ�Inodeָ�� */
	unsigned int f_offset; /* �ļ���дλ��ָ�� */
	unsigned short f_uid;  /* �ļ������ߵ��û���ʶ�� */
	unsigned short f_gid;  /* �ļ������ߵ����ʶ�� */
	File()
	{
		this->f_inode = NULL;
	}
};

/*
 * Ŀ¼Directory��
 * �ýṹʵ�������δ����湴����Ŀ¼�ṹ
 * һ��Directory���һ��BLOCK��С
 */
class Directory
{
public:
	unsigned int d_inodenumber[NUM_SUB_DIR];	 // ��Ŀ¼Inode��
	char d_filename[NUM_SUB_DIR][NUM_FILE_NAME]; // ��Ŀ¼�ļ���

	Directory();
};

/*
 * ������ƿ������BufferManager����
 */
class BufferManager
{
private:
	Buf bFreeList;					   // ���ɻ�����п��ƿ�,��ʵʵ�ֵ���һ��˫������
	Buf SwBuf;						   // ����ͼ���������
	Buf devtab;						   // ����ֻ��һ���豸������ֻ��һ�������豸��
	Buf m_Buf[NUM_BUF];				   // ������ƿ�����
	char Buffer[NUM_BUF][SIZE_BUFFER]; // ����������
public:
	// ���캯��
	BufferManager();

	// ���������豸��Ŷ�ȡ����
	Buf *GetBlk(int blkno);

	// �������bpд��������
	void Bwrite(Buf *bp);

	void bwrite(const char *buf, unsigned int start_addr, unsigned int size);

	// ���������豸��Ŷ�ȡ����
	Buf *Bread(int blkno);

	// ��������
	void Bread(char *buf, int blkno, int offset, int size);
};

// �൱��FileSystem��FileManager��InodeTable�ĺ���
class FileSystem
{
private:
	short curId;				  // Ŀǰʹ�õ�userID
	BufferManager *bufManager;	  // ������ƿ������
	SuperBlock *spb;			  // ������
	UserTable *userTable;		  // �û���
	File openFileTable[NUM_FILE]; // ���ļ�������ֻ��һ����������û�н��̴��ļ���
	Inode inodeTable[NUM_INODE];  // �ڴ�Inode��
	Inode *curDirInode;			  // ָ��ǰĿ¼��Inodeָ��
	Inode *rootDirInode;		  // ��Ŀ¼�ڴ�Inode

	// ����path��ȡInode
	Inode *NameI(string path);

	// �ж�ָ�����Inode�Ƿ��Ѿ����ص��ڴ���
	int IsLoaded(int inumber);

	// ������ȡָ�����Inode���ڴ���
	Inode *IGet(int inumber);

	// ��openFileTable�����һ���ļ�
	int AddFileinFileTable(Inode *pInode);

	// ����pInode�Ƿ��и���mode��Ȩ��
	int Access(Inode *pInode, unsigned int mode);

	// ����һ�����е����Inode
	Inode *IAlloc();

public:
	enum FileMode
	{
		READ = 0x1,	 // ��
		WRITE = 0x2, // д
		EXC = 0x4	 // ִ�У��൱�ڴ��ļ�
	};

	FileSystem();

	BufferManager *GetBufferManager()
	{
		return this->bufManager;
	}

	// ��ʼ���ļ�ϵͳ
	void init();

	// ��ȡ��ǰ�û���ID
	short getCurUserID();

	// ����path���ļ����ļ���
	int fopen(string path);

	// ����fd�ر��ļ�
	// int Close(int fd);

	// �����ļ�
	int fcreate(string path);
};

#endif