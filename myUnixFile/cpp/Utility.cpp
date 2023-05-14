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

/// @brief 将字符串按照分隔符分割
/// @param strIn  要分割的字符串
/// @param delim  分隔符
/// @return  vector<string> 分割后的字符串数组
vector<string> stringSplit(const string &strIn, char delim)
{
    char *str = const_cast<char *>(strIn.c_str());
    string s;
    s.append(1, delim);
    vector<string> elems;
    char *splitted = strtok(str, s.c_str());
    while (splitted != NULL)
    {
        elems.push_back(string(splitted));
        splitted = strtok(NULL, s.c_str());
    }
    return elems;
}