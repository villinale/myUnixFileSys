#pragma once
#include "header.h"

class Inode
{
public:
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

	/* static const member */
	static const unsigned int IALLOC = 0x8000;					/* �ļ���ʹ�� */
	static const unsigned int IFMT = 0x6000;					/* �ļ��������� */
	static const unsigned int IFDIR = 0x4000;					/* �ļ����ͣ�Ŀ¼�ļ� */
	static const unsigned int IFCHR = 0x2000;					/* �ַ��豸���������ļ� */
	static const unsigned int IFBLK = 0x6000;					/* ���豸���������ļ���Ϊ0��ʾ���������ļ� */
	static const unsigned int ILARG = 0x1000;					/* �ļ��������ͣ����ͻ�����ļ� */
	static const unsigned int ISUID = 0x800;					/* ִ��ʱ�ļ�ʱ���û�����Ч�û�ID�޸�Ϊ�ļ������ߵ�User ID */
	static const unsigned int ISGID = 0x400;					/* ִ��ʱ�ļ�ʱ���û�����Ч��ID�޸�Ϊ�ļ������ߵ�Group ID */
	static const unsigned int ISVTX = 0x200;					/* ʹ�ú���Ȼλ�ڽ������ϵ����Ķ� */
	static const unsigned int IREAD = 0x100;					/* ���ļ��Ķ�Ȩ�� */
	static const unsigned int IWRITE = 0x80;					/* ���ļ���дȨ�� */
	static const unsigned int IEXEC = 0x40;						/* ���ļ���ִ��Ȩ�� */
	static const unsigned int IRWXU = (IREAD | IWRITE | IEXEC); /* �ļ������ļ��Ķ���д��ִ��Ȩ�� */
	static const unsigned int IRWXG = ((IRWXU) >> 3);			/* �ļ���ͬ���û����ļ��Ķ���д��ִ��Ȩ�� */
	static const unsigned int IRWXO = ((IRWXU) >> 6);			/* �����û����ļ��Ķ���д��ִ��Ȩ�� */

	static const int BLOCK_SIZE = 512;									 /* �ļ��߼����С: 512�ֽ� */
	static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int); /* ÿ�����������(��������)�����������̿�� */

	static const int SMALL_FILE_BLOCK = 6;							/* С���ļ���ֱ������������Ѱַ���߼���� */
	static const int LARGE_FILE_BLOCK = 128 * 2 + 6;				/* �����ļ�����һ�μ������������Ѱַ���߼���� */
	static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6; /* �����ļ��������μ����������Ѱַ�ļ��߼���� */

	static const int PIPSIZ = SMALL_FILE_BLOCK * BLOCK_SIZE;

	/* static member */
	static int rablock; /* ˳���ʱ��ʹ��Ԥ�����������ļ�����һ�ַ��飬rablock��¼����һ�߼����
						����bmapת���õ��������̿�š���rablock��Ϊ��̬������ԭ�򣺵���һ��bmap�Ŀ���
						�Ե�ǰ���Ԥ������߼���Ž���ת����bmap���ص�ǰ��������̿�ţ����ҽ�Ԥ����
						�������̿�ű�����rablock�С� */
};