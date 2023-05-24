/*
 * @Author: yingxin wang
 * @Date: 2023-05-14 11:37:02
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-21 20:23:15
 * @Description: 使用到的工具函数
 */

#include "header.h"
#include "errno.h"

// 将char*转换为Directory*
Directory *char2Directory(char *ch);

// 将char*转换为DiskInode*
DiskInode *char2DiskInode(char *ch);

// 将char*转换为SuperBlock*
SuperBlock *char2SuperBlock(char *ch);

UserTable *char2UserTable(char *ch);

// 将Directory*转换为char*
char *directory2Char(Directory *dir);

// 将uint数组转换为char*
char *uintArray2Char(unsigned int *arr, int len);

// 将UserTable*转换为char*
char *userTable2Char(UserTable *user);

char* spb2Char(SuperBlock* spb);

// 将字符串按照分隔符分割
vector<string> stringSplit(const string &strIn, char delim);