/*
 * @Author: yingxin wang
 * @Date: 2023-05-14 11:37:44
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-21 20:26:24
 * @Description: ����д���
 */
#include "../h/Utility.h"

/// @brief ��char*ת��ΪDirectory*
/// @param ch Ҫת�����ַ�
/// @return Directory* ת�����Directoryָ��
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

// ��char*ת��ΪSuperBlock*
SuperBlock *char2SuperBlock(char *ch)
{
    try
    {
        SuperBlock *objPtr = reinterpret_cast<SuperBlock *>(ch);
        return objPtr;
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

UserTable *char2UserTable(char *ch)
{
    try
    {
        UserTable *objPtr = reinterpret_cast<UserTable *>(ch);
        return objPtr;
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

/// @brief ��Directory*ת��Ϊchar*
/// @param dir Ҫת����Directoryָ��
/// @return char* ת������ַ�
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

char *uintArray2Char(unsigned int *arr, int len)
{
    try
    {
        char *ch = new char[len * sizeof(char)];
        for (int i = 0; i < len; i++)
            ch[i] = arr[i];
        return ch;
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

// ��UserTable*ת��Ϊchar*
char *userTable2Char(UserTable *user)
{
    try
    {
        char *ch = reinterpret_cast<char *>(user);
        return ch;
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

/// @brief ��char*ת��ΪDiskInode*
/// @param ch Ҫת�����ַ�
/// @return DiskInode* ת�����DiskInodeָ��
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

/// @brief ���ַ������շָ����ָ�
/// @param strIn  Ҫ�ָ���ַ���
/// @param delim  �ָ���
/// @return  vector<string> �ָ����ַ�������
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