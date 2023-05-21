/*
 * @Author: yingxin wang
 * @Date: 2023-05-21 16:44:37
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-22 00:13:30
 * @Description: FileSystem类在main中可以调用的可交互的函数,尽量做到只输出
 */

#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

void FileSystem::help()
{
    // fformat\ls\mkdir\fcreat\fopen\fclose\fread\fwrite\flseek\fdelete
    cout << "命令        说明" << endl;
    printf("cd          格式化文件系统\n");
    printf("fformat     格式化文件系统\n");
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

        this->curDir = this->curDir.substr(0, this->curDir.find_last_of('/'));

        this->IPut(this->curDirInode); // 释放当前目录的Inode
        // 回退父目录的Inode
        this->curDirInode = this->IGet(this->curDirInode->GetParentInumber());
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
    this->curDirInode = this->IGet(dir->d_inodenumber[i]);
    this->curDir += "/" + subname;
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

void FileSystem::fun()
{
    cout << "输入help可以查看命令清单" << endl;
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