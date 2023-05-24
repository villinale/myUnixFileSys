/*
 * @Author: yingxin wang
 * @Date: 2023-05-21 16:44:37
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-24 15:03:03
 * @Description: FileSystem类在main中可以调用的可交互的函数,尽量做到只输出
 */

#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

void FileSystem::help()
{
    // fformat\ls\mkdir\fcreat\fopen\fclose\fread\fwrite\flseek\fdelete
    printf("--------------目录相关---------------\n");
    printf("ls                     查看子目录\n");
    printf("cd <dir-name>          打开名字为dir-name的子目录\n");
    printf("rmdir <dir-name>       删除名字为dir-name的子目录\n");
    printf("--------------文件相关---------------\n");
    printf("open <file-path>       打开路径为file-path的文件\n");
    printf("                       支持以/开头的从根目录打开 与 不以/开头的从当前目录打开\n");
    printf("close <file-path>      关闭路径为file-path的文件\n");
    printf("open <file-path>       打开路径为file-path的文件\n");

    printf("----------------其他----------------\n");
    printf("fformat             格式化文件系统\n");
}

/// @brief 初始化系统，用于已有磁盘文件的情况
void FileSystem::init()
{
    fstream fd(DISK_PATH, ios::out | ios::in | ios::binary);
    // 如果没有打开文件则输出提示信息并throw错误
    if (!fd.is_open())
    {
        cout << "无法打开一级文件myDisk.img" << endl;
        throw(errno);
    }
    fd.close();

    // 对缓存相关内容进行初始化
    this->bufManager = new BufferManager();

    // 读取超级块
    Buf *buf = this->bufManager->Bread(POSITION_SUPERBLOCK);
    this->spb = char2SuperBlock(buf->b_addr);

    // 读取根目录Inode
    buf = this->bufManager->Bread(POSITION_DISKINODE);
    this->rootDirInode = this->IAlloc();
    this->rootDirInode->ICopy(buf, ROOT_DIR_INUMBER);
    this->curDirInode = this->rootDirInode;
    this->curId = ROOT_ID; // 先这样初始化
    this->curDir = "/";

    // 读取用户信息表
    // 不能直接调用this->fopen，因为userTable本身还没有初始化
    Inode *pinode = this->NameI("/etc/userTable.txt");
    // 没有找到相应的Inode
    if (pinode == NULL)
    {
        cout << "没有找到/etc/userTable.txt!" << endl;
        throw(ENOENT);
        return;
    }
    // 如果找到，判断所要找的文件是不是文件类型
    if (!(pinode->i_mode & Inode::INodeMode::IFILE))
    {
        cout << "不是一个正确的/etc/userTable.txt文件!" << endl;
        throw(ENOTDIR);
        return;
    }
    Buf *bp = this->bufManager->Bread(pinode->Bmap(0)); // userTable.txt文件本身只占一个盘块大小
    this->userTable = char2UserTable(bp->b_addr);
}

/// @brief 列目录
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

/// @brief 打开子目录
/// @param subname 子目录名称
void FileSystem::cd(string subname)
{
    // 回退到父目录的情况
    if (subname == "..")
    {
        if (this->curDir == "/") // 根目录情况
            return;

        int id = this->curDir.find_last_of('/');
        if (id == 0) // 说明这是根目录下的子目录
            this->curDir = "/";
        else
            this->curDir = this->curDir.substr(0, this->curDir.find_last_of('/'));

        this->IPut(this->curDirInode); // 释放当前目录的Inode
        // 回退父目录的Inode
        this->curDirInode = this->IGet(this->curDirInode->GetParentInumber());

        return;
    }
    else if (subname == ".") // 当前目录情况
    {
        return;
    }

    // 普通情况，进入子文件夹中
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
        cout << "目录不存在!" << endl;
        return;
    }
    if (this->curDirInode == this->rootDirInode) // root目录只增加子目录名
        this->curDir += subname;
    else
        this->curDir += "/" + subname;
    this->curDirInode = this->IGet(dir->d_inodenumber[i]);
}

/// @brief 删除子目录
/// @param subname 子目录名称
void FileSystem::rmdir(string subname)
{
    if (subname == "." || subname == "..")
    {
        cout << "不能删除当前目录或父目录!" << endl;
        return;
    }

    // 普通情况，删除子文件夹
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
        cout << "所要删除的目录不存在!" << endl;
        return;
    }

    // 获取所要删除的文件夹的Inode
    Inode *pDeleteInode = this->IGet(dir->d_inodenumber[i]);
    if (NULL == pDeleteInode) // 这样的情况应该不存在，但是写一下
    {
        cout << "所要删除的目录不存在!" << endl;
        return;
    }
    if (pDeleteInode->i_mode & Inode::INodeMode::IFILE) // 如果是文件类型
    {
        cout << "请输入正确的子目录名!" << endl;
        return;
    }
    Directory *deletedir = pDeleteInode->GetDir();
    for (int i = 2; i < NUM_SUB_DIR; i++) // 从第3个目录项开始，所有目录项的前两个都是自己和父亲
    {
        if (deletedir->d_inodenumber[i] != 0)
        {
            cout << "目录非空，不能删除!" << endl;
            return;
        }
    }

    // 删除子目录inode,其实只是unlink
    pDeleteInode->i_nlink--;
    this->IPut(pDeleteInode); // 当i_nlink为0时，会释放Inode

    // 删除父目录下的子目录项
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
        cout << "请输入用户名:";
        cin >> name;
        cout << "请输入密码:";
        cin >> pswd;
        if (name.empty() || pswd.empty())
        {
            cout << "输入非法!" << endl;
            continue;
        }
        id = this->userTable->FindUser(name.c_str(), pswd.c_str());
        if (id == -1)
        {
            cout << "用户不存在!" << endl;
            continue;
        }
        else
            break;
    }
    cout << "登陆成功!" << endl;
    this->curId = id;
    this->curName = name;
}

void FileSystem::format()
{
    cout << this->curName << this->curDir << ">"
         << "确定要进行格式化?[y] ";
    string strIn;
    getline(cin, strIn);
    cout << "正在格式化" << endl
         << endl;
    if (strIn == "y" || strIn == "Y")
    {
        this->fformat();
    }
    cout << "格式化结束" << endl
         << endl;
}

void FileSystem::fun()
{
    this->login();

    cout << "输入help可以查看命令清单" << endl;
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
            // 目录管理
            if (input[0] == "ls") // 查看子目录
                this->ls();
            else if (input[0] == "cd") // 进入子目录
                this->cd(input[1]);
            else if (input[0] == "rmdir") // 删除子目录
                this->rmdir(input[1]);
            // 文件管理
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
            cout << "与linux错误码保持一致" << endl
                 << endl;
        }
    }
}