/*
 * @Author: yingxin wang
 * @Date: 2023-05-10 21:22:11
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-21 21:53:23
 * @Description: BufferManager相关操作
 */
#include "../h/header.h"
#include "../h/errno.h"

Buf::Buf()
{
    this->b_flags = BufFlag::B_NONE;
    this->av_back = NULL;
    this->av_forw = NULL;
    this->b_forw = NULL;
    this->b_back = NULL;

    this->b_wcount = -1;
    this->b_blkno = -1;
    this->b_addr = NULL;
}

BufferManager::BufferManager()
{
    // 自由缓存队列控制块，实现双向链表的链接，但是本身没有对应的缓冲区数组
    // 一开始整个缓存控制数组就是一个自由缓存队列
    this->bFreeList.b_forw = &m_Buf[NUM_BUF - 1];
    this->bFreeList.b_back = &m_Buf[0];
    this->bFreeList.av_forw = &m_Buf[0];
    this->bFreeList.av_back = &m_Buf[NUM_BUF - 1];

    // 对缓存控制块数组中每一个缓存实现链接，b_flags、b_wcount、b_blkno在缓存构造时就已构造
    for (auto i = 0; i < NUM_BUF; i++)
    {
        this->m_Buf[i].b_addr = this->Buffer[i];

        // 前驱节点
        this->m_Buf[i].b_forw = (i - 1 >= 0) ? (&m_Buf[i - 1]) : (&bFreeList);
        // 后继节点
        this->m_Buf[i].b_back = (i + 1 < NUM_BUF) ? (&m_Buf[i + 1]) : (&bFreeList);
        // 上一个空闲缓存控制块的指针
        this->m_Buf[i].av_forw = (i + 1 < NUM_BUF) ? (&m_Buf[i - 1]) : (&bFreeList);
        // 下一个空闲缓存控制块的指针
        this->m_Buf[i].av_back = (i - 1 >= 0) ? (&m_Buf[i - 1]) : (&bFreeList);
    }

    // 磁盘设备表
    this->devtab.b_forw = &devtab;
    this->devtab.b_back = &devtab;
    this->devtab.av_forw = &devtab;
    this->devtab.av_back = &devtab;
}

/// @brief 申请缓存，而且只会在Bread中调用，不在Bwrite中调用，因为每次写都是要先读，再对读后获得的缓存进行修改，非常精妙！！！
/// @param blkno 逻辑块号
/// @return 寻找到的Buf
Buf *BufferManager::GetBlk(int blkno)
{
    Buf *bp = NULL;

    // 在设备队列中找与blkno相同者，直接利用
    for (bp = this->devtab.b_back; bp != &(this->devtab); bp = bp->b_back)
    {
        if (bp->b_blkno == blkno)
        {
            return bp;
        }
    }

    // 在自由队列中寻找
    // 自由队列为空
    if (this->bFreeList.av_forw == &this->bFreeList)
    {
        // 不太可能，因为每次都是设备读写之后就立即释放字符块
    }

    // 取出自由队列队头
    bp = this->bFreeList.av_back;
    // 从自由队列取出
    bp->av_forw->av_back = bp->av_back;
    bp->av_back->av_forw = bp->av_forw;
    // 从原设备队列或NODEV队列取出
    bp->b_forw->b_back = bp->b_back;
    bp->b_back->b_forw = bp->b_forw;

    // 如果该字符块是延迟写，将其异步写到磁盘上
    if (bp->b_flags & Buf::B_DELWRI)
        this->Bwrite(bp);

    bp->b_flags = Buf::B_NONE;

    // 从原设备队列中抽出
    bp->b_back->b_forw = bp->b_forw;
    bp->b_forw->b_back = bp->b_back;
    // 加入新的设备队列
    bp->b_forw = this->devtab.b_forw;
    bp->b_back = &(this->devtab);
    this->devtab.b_forw->b_back = bp;
    this->devtab.b_forw = bp;

    bp->b_blkno = blkno;
    return bp;
}

/// @brief 将缓存块bp写到磁盘上
/// @param bp 缓存块
void BufferManager::Bwrite(Buf *bp)
{
    // 将缓存放入设备的I/O请求队列队尾
    bp->av_forw = this->devtab.av_forw;
    bp->av_back = &(this->devtab);
    bp->av_forw->av_back = bp;
    bp->av_back->av_forw = bp;

    // 开始写操作
    bp->b_flags |= Buf::B_WRITE;
    fstream fd;
    fd.open(DISK_PATH, ios::in | ios::out | ios::binary);
    if (!fd.is_open())
    {
        cout << "无法打开一级磁盘文件myDisk.img" << endl;
        throw(ENOENT);
    }
    // TODO：这里可能要修改
    // 这里没太搞懂，在上层进行写操作的时候，
    // 是先读盘块，再写盘块相关内容，只修改缓存中内容，所以是SIZE_BUFFER地写
    fd.seekp(streampos(bp->b_blkno) * streampos(SIZE_BUFFER), ios::beg);
    fd.write((const char *)bp->b_addr, SIZE_BUFFER);
    fd.close();
    // 写操作完成

    // 更新标志
    bp->b_flags = Buf::BufFlag::B_DONE;
    // 从I/O请求队列取出
    bp->av_forw->av_back = bp->av_back;
    bp->av_back->av_forw = bp->av_forw;
    // 加入自由队列
    bp->av_forw = (this->bFreeList).av_forw;
    bp->av_back = &((this->bFreeList));
    bp->av_forw->av_back = bp;
    bp->av_back->av_forw = bp;
}

void BufferManager::Bdwrite(Buf *bp)
{
    // 将缓存放入设备的I/O请求队列队尾
    bp->av_forw = this->devtab.av_forw;
    bp->av_back = &(this->devtab);
    bp->av_forw->av_back = bp;
    bp->av_back->av_forw = bp;

    // 标记为延迟写
    bp->b_flags |= (Buf::B_DELWRI | Buf::B_DONE);

    // 从I/O请求队列取出
    bp->av_forw->av_back = bp->av_back;
    bp->av_back->av_forw = bp->av_forw;
    // 加入自由队列
    bp->av_forw = (this->bFreeList).av_forw;
    bp->av_back = &((this->bFreeList));
    bp->av_forw->av_back = bp;
    bp->av_back->av_forw = bp;
}

/// @brief 根据物理设备块号读取缓存
/// @param blkno 所要进行读取的物理设备块号
/// @return  返回读取到的缓存块
Buf *BufferManager::Bread(int blkno)
{
    Buf *bp;
    // 根据设备号，字符块号申请缓存
    bp = this->GetBlk(blkno);
    // 如果在设备队列中找到所需缓存，即B_DONE已设置，就不需进行I/O操作
    if (bp->b_flags & Buf::B_DONE)
    {
        return bp;
    }

    // 没有找到相应缓存,I/O读操作，送入I/O请求队列
    bp->b_flags |= Buf::B_READ;
    bp->av_forw = this->devtab.av_forw;
    bp->av_back = &(this->devtab);
    bp->av_forw->av_back = bp;
    bp->av_back->av_forw = bp;

    // 开始读操作
    fstream fin;
    fin.open(DISK_PATH, ios::in | ios::binary);
    if (!fin.is_open())
    {
        cout << "无法打开一级磁盘文件myDisk.img" << endl;
        throw(ENOENT);
        return NULL;
    }
    // TODO:也没太搞懂为啥要读SIZE_BUFFER字节
    fin.seekg(streampos(blkno) * streampos(SIZE_BUFFER), ios::beg);
    fin.read(bp->b_addr, SIZE_BUFFER);
    fin.close();

    // 读操作完成
    // 更新标志
    bp->b_flags = Buf::BufFlag::B_DONE;

    // 从I/O请求队列取出
    bp->av_forw->av_back = bp->av_back;
    bp->av_back->av_forw = bp->av_forw;
    // 加入自由队列队尾
    bp->av_forw = this->bFreeList.av_forw;
    bp->av_back = &(this->bFreeList);
    bp->av_forw->av_back = bp;
    bp->av_back->av_forw = bp;

    return bp;
}

/// @brief 对磁盘进行读操作,但是设置了盘块号、偏移量和读取大小
/// @param buf      读取的数据存放的位置
/// @param blkno    所要进行读取的设备块号
/// @param offset   读取的起始位置
/// @param size     读取的大小
void BufferManager::Bread(char *buf, int blkno, int offset, int size)
{
    if (offset + size > SIZE_BUFFER)
    {
        cout << "读取的大小超过了一个盘块的大小" << endl;
        throw(EOVERFLOW);
    }
    if (buf == nullptr)
    {
        cout << "读取的缓冲区为空" << endl;
        throw(EINVAL);
    }
    Buf *bp;
    bp = this->Bread(blkno);
    memcpy(buf, bp->b_addr + offset, size);
    return;
}

void BufferManager::ClrBuf(Buf *bp)
{
    int *pInt = (int *)bp->b_addr;
    for (unsigned int i = 0; i < SIZE_BUFFER / sizeof(int); i++)
        pInt[i] = 0;
    bp->b_wcount = 0;
}

void BufferManager::SaveAll()
{
    for (int i = 0; i < NUM_BUF; i++)
        if (this->m_Buf[i].b_flags & Buf::B_DELWRI)
            this->Bwrite(&this->m_Buf[i]);
}