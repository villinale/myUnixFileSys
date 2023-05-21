/*
 * @Author: yingxin wang
 * @Date: 2023-05-21 16:44:37
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-22 00:13:30
 * @Description: FileSystem����main�п��Ե��õĿɽ����ĺ���,��������ֻ���
 */

#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

void FileSystem::help()
{
    // fformat\ls\mkdir\fcreat\fopen\fclose\fread\fwrite\flseek\fdelete
    cout << "����        ˵��" << endl;
    printf("cd          ��ʽ���ļ�ϵͳ\n");
    printf("fformat     ��ʽ���ļ�ϵͳ\n");
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

        this->curDir = this->curDir.substr(0, this->curDir.find_last_of('/'));

        this->IPut(this->curDirInode); // �ͷŵ�ǰĿ¼��Inode
        // ���˸�Ŀ¼��Inode
        this->curDirInode = this->IGet(this->curDirInode->GetParentInumber());
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
    this->curDirInode = this->IGet(dir->d_inodenumber[i]);
    this->curDir += "/" + subname;
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

void FileSystem::fun()
{
    cout << "����help���Բ鿴�����嵥" << endl;
    vector<string> input;
    string strIn;
    while (true)
    {
        cout << this->curName << "\\" << this->curDir << ">";
        getline(cin, strIn);
        input = stringSplit(strIn, ' ');
        if (input.size() == 0)
            continue;

        if (input[0] == "ls")
            this->ls();
        else if (input[0] == "cd")
            this->cd(input[1]);
    }
}