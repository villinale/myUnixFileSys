#include "../h/header.h"
#include "../h/errno.h"

Directory::Directory() 
{
    for (int i = 0; i < NUM_SUB_DIR; i++)
    {
        this->d_inodenumber[i] = 0;
        strcpy(this->d_filename[i], "");
    }
}

/// @brief 根据目录名name和Inode号inumber创建一个子目录
/// @param name 子目录名
/// @param inumber 子目录Inode号
/// @return int 0表示成功,-1表示失败
int Directory::mkdir(const char *name, const int inumber)
{
    if (inumber < 0)
        return -1;
    bool isFull = true; // 是否已经满了
    int iinDir = 0;     // 空闲的第一个位置
    for (int i = 0; i < NUM_SUB_DIR; i++)
    {
        // 如果找到对应子目录,说明文件已存在
        if (strcmp(this->d_filename[i], name) == 0)
        {
            cout << "文件已存在!" << endl;
            throw(EEXIST);
            return -1;
        }
        if (isFull && this->d_inodenumber[i] == 0)
        {
            isFull = false;
            iinDir = i;
        }
    }

    // 如果目录已满
    if (isFull)
    {
        cout << "目录已满!" << endl;
        throw(ENOSPC);
        return -1;
    }

    // 将子目录名和Inode号写入目录
    strcpy(this->d_filename[iinDir], name);
    this->d_inodenumber[iinDir] = inumber;
    return 0;
}