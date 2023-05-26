/*
 * @Author: yingxin wang
 * @Date: 2023-05-14 11:37:44
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-26 16:55:58
 * @Description: 请填写简介
 */
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
    catch (exception &e)
    {
        cerr << "Exception caught: " << e.what() << endl;
    }
}

// 将char*转换为SuperBlock*
SuperBlock *char2SuperBlock(char *ch)
{
    try
    {
        SuperBlock *objPtr = reinterpret_cast<SuperBlock *>(ch);
        return objPtr;
    }
    catch (exception &e)
    {
        cerr << "Exception caught: " << e.what() << endl;
    }
}

UserTable *char2UserTable(char *ch)
{
    try
    {
        UserTable *objPtr = reinterpret_cast<UserTable *>(ch);
        return objPtr;
    }
    catch (exception &e)
    {
        cerr << "Exception caught: " << e.what() << endl;
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
    catch (exception &e)
    {
        cerr << "Exception caught: " << e.what() << endl;
    }
}

char *uintArray2Char(unsigned int *arr, int len)
{
    try
    {
        char *ch = new char[len * sizeof(char)];
        for (int i = 0; i < len; i++)
            ch[i] = arr[i];
        return ch;
    }
    catch (exception &e)
    {
        cerr << "Exception caught: " << e.what() << endl;
    }
}

char *spb2Char(SuperBlock *spb)
{
    try
    {
        char *ch = new char[sizeof(SuperBlock) + 1];
        ch[sizeof(SuperBlock)] = '\0';
        memcpy(ch, reinterpret_cast<char *>(spb), sizeof(SuperBlock));
        return ch;
    }
    catch (exception &e)
    {
        cerr << "Exception caught: " << e.what() << endl;
    }
}

// 将UserTable*转换为char*
char *userTable2Char(UserTable *user)
{
    try
    {
        char *ch = reinterpret_cast<char *>(user);
        return ch;
    }
    catch (exception &e)
    {
        cerr << "Exception caught: " << e.what() << endl;
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
    catch (exception &e)
    {
        cerr << "Exception caught: " << e.what() << endl;
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

string timestampToString(unsigned int timestamp)
{
    time_t t = static_cast<time_t>(timestamp);
    tm *timeInfo = localtime(&t);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);

    return string(buffer);
}

string mode2String(unsigned short mode)
{
    /*
        OWNER_R = 0x400,
        OWNER_W = 0x200,
        OWNER_X = 0x100,
        GROUP_R = 0x40,
        GROUP_W = 0x20,
        GROUP_X = 0x10,
        OTHER_R = 0x4,
        OTHER_W = 0x2,
        OTHER_X = 0x1,*/
    std::string permissionString;

    // 所有者权限
    permissionString += (mode & Inode::OWNER_R) ? "r" : "-";
    permissionString += (mode & Inode::OWNER_W) ? "w" : "-";

    // 组权限
    permissionString += (mode & Inode::GROUP_R) ? "r" : "-";
    permissionString += (mode & Inode::GROUP_W) ? "w" : "-";

    // 其他用户权限
    permissionString += (mode & Inode::OTHER_R) ? "r" : "-";
    permissionString += (mode & Inode::OTHER_W) ? "w" : "-";

    return permissionString;
}