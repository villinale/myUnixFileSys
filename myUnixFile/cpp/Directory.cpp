#include "../h/header.h"
#include "../h/errno.h"

Directory::Directory() 
{
    for (int i = 0; i < NUM_SUB_DIR; i++)
    {
        this->d_inodenumber[i] = 0;
        strcpy(this->d_filename[i], "");
    }
}

/// @brief ����Ŀ¼��name��Inode��inumber����һ����Ŀ¼
/// @param name ��Ŀ¼��
/// @param inumber ��Ŀ¼Inode��
/// @return int 0��ʾ�ɹ�,-1��ʾʧ��
int Directory::mkdir(const char *name, const int inumber)
{
    if (inumber < 0)
        return -1;
    bool isFull = true; // �Ƿ��Ѿ�����
    int iinDir = 0;     // ���еĵ�һ��λ��
    for (int i = 0; i < NUM_SUB_DIR; i++)
    {
        // ����ҵ���Ӧ��Ŀ¼,˵���ļ��Ѵ���
        if (strcmp(this->d_filename[i], name) == 0)
        {
            cout << "�ļ��Ѵ���!" << endl;
            throw(EEXIST);
            return -1;
        }
        if (isFull && this->d_inodenumber[i] == 0)
        {
            isFull = false;
            iinDir = i;
        }
    }

    // ���Ŀ¼����
    if (isFull)
    {
        cout << "Ŀ¼����!" << endl;
        throw(ENOSPC);
        return -1;
    }

    // ����Ŀ¼����Inode��д��Ŀ¼
    strcpy(this->d_filename[iinDir], name);
    this->d_inodenumber[iinDir] = inumber;
    return 0;
}