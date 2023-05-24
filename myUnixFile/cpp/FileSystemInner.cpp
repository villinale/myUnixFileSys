/*
 * @Author: yingxin wang
 * @Date: 2023-05-21 16:31:20
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-24 19:37:14
 * @Description: FileSystem内部调用的各个函数内容
 */
#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

/// @brief FileSystem类构造函数
FileSystem::FileSystem()
{
}

/// @brief 获取当前UserID
/// @return short 当前UserID
short FileSystem::getCurUserID()
{
    return this->curId;
}

/// @brief 判断指定外存Inode是否已经加载到内存中
/// @param inumber 外存Inode编号
/// @return int 如果已经加载，返回内存Inode在inodeTable的编号，否则返回-1
int FileSystem::IsLoaded(int inumber)
{
    // 寻找指定外存Inode的内存inode拷贝
    for (int i = 0; i < NUM_INODE; i++)
        if (this->inodeTable[i].i_number == inumber && this->inodeTable[i].i_count != 0)
            return i;

    return -1;
}

/// @brief 从外存读取指定外存Inode到内存中
/// @param inumber 外存Inode编号
/// @return Inode* 内存Inode拷贝
Inode *FileSystem::IGet(int inumber)
{
    Inode *pInode = NULL;
    // 在inodeTable中查找指定外存Inode的内存inode拷贝
    int iInTable = this->IsLoaded(inumber);

    // 如果找到了，直接返回内存inode拷贝，引用计数加1
    if (iInTable != -1)
    {
        pInode = &this->inodeTable[iInTable];
        pInode->i_count++;
        pInode->i_atime = unsigned int(time(NULL));
        return pInode;
    }

    // 没有找到，先从内存Inode节点表中分配一个Inode,再从外存读取
    for (int i = 0; i < NUM_INODE; i++)
        // 如果该内存Inode引用计数为零，则该Inode表示空闲，可以使用
        if (this->inodeTable[i].i_count == 0)
        {
            pInode = &(this->inodeTable[i]);
            break;
        }

    // 如果内存InodeTable已满，抛出异常
    if (pInode == NULL)
    {
        cout << "内存InodeTable已满" << endl;
        throw(ENFILE);
        return pInode;
    }

    // 如果内存InodeTabl没满，从外存读取指定外存Inode到内存中
    pInode->i_number = inumber;
    pInode->i_count++;
    pInode->i_atime = unsigned int(time(NULL));

    // 将该外存Inode读入缓冲区
    Buf *pBuf = this->bufManager->Bread(POSITION_DISKINODE + (inumber - 1) / NUM_INODE_PER_BLOCK);
    // 将缓冲区中的外存Inode信息拷贝到新分配的内存Inode中
    pInode->ICopy(pBuf, inumber);
    return pInode;
}

/// @brief 根据文件路径查找对应的Inode
/// @param path 文件路径
/// @return Inode* 返回对应的Inode，如果没有找到，返回NULL
Inode *FileSystem::NameI(string path)
{
    Inode *pInode;
    Buf *pbuf;
    vector<string> paths = stringSplit(path, '/'); // 所以要求文件夹和文件的名中不能出现"/"
    int ipaths = 0;
    bool isFind = false;

    // 第一个字符为/表示绝对路径
    if (path.size() != 0 && path[0] == '/') // 从根目录开始查找
        pInode = this->rootDirInode;
    else // 相对路径的查找
        pInode = this->curDirInode;
    int blkno = pInode->Bmap(0);
    // 读取磁盘的数据

    while (true)
    {
        isFind = false;
        // 包含path为空的情况
        if (ipaths == paths.size()) // 这种情况说明找到了对应的文件或目录
            break;
        else if (ipaths >= paths.size())
            return NULL;

        // 如果现有的Inode是目录文件才正确,因为在这里面的循环才会找到文件/目录
        // 一旦找到文件/目录不会进入这个循环
        if (pInode->i_mode & Inode::INodeMode::IDIR)
        {
            // 计算要读的物理盘块号
            // 由于目录文件只占一个盘块，所以只有一项不为空
            int blkno = pInode->Bmap(0);
            // 读取磁盘的数据
            pbuf = this->bufManager->Bread(blkno);

            // 将数据转为目录结构
            Directory *fatherDir = char2Directory(pbuf->b_addr);

            // 循环查找目录中的每个元素
            for (int i = 0; i < NUM_SUB_DIR; i++)
            {
                // 如果找到对应子目录
                if (paths[ipaths] == fatherDir->d_filename[i])
                {
                    ipaths++;
                    isFind = true;
                    pInode = this->IGet(fatherDir->d_inodenumber[i]);
                    break;
                }
            }

            // 如果没有找到对应的文件或目录
            if (!isFind)
                return NULL;
        }
        else // 不是目录文件是错误的
            return NULL;
    }

    // 到这个部分说明找到了对应的文件或者目录
    return pInode;
}

/// @brief 查找pInode是否有给定mode的权限
/// @param pInode 要查找的Inode
/// @param mode   要查找的权限，定义间见FileSystem::FileMode
/// @return       如果有权限，返回1，否则返回0
int FileSystem::Access(Inode *pInode, unsigned int mode)
{
    // 如果是超级用户，直接返回
    if (this->curId == ROOT_ID)
        return 1;

    // 如果是文件所有者
    if (this->curId == pInode->i_uid)
    {
        if (mode == FileMode::EXC)
            return pInode->i_mode & Inode::INodeMode::OWNER_X;
        else if (mode == FileMode::WRITE) // 写权力前提是有读权力
            return (pInode->i_mode & Inode::INodeMode::OWNER_R) && (pInode->i_mode & Inode::INodeMode::OWNER_W);
        else if (mode == FileMode::READ)
            return pInode->i_mode & Inode::INodeMode::OWNER_R;
        else
            return 0;
    }

    // 如果是文件所有者所在的组
    if (this->curId == pInode->i_gid)
    {
        if (mode == FileMode::EXC)
            return pInode->i_mode & Inode::INodeMode::GROUP_X;
        else if (mode == FileMode::WRITE) // 写权力前提是有读权力
            return (pInode->i_mode & Inode::INodeMode::GROUP_R) && (pInode->i_mode & Inode::INodeMode::GROUP_W);
        else if (mode == FileMode::READ)
            return pInode->i_mode & Inode::INodeMode::GROUP_R;
        else
            return 0;
    }

    // 如果是其他用户
    if (mode == FileMode::EXC)
        return pInode->i_mode & Inode::INodeMode::OTHER_X;
    else if (mode == FileMode::WRITE) // 写权力前提是有读权力
        return (pInode->i_mode & Inode::INodeMode::GROUP_R) && (pInode->i_mode & Inode::INodeMode::OTHER_W);
    else if (mode == FileMode::READ)
        return pInode->i_mode & Inode::INodeMode::OTHER_R;
    else
        return 0;
}

/// @brief 获取绝对路径，假设路径正确
/// @param path 相对路径或绝对路径
/// @return string 绝对路径
string FileSystem::GetAbsolutionPath(string path)
{
    if (path[0] == '/')
        return path;
    else
        return this->curDir + path;
}

/// @brief 分配空闲数据盘块
/// @return Buf* 返回分配到的缓冲区，如果分配失败，返回NULL
Buf *FileSystem::Alloc()
{
    int blkno; // 分配到的空闲磁盘块编号
    Buf *pBuf;

    // 从索引表“栈顶”获取空闲磁盘块编号
    blkno = this->spb->s_free[--this->spb->s_nfree];

    // 已分配尽所有的空闲磁盘块，直接返回
    if (0 == blkno)
    {
        this->spb->s_nfree = 0;
        cout << "磁盘已满!没有空余盘块" << endl;
        throw(ENOSPC);
        return NULL;
    }

    // 空闲磁盘块索引表已空，下一组空闲磁盘块的编号读入SuperBlock的s_free
    if (this->spb->s_nfree <= 0)
    {
        // 读入该空闲磁盘块
        pBuf = this->bufManager->Bread(blkno);

        int *p = (int *)pBuf->b_addr;

        // 首先读出空闲盘块数s_nfre
        this->spb->s_nfree = (unsigned int)pBuf->b_addr[0];

        // 根据空闲盘块数读取空闲盘块索引表
        for (int i = 0; i < this->spb->s_nfree; i++)
            this->spb->s_free[i] = (unsigned int)pBuf->b_addr[i + 1];
    }

    // 这样的话分配一空闲磁盘块，返回该磁盘块的缓存指针
    pBuf = this->bufManager->GetBlk(blkno); // 为该磁盘块申请缓存
    this->bufManager->ClrBuf(pBuf);         // 清空缓存中的数据

    return pBuf;
}

/// @brief 分配一个空闲的外存Inode
/// @return Inode* 返回分配到的内存Inode，如果分配失败，返回NULL
Inode *FileSystem::IAlloc()
{
    Buf *pBuf;
    Inode *pNode;
    int ino = 0; // 分配到的空闲外存Inode编号

    // SuperBlock直接管理的空闲Inode索引表已空
    // 注入新的空闲Inode索引表
    if (this->spb->s_ninode <= 0)
    {
        // 依次读入磁盘Inode区中的磁盘块，搜索其中空闲外存Inode，记入空闲Inode索引表
        for (int i = 0; i < this->spb->s_isize; i++)
        {
            pBuf = this->bufManager->Bread(POSITION_DISKINODE + i / NUM_INODE_PER_BLOCK);

            // 获取缓冲区首址
            int *p = (int *)pBuf->b_addr;

            // 检查该缓冲区中每个外存Inode的i_mode != 0，表示已经被占用
            for (int j = 0; j < NUM_INODE_PER_BLOCK; j++)
            {
                ino++;
                int mode = *(p + j * sizeof(DiskInode) / sizeof(int));

                // 该外存Inode已被占用，不能记入空闲Inode索引表
                if (mode != 0)
                {
                    continue;
                }

                /*
                 * 如果外存inode的i_mode==0，此时并不能确定
                 * 该inode是空闲的，因为有可能是内存inode没有写到
                 * 磁盘上,所以要继续搜索内存inode中是否有相应的项
                 * 从源码中得到的注释
                 */
                if (this->IsLoaded(ino) == -1)
                {
                    // 该外存Inode没有对应的内存拷贝，将其记入空闲Inode索引表
                    this->spb->s_inode[this->spb->s_ninode++] = ino;

                    // 如果空闲索引表已经装满，则不继续搜索
                    if (this->spb->s_ninode >= 100)
                        break;
                }
            }

            // 如果空闲索引表已经装满，则不继续搜索
            if (this->spb->s_ninode >= 100)
                break;
        }
    }

    // 如果这样了还没有可用外存Inode，返回NULL
    if (this->spb->s_ninode <= 0)
    {
        cout << "磁盘上外存Inode区已满!" << endl;
        throw(ENOSPC);
        return NULL;
    }

    // 现在从外存分配内存Inode
    int inumber = this->spb->s_inode[--this->spb->s_ninode];
    pNode = IGet(inumber);
    if (NULL == pNode) // 不做修改操作
        return NULL;

    if (0 == pNode->i_mode)
    {
        pNode->Clean();
        return pNode;
    }
    return pNode;
}

/// @brief 分配空闲打开文件控制块File结构
/// @return File* 返回分配到的打开文件控制块File结构，如果分配失败，返回NULL
File *FileSystem::FAlloc(int &iloc)
{
    for (int i = 0; i < NUM_FILE; i++)
        if (this->openFileTable[i].f_inode == NULL)
        {
            iloc = i;
            return &this->openFileTable[i];
        }
    iloc = -1;
    return NULL;
}

/// @brief 释放指定的数据盘块
/// @param blkno 要释放的数据盘块号
void FileSystem::Free(int blkno)
{
    Buf *pBuf;
    if (blkno < POSITION_BLOCK)
    {
        cout << "不能释放系统盘块" << endl;
        return;
    }

    // 如果先前系统中已经没有空闲盘块，现在释放的是系统中第1块空闲盘块
    if (this->spb->s_nfree <= 0)
    {
        this->spb->s_nfree = 1;
        this->spb->s_free[0] = 0;
    }

    // 如果SuperBlock中直接管理空闲磁盘块号的栈已满
    if (this->spb->s_nfree >= NUM_FREE_BLOCK_GROUP)
    {
        // 分配一个新缓存块，用于存放新的空闲磁盘块号
        pBuf = this->bufManager->GetBlk(blkno);

        // 将s_nfree和s_free[100]写入回收盘块的前101个字节
        // s_free[0]=回收的盘块号
        // s_nfree=1
        // 以上摘自PPT
        unsigned int *stack = new unsigned int[NUM_FREE_BLOCK_GROUP + 1]{0}; // 第一位是链接的上一组的盘块个数
        stack[0] = this->spb->s_nfree;                                       // 第一位是链接的上一组的盘块个数
        for (int i = 0; i < NUM_FREE_BLOCK_GROUP; i++)
            stack[i + 1] = this->spb->s_free[i];
        memcpy(pBuf->b_addr, uintArray2Char(stack, NUM_FREE_BLOCK_GROUP + 1), sizeof(int)* NUM_FREE_BLOCK_GROUP + 1);
        bufManager->Bwrite(pBuf);

        this->spb->s_nfree = 0;
    }

    // 释放数据盘块号
    this->spb->s_free[this->spb->s_nfree++] = blkno;
}

/// @brief 释放外存Inode节点
/// @param number 外存Inode编号
void FileSystem::IFree(int number)
{
    // spb够用
    if (this->spb->s_ninode >= NUM_FREE_INODE)
        return;

    // 将该外存Inode的编号记入空闲Inode索引表，后来的会直接覆盖掉它的内容
    this->spb->s_inode[this->spb->s_ninode++] = number;
}

/// @brief 释放InodeTable中的Inode节点
/// @param pNode
void FileSystem::IPut(Inode *pNode)
{
    // 当前进程为引用该内存Inode的唯一进程，且准备释放该内存Inode
    if (pNode->i_count == 1)
    {
        pNode->i_mtime = unsigned int(time(NULL));
        pNode->i_atime = unsigned int(time(NULL));
        pNode->WriteI();
        // TODO:在这里源码没搞懂：为什么时先释放了外存inode，还把内存inode信息更新
        // 该文件已经没有目录路径指向它
        if (pNode->i_nlink <= 0)
        {
            // 释放该文件占据的数据盘块
            pNode->ITrunc();
            pNode->i_mode = 0;
            // 释放对应的外存Inode
            this->IFree(pNode->i_number);
        }
        pNode->Clean();
    }

    // 减少内存Inode的引用计数
    pNode->i_count--;
}

/// @brief 将超级块写回磁盘
void FileSystem::WriteSpb()
{
    char *p = spb2Char(this->spb);
    Buf *bp = this->bufManager->Bread(POSITION_SUPERBLOCK);
    memcpy(bp->b_addr, p, SIZE_BUFFER);
    this->bufManager->Bwrite(bp);
    bp = this->bufManager->Bread(POSITION_SUPERBLOCK + 1);
    // 这里之前有内存泄漏
    memcpy(bp->b_addr + SIZE_BLOCK, p + SIZE_BLOCK, sizeof(SuperBlock) - SIZE_BLOCK);
    this->bufManager->Bwrite(bp);
}
