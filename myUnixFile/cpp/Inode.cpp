/*
 * @Author: yingxin wang
 * @Date: 2023-05-10 14:16:31
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-14 14:56:22
 * @Description: Inode类相关操作
 */

#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

/// @brief      将逻辑块号lbn映射到物理盘块号phyBlkno
/// @param lbn  逻辑块号lbn，指的是在i_addr[]中的索引
/// @return int 返回物理盘块号phyBlkno
int Inode::Bmap(int lbn)
{
    /*
     * MyFileManager的文件索引结构：(小型、大型文件)
     * (1) i_addr[0] - i_addr[9]为直接索引表，文件长度范围是0 ~ 9个盘块
     */

    int phyBlkno = this->i_addr[lbn]; // 转换后的物理盘块号

    /*
     * 如果该逻辑块号还没有相应的物理盘块号与之对应，则分配一个物理块。
     * 这通常发生在对文件的写入，当写入位置超出文件大小，即对当前
     * 文件进行扩充写入，就需要分配额外的磁盘块，并为之建立逻辑块号
     * 与物理盘块号之间的映射。
     */
    // if (phyBlkno == 0 && (pFirstBuf = fs.Alloc(this->i_dev)) != NULL)
    // {
    //     /*
    //      * 因为后面很可能马上还要用到此处新分配的数据块，所以不急于立刻输出到
    //      * 磁盘上；而是将缓存标记为延迟写方式，这样可以减少系统的I/O操作。
    //      */
    //     bufMgr.Bdwrite(pFirstBuf);
    //     phyBlkno = pFirstBuf->b_blkno;
    //     /* 将逻辑块号lbn映射到物理盘块号phyBlkno */
    //     this->i_addr[lbn] = phyBlkno;
    //     this->i_flag |= Inode::IUPD;
    // }

    return phyBlkno;
}

void Inode::ICopy(Buf *bp, int inumber)
{
    // 从外存Inode读取数据到内存Inode
    DiskInode *dp;

    int offset = (inumber % NUM_INODE_PER_BLOCK) * sizeof(DiskInode);
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