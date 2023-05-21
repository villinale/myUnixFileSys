/*
 * @Author: yingxin wang
 * @Date: 2023-05-21 16:31:20
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-21 16:54:54
 * @Description: FileSystem�ڲ����õĸ�����������
 */
#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

/// @brief FileSystem�๹�캯��
FileSystem::FileSystem()
{
}

/// @brief ��ȡ��ǰUserID
/// @return short ��ǰUserID
short FileSystem::getCurUserID()
{
    return this->curId;
}

/// @brief �ж�ָ�����Inode�Ƿ��Ѿ����ص��ڴ���
/// @param inumber ���Inode���
/// @return int ����Ѿ����أ������ڴ�Inode��inodeTable�ı�ţ����򷵻�-1
int FileSystem::IsLoaded(int inumber)
{
    // Ѱ��ָ�����Inode���ڴ�inode����
    for (int i = 0; i < NUM_INODE; i++)
        if (this->inodeTable[i].i_number == inumber && this->inodeTable[i].i_count != 0)
            return i;

    return -1;
}

/// @brief ������ȡָ�����Inode���ڴ���
/// @param inumber ���Inode���
/// @return Inode* �ڴ�Inode����
Inode *FileSystem::IGet(int inumber)
{
    Inode *pInode = NULL;
    // ��inodeTable�в���ָ�����Inode���ڴ�inode����
    int iInTable = this->IsLoaded(inumber);

    // ����ҵ��ˣ�ֱ�ӷ����ڴ�inode���������ü�����1
    if (iInTable != -1)
    {
        pInode = &this->inodeTable[iInTable];
        pInode->i_count++;
        pInode->i_atime = unsigned int(time(NULL));
        return pInode;
    }

    // û���ҵ����ȴ��ڴ�Inode�ڵ���з���һ��Inode,�ٴ�����ȡ
    for (int i = 0; i < NUM_INODE; i++)
        // ������ڴ�Inode���ü���Ϊ�㣬���Inode��ʾ���У�����ʹ��
        if (this->inodeTable[i].i_count == 0)
        {
            pInode = &(this->inodeTable[i]);
            break;
        }

    // ����ڴ�InodeTable�������׳��쳣
    if (pInode == NULL)
    {
        cout << "�ڴ�InodeTable����" << endl;
        throw(ENFILE);
        return pInode;
    }

    // ����ڴ�InodeTablû����������ȡָ�����Inode���ڴ���
    pInode->i_number = inumber;
    pInode->i_count++;
    pInode->i_atime = unsigned int(time(NULL));

    // �������Inode���뻺����
    Buf *pBuf = this->bufManager->Bread(POSITION_DISKINODE + (inumber - 1) / NUM_INODE_PER_BLOCK);
    // ���������е����Inode��Ϣ�������·�����ڴ�Inode��
    pInode->ICopy(pBuf, inumber);
    return pInode;
}

/// @brief �����ļ�·�����Ҷ�Ӧ��Inode
/// @param path �ļ�·��
/// @return Inode* ���ض�Ӧ��Inode�����û���ҵ�������NULL
Inode *FileSystem::NameI(string path)
{
    Inode *pInode;
    Buf *pbuf;
    Directory dir;
    vector<string> paths = stringSplit(path, '/'); // ����Ҫ���ļ��к��ļ������в��ܳ���"/"
    int ipaths = 0;
    bool isFind = false;

    // ��һ���ַ�Ϊ/��ʾ����·��
    if (path[0] == '/') // �Ӹ�Ŀ¼��ʼ����
        pInode = this->rootDirInode;
    else // ���·���Ĳ���
        pInode = this->curDirInode;

    while (true)
    {
        isFind = false;
        if (ipaths == paths.size()) // �������˵���ҵ��˶�Ӧ���ļ���Ŀ¼
            break;
        else if (ipaths >= paths.size())
            return NULL;

        // ������е�Inode��Ŀ¼�ļ�����ȷ,��Ϊ���������ѭ���Ż��ҵ��ļ�/Ŀ¼
        // һ���ҵ��ļ�/Ŀ¼����������ѭ��
        if (pInode->i_mode & Inode::INodeMode::IDIR)
        {
            // ����Ҫ���������̿��
            // ����Ŀ¼�ļ�ֻռһ���̿飬����ֻ��һ�Ϊ��
            int blkno = pInode->Bmap(0);
            // ��ȡ���̵�����
            pbuf = this->bufManager->Bread(blkno);

            // ������תΪĿ¼�ṹ
            Directory *fatherDir = char2Directory(pbuf->b_addr);

            // ѭ������Ŀ¼�е�ÿ��Ԫ��
            for (int i = 0; i < NUM_SUB_DIR; i++)
            {
                // ����ҵ���Ӧ��Ŀ¼
                if (paths[ipaths] == fatherDir->d_filename[i])
                {
                    ipaths++;
                    isFind = true;
                    pInode = this->IGet(fatherDir->d_inodenumber[i]);
                    break;
                }
            }

            // ���û���ҵ���Ӧ���ļ���Ŀ¼
            if (!isFind)
                return NULL;
        }
        else // ����Ŀ¼�ļ��Ǵ����
            return NULL;
    }

    // ���������˵���ҵ��˶�Ӧ���ļ�����Ŀ¼
    return pInode;
}

/// @brief ����pInode�Ƿ��и���mode��Ȩ��
/// @param pInode Ҫ���ҵ�Inode
/// @param mode   Ҫ���ҵ�Ȩ�ޣ�������FileSystem::FileMode
/// @return       �����Ȩ�ޣ�����1�����򷵻�0
int FileSystem::Access(Inode *pInode, unsigned int mode)
{
    // ����ǳ����û���ֱ�ӷ���
    if (this->curId == ROOT_ID)
        return 1;

    // ������ļ�������
    if (this->curId == pInode->i_uid)
    {
        if (mode == FileMode::EXC)
            return pInode->i_mode & Inode::INodeMode::OWNER_X;
        else if (mode == FileMode::WRITE) // дȨ��ǰ�����ж�Ȩ��
            return (pInode->i_mode & Inode::INodeMode::OWNER_R) && (pInode->i_mode & Inode::INodeMode::OWNER_W);
        else if (mode == FileMode::READ)
            return pInode->i_mode & Inode::INodeMode::OWNER_R;
        else
            return 0;
    }

    // ������ļ����������ڵ���
    if (this->curId == pInode->i_gid)
    {
        if (mode == FileMode::EXC)
            return pInode->i_mode & Inode::INodeMode::GROUP_X;
        else if (mode == FileMode::WRITE) // дȨ��ǰ�����ж�Ȩ��
            return (pInode->i_mode & Inode::INodeMode::GROUP_R) && (pInode->i_mode & Inode::INodeMode::GROUP_W);
        else if (mode == FileMode::READ)
            return pInode->i_mode & Inode::INodeMode::GROUP_R;
        else
            return 0;
    }

    // ����������û�
    if (mode == FileMode::EXC)
        return pInode->i_mode & Inode::INodeMode::OTHER_X;
    else if (mode == FileMode::WRITE) // дȨ��ǰ�����ж�Ȩ��
        return (pInode->i_mode & Inode::INodeMode::GROUP_R) && (pInode->i_mode & Inode::INodeMode::OTHER_W);
    else if (mode == FileMode::READ)
        return pInode->i_mode & Inode::INodeMode::OTHER_R;
    else
        return 0;
}

/// @brief ������������̿�
/// @return Buf* ���ط��䵽�Ļ��������������ʧ�ܣ�����NULL
Buf *FileSystem::Alloc()
{
    int blkno; // ���䵽�Ŀ��д��̿���
    Buf *pBuf;

    // ��������ջ������ȡ���д��̿���
    blkno = this->spb->s_free[--this->spb->s_nfree];

    // �ѷ��価���еĿ��д��̿飬ֱ�ӷ���
    if (0 == blkno)
    {
        this->spb->s_nfree = 0;
        cout << "��������!û�п����̿�" << endl;
        throw(ENOSPC);
        return NULL;
    }

    // ���д��̿��������ѿգ���һ����д��̿�ı�Ŷ���SuperBlock��s_free
    if (this->spb->s_nfree <= 0)
    {
        // ����ÿ��д��̿�
        pBuf = this->bufManager->Bread(blkno);

        int *p = (int *)pBuf->b_addr;

        // ���ȶ��������̿���s_nfre
        this->spb->s_nfree = (unsigned int)pBuf->b_addr[0];

        // ���ݿ����̿�����ȡ�����̿�������
        for (int i = 0; i < this->spb->s_nfree; i++)
            this->spb->s_free[i] = (unsigned int)pBuf->b_addr[i + 1];
    }

    // �����Ļ�����һ���д��̿飬���ظô��̿�Ļ���ָ��
    pBuf = this->bufManager->GetBlk(blkno); // Ϊ�ô��̿����뻺��
    this->bufManager->ClrBuf(pBuf);         // ��ջ����е�����

    return pBuf;
}

/// @brief ����һ�����е����Inode
/// @return Inode* ���ط��䵽���ڴ�Inode���������ʧ�ܣ�����NULL
Inode *FileSystem::IAlloc()
{
    Buf *pBuf;
    Inode *pNode;
    int ino = 0; // ���䵽�Ŀ������Inode���

    // SuperBlockֱ�ӹ���Ŀ���Inode�������ѿ�
    // ע���µĿ���Inode������
    if (this->spb->s_ninode <= 0)
    {
        // ���ζ������Inode���еĴ��̿飬�������п������Inode���������Inode������
        for (int i = 0; i < this->spb->s_isize; i++)
        {
            pBuf = this->bufManager->Bread(POSITION_DISKINODE + i / NUM_INODE_PER_BLOCK);

            // ��ȡ��������ַ
            int *p = (int *)pBuf->b_addr;

            // ���û�������ÿ�����Inode��i_mode != 0����ʾ�Ѿ���ռ��
            for (int j = 0; j < NUM_INODE_PER_BLOCK; j++)
            {
                ino++;
                int mode = *(p + j * sizeof(DiskInode) / sizeof(int));

                // �����Inode�ѱ�ռ�ã����ܼ������Inode������
                if (mode != 0)
                {
                    continue;
                }

                /*
                 * ������inode��i_mode==0����ʱ������ȷ��
                 * ��inode�ǿ��еģ���Ϊ�п������ڴ�inodeû��д��
                 * ������,����Ҫ���������ڴ�inode���Ƿ�����Ӧ����
                 * ��Դ���еõ���ע��
                 */
                if (this->IsLoaded(ino) == -1)
                {
                    // �����Inodeû�ж�Ӧ���ڴ濽��������������Inode������
                    this->spb->s_inode[this->spb->s_ninode++] = ino;

                    /* ��������������Ѿ�װ�����򲻼������� */
                    if (this->spb->s_ninode >= 100)
                        break;
                }
            }

            // ��������������Ѿ�װ�����򲻼�������
            if (this->spb->s_ninode >= 100)
                break;
        }
    }

    // ��������˻�û�п������Inode������NULL
    if (this->spb->s_ninode <= 0)
    {
        cout << "���������Inode������!" << endl;
        throw(ENOSPC);
        return NULL;
    }

    // ���ڴ��������ڴ�Inode
    int inumber = this->spb->s_inode[--this->spb->s_ninode];
    pNode = IGet(inumber);
    if (NULL == pNode) // �����޸Ĳ���
        return NULL;

    if (0 == pNode->i_mode)
    {
        pNode->Clean();
        return pNode;
    }
}

/// @brief ������д��ļ����ƿ�File�ṹ
/// @return File* ���ط��䵽�Ĵ��ļ����ƿ�File�ṹ���������ʧ�ܣ�����NULL
File *FileSystem::FAlloc()
{
    for (int i = 0; i < NUM_FILE; i++)
        if (this->openFileTable[i].f_inode == NULL)
            return &this->openFileTable[i];
    return NULL;
}

/// @brief �ͷ�InodeTable�е�Inode�ڵ�
/// @param pNode
void FileSystem::IPut(Inode *pNode)
{
    // ��ǰ����Ϊ���ø��ڴ�Inode��Ψһ���̣���׼���ͷŸ��ڴ�Inode
    if (pNode->i_count == 1)
    {
        pNode->i_mtime = unsigned int(time(NULL));
        pNode->i_atime = unsigned int(time(NULL));
        pNode->WriteI();
        pNode->Clean();
    }

    // �����ڴ�Inode�����ü��������ѵȴ�����
    pNode->i_count--;
}

/// @brief ��������д�ش���
void FileSystem::WriteSpb()
{
    Buf *bp = this->bufManager->Bread(POSITION_SUPERBLOCK);
    memcpy_s(bp->b_addr, SIZE_BUFFER, (const char *)this->spb, SIZE_BUFFER);
    this->bufManager->Bwrite(bp);
    bp = this->bufManager->Bread(POSITION_SUPERBLOCK + 1);
    memcpy_s(bp->b_addr, SIZE_BUFFER, (const char *)this->spb + SIZE_BLOCK, SIZE_BUFFER);
    this->bufManager->Bwrite(bp);
}
