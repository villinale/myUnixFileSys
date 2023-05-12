/*
 * @Author: yingxin wang
 * @Date: 2023-05-10 14:16:31
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-12 21:52:26
 * @Description: Inode类相关操作
 */

#include "../h/header.h"
#include "../h/errno.h"

int Inode::Bmap(int lbn)
{
    /*
     * MyFileManager的文件索引结构：(小型、大型文件)
     * (1) i_addr[0] - i_addr[9]为直接索引表，文件长度范围是0 ~ 8个盘块；
     *
     */

    int phyBlkno = this->i_addr[lbn]; // 转换后的物理盘块号

    /*
     * 如果该逻辑块号还没有相应的物理盘块号与之对应，则分配一个物理块。
     * 这通常发生在对文件的写入，当写入位置超出文件大小，即对当前
     * 文件进行扩充写入，就需要分配额外的磁盘块，并为之建立逻辑块号
     * 与物理盘块号之间的映射。
     */
    if (phyBlkno == 0 && (pFirstBuf = fileSys.Alloc(this->i_dev)) != NULL)
    {
        /*
         * 因为后面很可能马上还要用到此处新分配的数据块，所以不急于立刻输出到
         * 磁盘上；而是将缓存标记为延迟写方式，这样可以减少系统的I/O操作。
         */
        bufMgr.Bdwrite(pFirstBuf);
        phyBlkno = pFirstBuf->b_blkno;
        /* 将逻辑块号lbn映射到物理盘块号phyBlkno */
        this->i_addr[lbn] = phyBlkno;
        this->i_flag |= Inode::IUPD;
    }
    /* 找到预读块对应的物理盘块号 */
    Inode::rablock = 0;
    if (lbn <= 4)
    {
        /*
         * i_addr[0] - i_addr[5]为直接索引表。如果预读块对应物理块号可以从
         * 直接索引表中获得，则记录在Inode::rablock中。如果需要额外的I/O开销
         * 读入间接索引块，就显得不太值得了。漂亮！
         */
        Inode::rablock = this->i_addr[lbn + 1];
    }

    return phyBlkno;
}