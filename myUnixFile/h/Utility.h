/*
 * @Author: yingxin wang
 * @Date: 2023-05-14 11:37:02
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-14 14:51:30
 * @Description: 使用到的工具函数
 */

#include "header.h"
#include "errno.h"

Directory *char2Directory(char *ch);
char *directory2Char(Directory *dir);
DiskInode *char2DiskInode(char *ch);