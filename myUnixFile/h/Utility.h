/*
 * @Author: yingxin wang
 * @Date: 2023-05-14 11:37:02
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-14 14:51:30
 * @Description: 使用到的工具函数
 */

#include "header.h"
#include "errno.h"

// 将char*转换为Directory*
Directory *char2Directory(char *ch);

// 将Directory*转换为char*
char *directory2Char(Directory *dir);

// 将char*转换为DiskInode*
DiskInode *char2DiskInode(char *ch);

// 将字符串按照分隔符分割
vector<string> stringSplit(const string &strIn, char delim);