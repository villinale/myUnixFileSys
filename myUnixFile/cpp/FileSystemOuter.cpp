/*
 * @Author: yingxin wang
 * @Date: 2023-05-21 16:44:37
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-24 20:46:40
 * @Description: FileSystem����main�п��Ե��õĿɽ����ĺ���,��������ֻ���
 */

#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

void FileSystem::help()
{
    // fformat\ls\mkdir\fcreat\fopen\fclose\fread\fwrite\flseek\fdelete
    printf("���������д�'<>'�����Ǳ���ģ���'[]'�����ǿ�ѡ���\n");
    printf("--------------Ŀ¼���---------------\n");
    printf("ls                                      �鿴��ǰĿ¼�µ���Ŀ¼\n");
    printf("cd <dir-name>                           ���ڵ�ǰĿ¼������Ϊdir-name����Ŀ¼\n");
    printf("mkdir <dir-name>                        �����ڵ�ǰĿ¼������Ϊdir-name����Ŀ¼\n");
    printf("rmdir <dir-name>                        ɾ���ڵ�ǰĿ¼������Ϊdir-name����Ŀ¼\n");
    printf("--------------�ļ����---------------\n");
    printf("touch <file-name>                       �ڵ�ǰĿ¼�´�������Ϊfile-name���ļ�\n");
    printf("open <file-name>                        �򿪵�ǰĿ¼������Ϊfile-name���ļ�\n");
    printf("close <file-name>                       �رյ�ǰĿ¼������Ϊfile-name���ļ�\n");
    printf("print <file-name>                       ��ȡ����ӡ��ǰĿ¼������Ϊfile-name���ļ�����\n");
    printf("write <file-name> [offset] [mode]       �ڵ�ǰĿ¼������Ϊfile-name���ļ���,ѡ���offsetλ�ÿ�ʼд��\n");
    printf("                                        offset��ѡ,��������,��ʾƫ����\n");
    printf("                                        mode��ѡ,������ģʽ:0��ʾ���ļ�ͷ+offsetλ�ÿ�ʼд,1��ʾ���ļ�ָ��λ��+offset��ʼд,2��ʾ���ļ�β-offset��ʼд,Ĭ�ϴ�ͷ��ʼд\n");
    printf("                                        ��������д��ģʽ,����д������,��ESC����ʾ����\n");

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
    Inode *pInode = NULL;
    int i;
    for (i = 0; i < NUM_SUB_DIR; i++)
    {
        if (dir->d_inodenumber[i] == 0)
            continue;
        if (strcmp(dir->d_filename[i], subname.c_str()) == 0)
        {
            pInode = this->IGet(dir->d_inodenumber[i]);
            if (pInode->i_mode & Inode::INodeMode::IFILE)
                continue;
            else if (pInode->i_mode & Inode::INodeMode::IDIR)
                break;
        }
    }
    if (i == NUM_SUB_DIR)
    {
        cout << "Ŀ¼������!" << endl;
        return;
    }
    this->curDir += subname + "/";
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

/// @brief ������Ŀ¼
/// @param subname ��Ŀ¼����
void FileSystem::mkdirout(string subname)
{
    if (subname.find_first_of("/") != -1)
    {
        cout << "Ŀ¼�����ܰ���'/'!" << endl;
        return;
    }

    // ��ͨ�����������Ŀ¼
    int res = this->mkdir(this->curDir + subname);
}

void FileSystem::openFile(string path)
{
    int fd = this->fopen(path);
    if (fd == -1)
    {
        cout << "���ļ�ʧ��!" << endl;
        return;
    }
    else
    {
        this->openFileMap[this->GetAbsolutionPath(path)] = fd + 1;
        cout << "�ɹ����ļ�!" << endl;
    }
}

void FileSystem::closeFile(string path)
{
    int fd = this->openFileMap[this->GetAbsolutionPath(path)];
    if (fd == -1)
    {
        cout << "�ļ�δ��!" << endl;
        return;
    }
    else
    {
        this->fclose(&(this->openFileTable[fd]));
        this->openFileMap.erase(this->GetAbsolutionPath(path));
        cout << "�ɹ��ر��ļ�!" << endl;
    }
}

void FileSystem::createFile(string path)
{
    if (path.find_first_of("/") != -1)
    {
        cout << "�ļ������ܰ���'/'!" << endl;
        return;
    }
    if (path == "." || path == "..")
    {
        cout << "�ļ������淶!" << endl;
        return;
    }

    int res = this->fcreate(path);
}

void FileSystem::printFile(string path)
{
    int fd = this->openFileMap[this->GetAbsolutionPath(path)];
    if (fd == 0)
    {
        cout << "�ļ�δ��!����ʹ��openָ����ļ�" << endl;
        return;
    }

    File* fp = &(this->openFileTable[fd - 1]);
    char* buffer = NULL;
    int count = fp->f_inode->i_size;
    int oldoffset = fp->f_offset;//��¼�ɵ��ļ�ָ��λ��
    fp->f_offset = 0;
    this->fread(fp, buffer, count);
    fp->f_offset = oldoffset;
    if (buffer == NULL)
    {
        cout << "Ϊ��!" << endl;
        return;
    }
    cout << "�ļ�����Ϊ:" << endl;
    for (int i = 0; i < count; i++)
        cout << *(buffer + i) ;
    cout << endl << "�ļ�����!" << endl;
}

void FileSystem::writeFile(string path, int offset, int mode)
{
    int fd = this->openFileMap[this->GetAbsolutionPath(path)];
    if (fd == 0)
    {
        cout << "�ļ�δ��!����ʹ��openָ����ļ�" << endl;
        return;
    }

    File *fp = &(this->openFileTable[fd - 1]);
    if (this->Access(fp->f_inode, FileMode::WRITE) == 0)
    {
        cout << "�ļ�û��дȨ��!" << endl;
        return;
    }

    if (this->fseek(fp, offset, mode) == -1)
    {
        cout << "�ļ�ָ���ƶ�ʧ��!" << endl;
        return;
    }

    cout << "��ʼ�����ַ�(��ESC���˳�):" << endl;
    string input;
    int i = 0;
    while (true)
    {
        if (_kbhit())
        {                       // ����Ƿ��а�������
            char ch = _getch(); // ��ȡ�����ַ�����

            if (ch == 27)
            {          // ����Ƿ��� ESC ��
                break; // �˳�ѭ��
            }

            if (ch == '\r')
            {                  // ����Ƿ�����س���
                input += '\n'; // ���س���ת��Ϊ���з�����ӵ������ַ�����
                cout << endl;  // ������з�
            }
            else
            {
                input += ch; // ���ַ���ӵ������ַ�����
                cout << ch;  // ��ʾ��ǰ������ַ�
            }
            i++;
        }
    }
    cout << "���������ַ�������" << i << endl;

    this->fwrite(input.c_str(), input.size(), fp);
}

void FileSystem::login()
{
    string name, pswd;
    short id;
    while (true)
    {
        cout << "�������û���:";
        getline(cin, name);
        cout << "����������:";
        getline(cin, pswd);
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
    cout << "��½�ɹ�!" << endl
         << endl;
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

    cout << "����help���Բ鿴�����嵥" << endl
         << endl;
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
            if (input.size() < 1)
                continue;

            // Ŀ¼����
            if (input[0] == "ls") // �鿴��Ŀ¼
            {
                if (input.size() > 1)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->ls();
            }
            else if (input[0] == "cd") // ������Ŀ¼
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->cd(input[1]);
            }
            else if (input[0] == "rmdir") // ɾ����Ŀ¼
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->rmdir(input[1]);
            }
            else if (input[0] == "mkdir")
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->mkdirout(input[1]);
            }

            // �ļ�����
            else if (input[0] == "touch")
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->createFile(input[1]);
            }
            else if (input[0] == "open")
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->openFile(input[1]);
            }
            else if (input[0] == "close")
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->closeFile(input[1]);
            }
            else if (input[0] == "write")
            {
                if (input.size() < 2 || input.size() > 4)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                int offset = input.size() == 3 ? atoi(input[2].c_str()) : 0;
                int mode = input.size() == 4 ? atoi(input[3].c_str()) : 0;
                this->writeFile(input[1], offset, mode);
            }
            else if (input[0] == "print")
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->printFile(input[1]);
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
            else
            {
                cout << "ָ����ڣ���ͨ��help����֧�ֹ���" << endl;
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