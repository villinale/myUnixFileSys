/*
 * @Author: yingxin wang
 * @Date: 2023-05-24 11:09:29
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-26 01:03:25
 * @Description: ����д���
 */
#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

extern FileSystem fs;

void SuperBlock::Init()
{
    this->s_isize = NUM_DISKINODE / NUM_INODE_PER_BLOCK;

    this->s_fsize = NUM_BLOCK_ALL;
    this->s_ninode = 0;
    for (int i = NUM_FREE_INODE - 1; i >= 0; i--)
        this->s_inode[this->s_ninode++] = i + 1; // ������ʹ�õ�ʱ����Ǵ�λ����͵�inode��ʼʹ��
    // inode��1��ʼ����

    // superblockһ��ʼӦ�ù����1���NUM_FREE_BLOCK_GROUP���̿�

    // ��������,�ӵ�ַ���ʼ��������
    // SPB����λ����С���̿�
    unsigned int end = NUM_BLOCK_ALL;
    unsigned int start = POSITION_BLOCK;
    int numgroup = (end - start + 1) / NUM_FREE_BLOCK_GROUP + 1;
    // ���ݼ���û��Ϊ5��,�ֱ���[79,100,100,100,99]���������ı�const�ĳ���
    // ���ڵ�һ����99��������һ��ʼs_nfree���1��һ��ʼ����79���̿�
    this->s_nfree = end - start - (numgroup - 1) * NUM_FREE_BLOCK_GROUP + 1;

    BufferManager *bufManager = fs.GetBufferManager();
    Buf *bp;
    // ��ʼ���䣬�ܹ�������̿��[start,end]�����˶�ȡ
    // ��superblock�����5��
    for (int i = 0; i < this->s_nfree; i++) // ����д��λ��С���̿��ȱ��ֵ�
        this->s_free[i] = this->s_nfree + start - i - 1;
    // bufManager->bwrite((const char*)this->s_free, P, (NUM_FREE_BLOCK_GROUP + 1) * sizeof(int));
    //  �����ڵ�һ���̿�д�����ݣ�������̿��[starti,endi)
    int iblk = start;                                        // ���ڵ�һ���̿��
    int istart_addr = iblk * SIZE_BLOCK;                     // ���ڵ�һ���̿�ĵ�ַ
    int inum = NUM_FREE_BLOCK_GROUP;                         // ��һ�����
    int starti = start + this->s_nfree;                      // ��һ�鿪ʼ���̿��
    int endi = start + this->s_nfree + NUM_FREE_BLOCK_GROUP; // ��һ��������̿��
    for (int i = 2; i <= numgroup; i++)                      // ���һ���Ѿ���superblock����
    {
        // ÿ���½�һ����֤���������
        unsigned int *stack = new unsigned int[inum + 1]{0}; // ��һλ�����ӵ���һ����̿����
        int j = starti;
        stack[0] = inum;   // ��һλ�����ӵ���һ����̿����
        if (i == numgroup) // ��һ���̿�ĵ�һ��������0����־����
        {
            stack[1] = 0;
            j++;
        }
        for (; j < endi; j++) // ѭ��д�����ӵ���һ���̿��
            stack[j - starti + 1] = j;
        // �����ڵ�һ���̿�д�����
        // bufManager->bwrite(uintArray2Char(stack, inum + 1), istart_addr, (inum + 1) * sizeof(int));
        bp = bufManager->GetBlk(iblk);
        // ����֮ǰ���ڴ�й©
        memcpy(bp->b_addr, uintArray2Char(stack, inum + 1), (inum + 1) * sizeof(char));
        bufManager->Bwrite(bp);

        // ���¸�������
        iblk = endi;
        istart_addr = iblk * SIZE_BLOCK;
        inum = (i != (numgroup - 1)) ? NUM_FREE_BLOCK_GROUP : (NUM_FREE_BLOCK_GROUP - 1); // ��һ����NUM_FREE_BLOCK_GROUP - 1��
        starti = endi;
        endi = endi + inum;

        delete[] stack;
    }
}