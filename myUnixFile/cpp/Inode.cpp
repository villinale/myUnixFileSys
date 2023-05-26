/*
 * @Author: yingxin wang
 * @Date: 2023-05-10 14:16:31
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-26 17:22:10
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
     * (1) i_addr[0] - i_addr[7]Ϊֱ���������ļ����ȷ�Χ��0 ~ 8���̿�
     * (2) i_addr[8] - i_addr[9]Ϊһ�μ�����������ڴ��̿�ţ�ÿ���̿�
     * �ϴ��128��(512/4)�ļ������̿�ţ������ļ����ȷ�Χ��9 - (128 * 2 + 8)���̿飻
     */

    Buf *pFirstBuf, *pSecondBuf;
    int phyBlkno; // ת����������̿��
    BufferManager *bufMgr = fs.GetBufferManager();

    // ����ÿһ�ε�Alloc������Bdwrite
    if (lbn < NUM_BLOCK_IFILE)
    {
        phyBlkno = this->i_addr[lbn];
        if (phyBlkno == 0 && (pFirstBuf = fs.Alloc()) != NULL) // ��Ȼ������Alloc���������ͻ���Bdwrite
        {
            /*
             * ��Ϊ����ܿ������ϻ�Ҫ�õ��˴��·�������ݿ飬���Բ��������������
             * �����ϣ����ǽ�������Ϊ�ӳ�д��ʽ���������Լ���ϵͳ��I/O������
             * ���������������
             */
            bufMgr->Bdwrite(pFirstBuf);
            phyBlkno = pFirstBuf->b_blkno;
            // ���߼����lbnӳ�䵽�����̿��phyBlkno
            this->i_addr[lbn] = phyBlkno;
        }
    }
    else
    {
        int index = (lbn - NUM_BLOCK_IFILE) / NUM_FILE_INDEX;
        phyBlkno = this->i_addr[index];
        if (phyBlkno == 0)
        {
            this->i_mode |= Inode::ILARG;
            // ����һ�����̿��ż��������
            if ((pFirstBuf = fs.Alloc()) == NULL) // ������Alloc�����ǽ������ͻ���Bdwrite
                return 0;
            // i_addr[index]�м�¼���������������̿��
            this->i_addr[index] = pFirstBuf->b_blkno;
        }
        else
        {
            // �����洢�����������ַ���
            pFirstBuf = bufMgr->Bread(phyBlkno);
        }

        int *p = (int *)pFirstBuf->b_addr;

        // �����ڼ���������е�ƫ����
        index = (lbn - NUM_BLOCK_IFILE) % NUM_FILE_INDEX;

        if ((phyBlkno = p[index]) == 0 && (pSecondBuf = fs.Alloc()) != NULL)
        {
            // �����䵽���ļ������̿�ŵǼ���һ�μ����������
            phyBlkno = pSecondBuf->b_blkno;
            p[index] = phyBlkno;
            // �������̿顢���ĺ��һ�μ�����������ӳ�д��ʽ���������
            bufMgr->Bdwrite(pSecondBuf);
            bufMgr->Bdwrite(pFirstBuf);
        }
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
/// @return int ��ȷ����0�����󷵻�-1
int Inode::AssignMode(unsigned short mode)
{
    if (mode & Inode::IFILE || mode & Inode::IDIR || mode & Inode::ILARG)
        return -1;
    this->i_mode = mode;
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
            if (i < NUM_BLOCK_IFILE) // �ͷ�ֱ��������
                fs.Free(this->i_addr[i]);
            else
            {
                // �ͷ�һ�μ��������
                Buf *pFirstBuf = bufMgr->Bread(this->i_addr[i]);
                int *p = (int *)pFirstBuf->b_addr;
                for (int j = 0; j < NUM_FILE_INDEX; j++)
                {
                    if (p[j] != 0)
                        fs.Free(p[j]);
                }
                fs.Free(this->i_addr[i]);
            }
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

// ����id��gid��ȡ�ļ�Ȩ���ַ���
string Inode::GetModeString(int id, int gid)
{
    string permissionString;

    if (id == this->i_uid)
    {
        // ������Ȩ��
        permissionString += (this->i_mode & OWNER_R) ? "r" : "-";
        permissionString += (this->i_mode & OWNER_W) ? "w" : "-";
    }
    else if (gid == this->i_gid)
    {
        permissionString += (this->i_mode & GROUP_R) ? "r" : "-";
        permissionString += (this->i_mode & GROUP_W) ? "w" : "-";
    }
    else
    {
        permissionString += (this->i_mode & OTHER_R) ? "r" : "-";
        permissionString += (this->i_mode & OTHER_W) ? "w" : "-";
    }
    return permissionString;
}