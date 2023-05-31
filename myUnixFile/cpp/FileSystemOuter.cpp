/*
 * @Author: yingxin wang
 * @Date: 2023-05-21 16:44:37
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-31 19:44:33
 * @Description: FileSystem����main�п��Ե��õĿɽ����ĺ���,��������ֻ���
 */

#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

void FileSystem::help()
{
    printf("���������д�'<>'�����Ǳ���ģ���'[]'�����ǿ�ѡ���\n");
    printf("��ע�⣺��ϵͳ��·����'/'�ָ���windowsϵͳ·����'\\'�ָ�\n");
    printf("        VS��Ĭ�ϱ�����GBK,��Ҫ��ȷ����ļ����ݣ��뱣�ֱ���һ��\n");
    cout << "        \033[31m�������ص�����̨,��Ҫ��ȷ�˳�ϵͳһ��Ҫ����exit\033[0m\n"; // �����ú�ɫ�ִ�ӡ����
    printf("--------------Ŀ¼���---------------\n");
    printf("ls                                      �鿴��ǰĿ¼�µ���Ŀ¼\n");
    printf("dir                                     �鿴��ǰĿ¼�µ���ϸ��Ϣ\n");
    printf("cd    <dir-name>                        ���ڵ�ǰĿ¼������Ϊdir-name����Ŀ¼\n");
    printf("mkdir <dir-name>                        �����ڵ�ǰĿ¼������Ϊdir-name����Ŀ¼\n");
    printf("rmdir <dir-name>                        ɾ���ڵ�ǰĿ¼������Ϊdir-name����Ŀ¼\n");
    printf("--------------�ļ����---------------\n");
    printf("touch <file-name>                       �ڵ�ǰĿ¼�´�������Ϊfile-name���ļ�\n");
    printf("rm    <file-name>                       ɾ����ǰĿ¼������Ϊfile-name���ļ�\n");
    printf("open  <file-name>                       �򿪵�ǰĿ¼������Ϊfile-name���ļ�\n");
    printf("chmod <file-name> <mode>                �޸ĵ�ǰĿ¼������Ϊfile-name���ļ���Ȩ��Ϊmode\n");
    printf("                                        mode��ʽ:rwrwrw,r����ɶ�,w�����д,-����û�����Ȩ��\n");
    printf("                                                ����ֱ�����ļ�������Ȩ�ޡ�ͬ���û�Ȩ�޺������û�Ȩ��\n");
    printf("                                                eg. rwr-r-�����ļ�������Ȩ�޿ɶ�д��ͬ���û�Ȩ�޿ɶ��������û�Ȩ�޿ɶ�\n");
    printf("close <file-name>                       �رյ�ǰĿ¼������Ϊfile-name���ļ�\n");
    printf("print <file-name>                       ��ȡ����ӡ��ǰĿ¼������Ϊfile-name���ļ�����(��Ҫ�ȴ��ļ�)\n");
    printf("fseek <file-name> <offset>              �ƶ��ļ�ָ��offset��ƫ����������Ϊ��\n");
    printf("write <file-name> [mode]                �ڵ�ǰĿ¼������Ϊfile-name���ļ��￪ʼд��(��Ҫ�ȴ��ļ�)\n");
    printf("                                        mode��ѡ,������ģʽ:0��ʾ���ļ�ͷλ�ÿ�ʼд,\n");
    printf("                                        1��ʾ���ļ�ָ��λ�ÿ�ʼд,2��ʾ���ļ�β��ʼд,Ĭ��ģʽΪ0\n");
    printf("                                        ��������д��ģʽ,����д������,��ESC����ʾ����\n");
    printf("cpfwin <win-path>                       ��windowsϵͳ������·��Ϊwin-path���ļ����Ƶ���ǰĿ¼��\n");
    printf("cpffs  <file-name> <win-path>           ����ϵͳ�ϵ�ǰĿ¼������Ϊfile-name���ļ����Ƶ�������·��Ϊwin-path���ļ���(��Ҫ�ȴ��ļ�)\n");
    printf("listopen                                ��ӡ�Ѵ��ļ��б�\n");
    printf("--------------�û����---------------\n");
    printf("relogin                                 ���µ�¼,��ر����е��ļ�,���֮ǰ���е�����\n");
    printf("adduser                                 ������û�,����ֻ����root�û�����\n");
    printf("deluser                                 ɾ���û�,����ֻ����root�û�����\n");
    printf("chgroup                                 �ı��û��û���,����ֻ����root�û�����\n");
    printf("listuser                                ��ӡ�����û���Ϣ\n");
    printf("----------------����----------------\n");
    printf("format                                  ��ʽ���ļ�ϵͳ\n");
    printf("exit                                    �˳�ϵͳ\n");
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

    // ��ȡ�����飬����������������
    char *ch = new char[sizeof(SuperBlock)];
    Buf *buf = this->bufManager->Bread(POSITION_SUPERBLOCK);
    memcpy(ch, buf->b_addr, SIZE_BLOCK);
    buf = this->bufManager->Bread(POSITION_SUPERBLOCK + 1);
    memcpy(ch + SIZE_BLOCK, buf->b_addr, SIZE_BLOCK);
    this->spb = char2SuperBlock(ch); // ����ɾ��ch

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

        this->curDir.erase(this->curDir.find_last_of('/'));                        // ɾ�����һ��'/'
        this->curDir = this->curDir.substr(0, this->curDir.find_last_of('/') + 1); // ��ȡ���һ��'/'
        Inode *p = this->curDirInode;
        // ���˸�Ŀ¼��Inode
        this->curDirInode = this->IGet(this->curDirInode->GetParentInumber());
        this->IPut(p); // �ͷŵ�ǰĿ¼��Inode

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

void FileSystem::dir()
{
    cout << "������" << this->curDir << "Ŀ¼�µ��ļ�:" << endl;
    Directory *dir = this->curDirInode->GetDir();
    int i;
    cout << std::left << setw(12) << "����Ȩ��"
         << std::left << setw(12) << "��ǰȨ��"
         << std::left << setw(20) << "�޸�ʱ��"
         << std::left << setw(10) << "�ļ�����"
         << std::left << setw(15) << "�ļ���С"
         << std::left << "�ļ���" << endl;
    for (i = 0; i < NUM_SUB_DIR; i++)
    {
        if (dir->d_inodenumber[i] == 0)
            break;
        Inode *p = this->IGet(dir->d_inodenumber[i]);
        string time = timestampToString(p->i_mtime);
        if (p->i_mode & Inode::INodeMode::IFILE)
            cout << std::left << setw(12) << mode2String(p->i_mode)
                 << std::left << setw(12) << p->GetModeString(this->curId, this->userTable->GetGId(this->curId))
                 << std::left << setw(20) << time
                 << std::left << setw(10) << " "
                 << std::left << setw(15) << p->i_size
                 << std::left << dir->d_filename[i] << endl;
        else if (p->i_mode & Inode::INodeMode::IDIR)
            cout << std::left << setw(12) << mode2String(p->i_mode)
                 << std::left << setw(12) << p->GetModeString(this->curId, this->userTable->GetGId(this->curId))
                 << std::left << setw(20) << time
                 << std::left << setw(10) << "<DIR>"
                 << std::left << setw(15) << " "
                 << std::left << dir->d_filename[i] << endl;
        this->IPut(p);
    }
    cout << endl;
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
    if (fd == 0)
    {
        cout << "�ļ�δ��!" << endl;
        return;
    }
    else
    {
        this->fclose(&(this->openFileTable[fd - 1]));
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

void FileSystem::removefile(string path)
{
    int fd = this->openFileMap[this->GetAbsolutionPath(path)];
    if (fd != 0)
    {
        cout << "�ļ��Ѵ�!���ȹر��ļ�" << endl;
        return;
    }
    int res = this->fdelete(this->curDir + path);
}

void FileSystem::printFile(string path)
{
    int fd = this->openFileMap[this->GetAbsolutionPath(path)];
    if (fd == 0)
    {
        cout << "�ļ�δ��!����ʹ��openָ����ļ�" << endl;
        return;
    }

    File *fp = &(this->openFileTable[fd - 1]);
    char *buffer = NULL;
    int count = fp->f_inode->i_size;
    int oldoffset = fp->f_offset; // ��¼�ɵ��ļ�ָ��λ��
    fp->f_offset = 0;
    this->fread(fp, buffer, count);
    fp->f_offset = oldoffset;
    if (buffer == NULL)
    {
        cout << "Ϊ��!" << endl;
        return;
    }
    cout << "�ļ�����Ϊ:" << endl;
    cout << "\033[31m" << buffer << "\033[0m"; // �����ú�ɫ�ִ�ӡ����
    cout << endl
         << "�ļ�����!" << endl;
}

void FileSystem::writeFile(string path, int mode)
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

    if (this->fseek(fp, 0, mode) == -1)
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
    cout << endl
         << "���������ַ�������" << i << endl;

    this->fwrite(input.c_str(), input.size(), fp);
}

void FileSystem::cpfwin(string path)
{
    fstream fd;
    fd.open(path, ios::in | ios::binary);
    if (!fd.is_open())
    {
        cout << "�޷����ļ�" << path << endl;
        return;
    }
    fd.seekg(0, fd.end);
    int filesize = fd.tellg(); // ��ȡ�ļ���С
    fd.seekg(0, fd.beg);
    char* buffer = new char[filesize + 1];
    fd.read(buffer, filesize); // ��ȡ�ļ�����
    buffer[filesize] = '\0';
    cout << buffer << endl;
    fd.close();

    vector<string> paths = stringSplit(path, '\\'); // winϵͳ�ϵ�·���ָ��Ϊ'\'
    string filename = paths[paths.size() - 1];      // ��ȡ�ļ���

    // �����ļ�
    int res = this->fcreate(filename);
    if (res == 0)
    {
        int fileloc = this->fopen(filename);
        File *filep = &(this->openFileTable[fileloc]);
        this->fwrite(buffer, filesize, filep);
        this->fclose(filep);
        cout << "�ɹ������ļ�" << filename << ",д���СΪ" << filesize << endl;
    }
}

void FileSystem::cpffs(string filename, string winpath)
{
    fstream fd;
    fd.open(winpath, ios::out);
    if (!fd.is_open())
    {
        cout << "�޷����ļ�" << winpath << endl;
        return;
    }

    // ��fs�е��ļ�
    int fileloc = this->fopen(filename);
    File *fp = &(this->openFileTable[fileloc]);
    char *buffer = NULL;
    int count = fp->f_inode->i_size;
    int oldoffset = fp->f_offset; // ��¼�ɵ��ļ�ָ��λ��
    fp->f_offset = 0;
    this->fread(fp, buffer, count);
    fp->f_offset = oldoffset;
    this->fclose(fp);

    if (buffer == NULL)
    {
        cout << "Ϊ��!" << endl;
        return;
    }
    fd.write(buffer, count); // д���ļ�����
    fd.close();
    cout << "�ɹ������ļ�" << filename << ",д���СΪ" << count << endl;
}

void FileSystem::prin0penFileList()
{
    cout << "��ǰ���ļ��б�:" << endl;
    if (this->openFileMap.empty())
    {
        cout << "�޴��ļ�!" << endl;
        return;
    }
    cout << std::left << setw(20) << "�ļ���·��" << setw(10) << "�ļ�������" << setw(10) << "�ļ�ָ��" << endl;
    for (const auto &pair : this->openFileMap)
        cout << std::left << setw(20) << pair.first << setw(10) << pair.second << setw(10) << this->openFileTable[pair.second - 1].f_offset << endl;
    cout << endl;
}

void FileSystem::chmod(string path, string mode)
{
    if (this->curId != 0)
    {
        cout << "ֻ��root�û������޸��ļ�Ȩ��!" << endl;
        return;
    }
    if (mode.size() != 6)
    {
        cout << "�����Ȩ�޸�ʽ����ȷ!" << endl;
        return;
    }

    Inode *p = this->NameI(path);
    if (p == NULL)
    {
        cout << "�ļ�������!" << endl;
        return;
    }

    unsigned short modeNum = p->String2Mode(mode);
    if (modeNum == -1)
    {
        cout << "�����Ȩ�޸�ʽ����ȷ!" << endl;
        return;
    }

    int res = p->AssignMode(modeNum);
    if (res == 0)
        cout << "�޸ĳɹ�!" << endl;
    else
        cout << "���ܸı��ļ�����!" << endl;
}

void FileSystem::changeseek(string path, int offset)
{
    int fd = this->openFileMap[this->GetAbsolutionPath(path)];
    if (fd == 0)
    {
        cout << "�ļ�δ��!����ʹ��openָ����ļ�" << endl;
        return;
    }

    Inode *p = this->NameI(this->GetAbsolutionPath(path));
    if (p == NULL)
    {
        cout << "�ļ�������!" << endl;
        return;
    }
    File *fp = &(this->openFileTable[fd - 1]);
    if ((fp->f_offset + offset) < 0 || (fp->f_offset + offset) > p->i_size)
    {
        cout << "�ļ�ָ�볬����Χ!" << endl;
        return;
    }
    fp->f_offset += offset;
    cout << "�ļ�ָ�����ƶ���" << fp->f_offset << endl;
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

void FileSystem::relogin()
{
    this->exit();
    this->init();
    string name, pswd;
    short id;
    while (true)
    {
        cout << endl
             << "�������û���:";
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

void FileSystem::adduser()
{
    string name, pswd, temp;
    short uid, id;
    cout << "�������û���:";
    getline(cin, name);
    cout << "����������:";
    getline(cin, pswd);
    cout << "��������id(root��idΪ0,unix��idΪ1):";
    getline(cin, temp);
    if (name.empty() || pswd.empty() || temp.empty())
    {
        cout << "����Ƿ�!" << endl;
        return;
    }
    uid = atoi(temp.c_str());
    this->userTable->AddUser(this->curId, name.c_str(), pswd.c_str(), uid);
    return;
}

void FileSystem::chgroup()
{
    if (this->curId != 0)
    {
        cout << "ֻ��root�û������޸��û���!" << endl;
        return;
    }

    string name, temp;
    short uid, id;
    cout << "�������û���:";
    getline(cin, name);
    cout << "��������id(root��idΪ0,unix��idΪ1):";
    getline(cin, temp);
    if (name.empty() || temp.empty())
    {
        cout << "����Ƿ�!" << endl;
        return;
    }
    uid = atoi(temp.c_str());
    this->userTable->ChangerUserGID(this->curId, name.c_str(), uid);
    return;
}

void FileSystem::deluser()
{
    string name;
    cout << "��������Ҫɾ�����û���:";
    getline(cin, name);
    if (name.empty())
    {
        cout << "����Ƿ�!" << endl;
        return;
    }
    this->userTable->DeleteUser(this->curId, name.c_str());
    return;
}

void FileSystem::printUserList()
{
    cout << "�û���Ϣ�б�:" << endl;
    cout << std::left << setw(10) << "�û�id" << setw(25) << "�û���" << setw(10) << "�û���id" << endl;
    for (int i = 0; i < NUM_USER; i++)
        if (this->userTable->u_id[i] != -1)
            cout << std::left << setw(10) << this->userTable->u_id[i] << setw(25) << this->userTable->u_name[i] << setw(5) << this->userTable->u_gid[i] << endl;
    cout << endl;
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
        input.clear();
        cout << endl
             << this->curName << this->curDir << ">";
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
            else if (input[0] == "dir")
            {
                if (input.size() < 1 || input.size() > 1)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->dir();
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
            else if (input[0] == "rm")
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->removefile(input[1]);
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
                if (input.size() < 2 || input.size() > 3)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                int mode = input.size() == 3 ? atoi(input[2].c_str()) : 0;
                this->writeFile(input[1], mode);
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
            else if (input[0] == "cpffs")
            {
                if (input.size() < 3 || input.size() > 3)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->cpffs(input[1], input[2]);
            }
            else if (input[0] == "cpfwin")
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->cpfwin(input[1]);
            }
            else if (input[0] == "chmod")
            {
                if (input.size() < 3 || input.size() > 3)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->chmod(input[1], input[2]);
            }
            else if (input[0] == "listopen")
            {
                if (input.size() < 1 || input.size() > 1)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->prin0penFileList();
            }
            else if (input[0] == "fseek")
            {
                if (input.size() < 3 || input.size() > 3)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->changeseek(input[1], stoi(input[2]));
            }

            // �û����
            else if (input[0] == "relogin")
            {
                if (input.size() < 1 || input.size() > 1)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->relogin();
            }
            else if (input[0] == "chgroup")
            {
                if (input.size() < 1 || input.size() > 1)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->chgroup();
            }
            else if (input[0] == "adduser")
            {
                if (input.size() < 1 || input.size() > 1)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->adduser();
            }
            else if (input[0] == "deluser")
            {
                if (input.size() < 1 || input.size() > 1)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->deluser();
            }
            else if (input[0] == "listuser")
            {
                if (input.size() < 1 || input.size() > 1)
                {
                    cout << "����Ƿ�!" << endl;
                    continue;
                }
                this->printUserList();
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
            cout << "error code��" << e << endl;
            cout << "��linux�����뱣��һ��" << endl
                 << endl;
        }
    }
}