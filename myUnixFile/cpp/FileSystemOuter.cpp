/*
 * @Author: yingxin wang
 * @Date: 2023-05-21 16:44:37
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-24 15:03:03
 * @Description: FileSystem����main�п��Ե��õĿɽ����ĺ���,��������ֻ���
 */

#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

void FileSystem::help()
{
    // fformat\ls\mkdir\fcreat\fopen\fclose\fread\fwrite\flseek\fdelete
    printf("--------------Ŀ¼���---------------\n");
    printf("ls                     �鿴��Ŀ¼\n");
    printf("cd <dir-name>          ������Ϊdir-name����Ŀ¼\n");
    printf("rmdir <dir-name>       ɾ������Ϊdir-name����Ŀ¼\n");
    printf("--------------�ļ����---------------\n");
    printf("open <file-path>       ��·��Ϊfile-path���ļ�\n");
    printf("                       ֧����/��ͷ�ĴӸ�Ŀ¼�� �� ����/��ͷ�Ĵӵ�ǰĿ¼��\n");
    printf("close <file-path>      �ر�·��Ϊfile-path���ļ�\n");
    printf("open <file-path>       ��·��Ϊfile-path���ļ�\n");

    printf("----------------����----------------\n");
    printf("fformat             ��ʽ���ļ�ϵͳ\n");
}

/// @brief ��ʼ��ϵͳ���������д����ļ������
void FileSystem::init()
{
    fstream fd(DISK_PATH, ios::out | ios::in | ios::binary);
    // ���û�д��ļ��������ʾ��Ϣ��throw����
    if (!fd.is_open())
    {
        cout << "�޷���һ���ļ�myDisk.img" << endl;
        throw(errno);
    }
    fd.close();

    // �Ի���������ݽ��г�ʼ��
    this->bufManager = new BufferManager();

    // ��ȡ������
    Buf *buf = this->bufManager->Bread(POSITION_SUPERBLOCK);
    this->spb = char2SuperBlock(buf->b_addr);

    // ��ȡ��Ŀ¼Inode
    buf = this->bufManager->Bread(POSITION_DISKINODE);
    this->rootDirInode = this->IAlloc();
    this->rootDirInode->ICopy(buf, ROOT_DIR_INUMBER);
    this->curDirInode = this->rootDirInode;
    this->curId = ROOT_ID; // ��������ʼ��
    this->curDir = "/";

    // ��ȡ�û���Ϣ��
    // ����ֱ�ӵ���this->fopen����ΪuserTable����û�г�ʼ��
    Inode *pinode = this->NameI("/etc/userTable.txt");
    // û���ҵ���Ӧ��Inode
    if (pinode == NULL)
    {
        cout << "û���ҵ�/etc/userTable.txt!" << endl;
        throw(ENOENT);
        return;
    }
    // ����ҵ����ж���Ҫ�ҵ��ļ��ǲ����ļ�����
    if (!(pinode->i_mode & Inode::INodeMode::IFILE))
    {
        cout << "����һ����ȷ��/etc/userTable.txt�ļ�!" << endl;
        throw(ENOTDIR);
        return;
    }
    Buf *bp = this->bufManager->Bread(pinode->Bmap(0)); // userTable.txt�ļ�����ֻռһ���̿��С
    this->userTable = char2UserTable(bp->b_addr);
}

/// @brief ��Ŀ¼
void FileSystem::ls()
{
    Directory *dir = this->curDirInode->GetDir();
    int i;
    for (i = 0; i < NUM_SUB_DIR; i++)
    {
        if (dir->d_inodenumber[i] == 0)
            break;
        cout << dir->d_filename[i] << "\t";
    }
    cout << endl;
}

/// @brief ����Ŀ¼
/// @param subname ��Ŀ¼����
void FileSystem::cd(string subname)
{
    // ���˵���Ŀ¼�����
    if (subname == "..")
    {
        if (this->curDir == "/") // ��Ŀ¼���
            return;

        int id = this->curDir.find_last_of('/');
        if (id == 0) // ˵�����Ǹ�Ŀ¼�µ���Ŀ¼
            this->curDir = "/";
        else
            this->curDir = this->curDir.substr(0, this->curDir.find_last_of('/'));

        this->IPut(this->curDirInode); // �ͷŵ�ǰĿ¼��Inode
        // ���˸�Ŀ¼��Inode
        this->curDirInode = this->IGet(this->curDirInode->GetParentInumber());

        return;
    }
    else if (subname == ".") // ��ǰĿ¼���
    {
        return;
    }

    // ��ͨ������������ļ�����
    Directory *dir = this->curDirInode->GetDir();
    int i;
    for (i = 0; i < NUM_SUB_DIR; i++)
    {
        if (dir->d_inodenumber[i] == 0)
            continue;
        if (strcmp(dir->d_filename[i], subname.c_str()) == 0)
            break;
    }
    if (i == NUM_SUB_DIR)
    {
        cout << "Ŀ¼������!" << endl;
        return;
    }
    if (this->curDirInode == this->rootDirInode) // rootĿ¼ֻ������Ŀ¼��
        this->curDir += subname;
    else
        this->curDir += "/" + subname;
    this->curDirInode = this->IGet(dir->d_inodenumber[i]);
}

/// @brief ɾ����Ŀ¼
/// @param subname ��Ŀ¼����
void FileSystem::rmdir(string subname)
{
    if (subname == "." || subname == "..")
    {
        cout << "����ɾ����ǰĿ¼��Ŀ¼!" << endl;
        return;
    }

    // ��ͨ�����ɾ�����ļ���
    Directory *dir = this->curDirInode->GetDir();
    int i;
    for (i = 0; i < NUM_SUB_DIR; i++)
    {
        if (dir->d_inodenumber[i] == 0)
            continue;
        if (strcmp(dir->d_filename[i], subname.c_str()) == 0)
            break;
    }
    if (i == NUM_SUB_DIR)
    {
        cout << "��Ҫɾ����Ŀ¼������!" << endl;
        return;
    }

    // ��ȡ��Ҫɾ�����ļ��е�Inode
    Inode *pDeleteInode = this->IGet(dir->d_inodenumber[i]);
    if (NULL == pDeleteInode) // ���������Ӧ�ò����ڣ�����дһ��
    {
        cout << "��Ҫɾ����Ŀ¼������!" << endl;
        return;
    }
    if (pDeleteInode->i_mode & Inode::INodeMode::IFILE) // ������ļ�����
    {
        cout << "��������ȷ����Ŀ¼��!" << endl;
        return;
    }
    Directory *deletedir = pDeleteInode->GetDir();
    for (int i = 2; i < NUM_SUB_DIR; i++) // �ӵ�3��Ŀ¼�ʼ������Ŀ¼���ǰ���������Լ��͸���
    {
        if (deletedir->d_inodenumber[i] != 0)
        {
            cout << "Ŀ¼�ǿգ�����ɾ��!" << endl;
            return;
        }
    }

    // ɾ����Ŀ¼inode,��ʵֻ��unlink
    pDeleteInode->i_nlink--;
    this->IPut(pDeleteInode); // ��i_nlinkΪ0ʱ�����ͷ�Inode

    // ɾ����Ŀ¼�µ���Ŀ¼��
    dir->deletei(i);
}

void FileSystem::openFile(string path)
{
    int fd = this->fopen(path);
    //File *fp =
}

void FileSystem::login()
{
    string name, pswd;
    short id;
    while (true)
    {
        cout << "�������û���:";
        cin >> name;
        cout << "����������:";
        cin >> pswd;
        if (name.empty() || pswd.empty())
        {
            cout << "����Ƿ�!" << endl;
            continue;
        }
        id = this->userTable->FindUser(name.c_str(), pswd.c_str());
        if (id == -1)
        {
            cout << "�û�������!" << endl;
            continue;
        }
        else
            break;
    }
    cout << "��½�ɹ�!" << endl;
    this->curId = id;
    this->curName = name;
}

void FileSystem::format()
{
    cout << this->curName << this->curDir << ">"
         << "ȷ��Ҫ���и�ʽ��?[y] ";
    string strIn;
    getline(cin, strIn);
    cout << "���ڸ�ʽ��" << endl
         << endl;
    if (strIn == "y" || strIn == "Y")
    {
        this->fformat();
    }
    cout << "��ʽ������" << endl
         << endl;
}

void FileSystem::fun()
{
    this->login();

    cout << "����help���Բ鿴�����嵥" << endl;
    vector<string> input;
    string strIn;
    while (true)
    {
        cout << this->curName << this->curDir << ">";
        getline(cin, strIn);
        input = stringSplit(strIn, ' ');
        if (input.size() == 0)
            continue;

        try
        {
            // Ŀ¼����
            if (input[0] == "ls") // �鿴��Ŀ¼
                this->ls();
            else if (input[0] == "cd") // ������Ŀ¼
                this->cd(input[1]);
            else if (input[0] == "rmdir") // ɾ����Ŀ¼
                this->rmdir(input[1]);
            // �ļ�����
            else if (input[0] == "open")
            {
            }
            else if (input[0] == "format")
            {
                this->format();
                this->exit();
                this->login();
            }
            else if (input[0] == "help")
                this->help();
            else if (input[0] == "exit")
            {
                this->exit();
                break;
            }
        }
        catch (int &e)
        {
            cout << "error code" << e << endl;
            cout << "��linux�����뱣��һ��" << endl
                 << endl;
        }
    }
}