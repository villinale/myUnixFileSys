#include "../h/Utility.h"

/// @brief 将char*转换为Directory*
/// @param ch 要转换的字符
/// @return Directory* 转换后的Directory指针
Directory *char2Directory(char *ch)
{
    try
    {
        Directory *objPtr = reinterpret_cast<Directory *>(ch);
        return objPtr;
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

/// @brief 将Directory*转换为char*
/// @param dir 要转换的Directory指针
/// @return char* 转换后的字符
char *directory2Char(Directory *dir)
{
    try
    {
        char *ch = reinterpret_cast<char *>(dir);
        return ch;
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

/// @brief 将char*转换为DiskInode*
/// @param ch 要转换的字符
/// @return DiskInode* 转换后的DiskInode指针
DiskInode *char2DiskInode(char *ch)
{
    try
    {
        DiskInode *objPtr = reinterpret_cast<DiskInode *>(ch);
        return objPtr;
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}