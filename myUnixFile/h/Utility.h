/*
 * @Author: yingxin wang
 * @Date: 2023-05-14 11:37:02
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-21 20:23:15
 * @Description: 一些工具函数
 */

#include "header.h"
#include "errno.h"

Directory *char2Directory(char *ch);

DiskInode *char2DiskInode(char *ch);

SuperBlock *char2SuperBlock(char *ch);

UserTable *char2UserTable(char *ch);

char *directory2Char(Directory *dir);

char *uintArray2Char(int *arr, int len);

char *userTable2Char(UserTable *user);

char *spb2Char(SuperBlock *spb);

vector<string> stringSplit(const string &strIn, char delim);

string timestampToString(unsigned int timestamp);

string mode2String(unsigned short mode);