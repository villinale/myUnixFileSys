/*
 * @Author: yingxin wang
 * @Date: 2023-05-18 16:12:47
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-18 16:12:58
 * @Description: ÇëÌîÐ´¼ò½é
 */
#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

File::File()
{
    this->f_inode = NULL;
    this->f_offset = 0;
    this->f_uid = -1;
    this->f_gid = -1;
}

void File::Clean()
{
    this->f_inode = NULL;
    this->f_offset = 0;
    this->f_uid = -1;
    this->f_gid = -1;
}