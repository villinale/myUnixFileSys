/*
 * @Author: yingxin wang
 * @Date: 2023-05-14 11:37:02
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-21 20:23:15
 * @Description: ʹ�õ��Ĺ��ߺ���
 */

#include "header.h"
#include "errno.h"

// ��char*ת��ΪDirectory*
Directory *char2Directory(char *ch);

// ��char*ת��ΪDiskInode*
DiskInode *char2DiskInode(char *ch);

// ��char*ת��ΪSuperBlock*
SuperBlock *char2SuperBlock(char *ch);

UserTable *char2UserTable(char *ch);

// ��Directory*ת��Ϊchar*
char *directory2Char(Directory *dir);

// ��uint����ת��Ϊchar*
char *uintArray2Char(unsigned int *arr, int len);

// ��UserTable*ת��Ϊchar*
char *userTable2Char(UserTable *user);

// ���ַ������շָ����ָ�
vector<string> stringSplit(const string &strIn, char delim);