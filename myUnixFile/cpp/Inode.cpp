/*
 * @Author: yingxin wang
 * @Date: 2023-05-10 14:16:31
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-24 14:21:21
 * @Description: Inode����ز���
 */

#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

extern FileSystem fs;

Inode::Inode()
{
    this->i_mode = 0;
    this->i_count = 0;
    this->i_nlink = 0;
    this->i_number = -1;
    this->i_uid = -1;
    this->i_gid = -1;
    this->i_size = 0;
    for (int i = 0; i < 10; i++)
    {
        this->i_addr[i] = 0;
    }
}

/// @brief      ���߼����lbnӳ�䵽�����̿��phyBlkno
/// @param lbn  �߼����lbn��ָ������i_addr[]�е�����
/// @return int ���������̿��phyBlkno
int Inode::Bmap(int lbn)
{
    /*
     * MyFileManager���ļ������ṹ��(С�͡������ļ�)
     * (1) i_addr[0] - i_addr[9]Ϊֱ���������ļ����ȷ�Χ��0 ~ 9���̿�
     */

    Buf *pFirstBuf;
    int phyBlkno = this->i_addr[lbn]; // ת����������̿��
    BufferManager *bufMgr = fs.GetBufferManager();

    /*
     * ������߼���Ż�û����Ӧ�������̿����֮��Ӧ�������һ������顣
     * ��ͨ�������ڶ��ļ���д�룬��д��λ�ó����ļ���С�����Ե�ǰ
     * �ļ���������д�룬����Ҫ�������Ĵ��̿飬��Ϊ֮�����߼����
     * �������̿��֮���ӳ�䡣
     */
    if (phyBlkno == 0 && (pFirstBuf = fs.Alloc()) != NULL)
    {
        /*
         * ��Ϊ����ܿ������ϻ�Ҫ�õ��˴��·�������ݿ飬���Բ��������������
         * �����ϣ����ǽ�������Ϊ�ӳ�д��ʽ���������Լ���ϵͳ��I/O������
         * ���������������
         */
        bufMgr->Bdwrite(pFirstBuf);
        phyBlkno = pFirstBuf->b_blkno;
        /* ���߼����lbnӳ�䵽�����̿��phyBlkno */
        this->i_addr[lbn] = phyBlkno;
    }

    return phyBlkno;
}

/// @brief     ���ݻ�������bp�����Inode��ȡ���ݵ��ڴ�Inode
/// @param bp  ������ָ��
/// @param inumber  ���Inode���
void Inode::ICopy(Buf *bp, int inumber)
{
    // �����Inode��ȡ���ݵ��ڴ�Inode
    DiskInode *dp;

    int offset = ((inumber - 1) % NUM_INODE_PER_BLOCK) * sizeof(DiskInode);
    dp = char2DiskInode(bp->b_addr + offset);

    this->i_mode = dp->d_mode;
    this->i_nlink = dp->d_nlink;
    this->i_uid = dp->d_uid;
    this->i_gid = dp->d_gid;
    this->i_size = dp->d_size;
    this->i_number = inumber;
    this->i_count = 1;
    for (int i = 0; i < NUM_I_ADDR; i++)
        this->i_addr[i] = dp->d_addr[i];
}

/// @brief ���ݹ�����ڴ�Inode�����ļ�Ȩ��
/// @return unsigned short �����ļ�Ȩ��
unsigned short Inode::AssignMode(short id, short gid)
{
    return 0;
}

/// @brief ���Inode����
/// ��������˼����Դ�룬�кܶණ����û�б����������i_number��i_count�ȵ�
/// ����������ʱ���ֺ��е���
/// TODO:��ʵ��û�п��������ǻ�û�п���,�������Ϊ�������ĺ���Iputֻ����this->i_count = 1ʱ�Ż������
/// �����ڵ��ú���this->i_count--
void Inode::Clean()
{
    this->i_mode = 0;
    // this->i_count = 0;
    this->i_nlink = 0;
    // this->i_number = -1;
    this->i_uid = -1;
    this->i_gid = -1;
    this->i_size = 0;
    for (int i = 0; i < 10; i++)
    {
        this->i_addr[i] = 0;
    }
}

/// @brief ���ڴ�Inode���µ������
void Inode::WriteI()
{
    Buf *bp;
    BufferManager *bufMgr = fs.GetBufferManager();

    // �Ӵ��̶�ȡ����Inode
    bp = bufMgr->Bread(POSITION_DISKINODE + (this->i_number - 1) / NUM_INODE_PER_BLOCK);
    int offset = ((this->i_number - 1) % NUM_INODE_PER_BLOCK) * sizeof(DiskInode);

    DiskInode *dp = new DiskInode;
    // ���ڴ�Inode���Ƶ�����Inode��
    dp->d_mode = this->i_mode;
    dp->d_nlink = this->i_nlink;
    dp->d_uid = this->i_uid;
    dp->d_gid = this->i_gid;
    dp->d_size = this->i_size;
    dp->d_atime = this->i_atime;
    dp->d_mtime = this->i_mtime;
    for (int i = 0; i < NUM_I_ADDR; i++)
        dp->d_addr[i] = this->i_addr[i];
    memcpy(bp->b_addr + offset, dp, SIZE_DISKINODE);
    bufMgr->Bwrite(bp);
    delete dp;
}

/// @brief ��ȡ��Ŀ¼inodenumber
/// @return int ���ظ�Ŀ¼��inodenumber
int Inode::GetParentInumber()
{
    // ��Ŀ¼�ļ�Ҳ���ǲ��ܻ�ȡ��Ŀ¼�����ǲ�������������
    if (!(this->i_mode & Inode::INodeMode::IDIR))
        return NULL;

    BufferManager *bufMgr = fs.GetBufferManager();

    // �Ȼ�ȡ��Ŀ¼��Ŀ¼������и�Ŀ¼λ��
    Buf *bp = bufMgr->Bread(this->i_addr[0]);
    Directory *dir = char2Directory(bp->b_addr);

    return dir->d_inodenumber[1];
}

/// @brief �ͷ������̿�
void Inode::ITrunc()
{
    BufferManager *bufMgr = fs.GetBufferManager();
    for (int i = 0; i < NUM_I_ADDR; i++)
    {
        if (this->i_addr[i] != 0)
        {
            fs.Free(this->i_addr[i]);
            this->i_addr[i] = 0;
        }
    }
}

/// @brief ��ȡĿ¼����
/// @return Directory* ����Ŀ¼����
Directory *Inode::GetDir()
{
    // ��Ŀ¼�ļ����ܻ�ȡĿ¼����
    if (!(this->i_mode & Inode::INodeMode::IDIR))
        return NULL;

    BufferManager *bufMgr = fs.GetBufferManager();

    Buf *bp = bufMgr->Bread(this->i_addr[0]);
    Directory *dir = char2Directory(bp->b_addr);
}