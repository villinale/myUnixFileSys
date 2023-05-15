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
        this->s_inode[this->s_ninode++] = i; // 这样在使用的时候就是从位置最低的inode开始使用

    // superblock一开始应该管理第1组的NUM_FREE_BLOCK_GROUP个盘块

    // 成组链接,从地址最大开始进行链接
    // SPB管理位置最小的盘块
    unsigned int end = NUM_BLOCK_ALL - 1;
    unsigned int start = POSITION_BLOCK;
    int numgroup = (end - start + 1) / NUM_FREE_BLOCK_GROUP + 1;
    // 根据计算得会成为5组,分别是[79,100,100,100,99]，不能随便改变const的常量
    // 由于第一组是99个，所以一开始s_nfree会加1，一开始管理79个盘块
    this->s_nfree = end - start - 1 - (numgroup - 1) * NUM_FREE_BLOCK_GROUP + 1;

    // 开始分配，总共分配的盘块号[start,end]，两端都取
    // 给superblock分配第5组
    for (int i = 0; i < this->s_nfree; i++)
        this->s_free[i] = start + i;
    // 给组内第一个盘块写入数据，分配的盘块号[starti,endi)，两端都取
    unsigned int stack[NUM_FREE_BLOCK_GROUP + 1] = {};       // 第一位是链接的上一组的盘块个数
    int iblk = start;                                        // 组内第一个盘块号
    int istart_addr = iblk * SIZE_BLOCK;                     // 组内第一个盘块的地址
    int inum = this->s_nfree;                                // 上一组个数
    int starti = start + this->s_nfree;                      // 上一组开始的盘块号
    int endi = start + this->s_nfree + NUM_FREE_BLOCK_GROUP; // 上一组结束的盘块号
    BufferManager *bufManager = fs.GetBufferManager();
    for (int i = 1; i <= numgroup; i++)
    {
        stack[0] = inum;                    // 第一位是链接的上一组的盘块个数
        for (int j = starti; j < endi; j++) // 循环写入链接的上一组盘块号
            stack[j - starti] = j;
        // 将组内第一个盘块写入内存
        bufManager->bwrite((const char *)stack, istart_addr, (NUM_FREE_BLOCK_GROUP + 1) * sizeof(int));

        // 更新各个参数
        iblk = endi;
        istart_addr = iblk * SIZE_BLOCK;
        inum = (i != numgroup) ? NUM_FREE_BLOCK_GROUP : (NUM_FREE_BLOCK_GROUP - 1); // 第一组是NUM_FREE_BLOCK_GROUP - 1个
        starti = endi;
        endi = endi + inum;
    }
}