/*
 * @Author: yingxin wang
 * @Date: 2023-05-21 16:44:37
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-21 21:45:15
 * @Description: FileSystem类在main中可以调用的可交互的函数,尽量做到只输出
 */

#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

void FileSystem::help()
{
    // fformat\ls\mkdir\fcreat\fopen\fclose\fread\fwrite\flseek\fdelete
    cout << "命令        说明" << endl;
    cout << "cd          格式化文件系统" << endl;
    cout << "fformat     格式化文件系统" << endl;
    printf("fformat                                     - 格式化文件系统\n");
    printf("mkdir      <dir name>                       - 创建目录\n");
    printf("cd         <dir name>                       - 进入目录\n");
    printf("ls                                          - 显示当前目录清单\n");
    printf("rmdir      <dir name>                       - 删除目录\n");
    printf("touch      <file name>                      - 创建新文件\n");
    printf("chmod      <file name/dir name> <mode(OTC)> - 修改文件或目录权限\n");
    printf("rm         <file name>                      - 删除文件\n");
    printf("login      <user name>                      - 用户登录\n");
    printf("logout                                      - 用户注销\n");
    printf("useradd    <user name> <group name>         - 添加用户\n");
    printf("userdel    <user name>                      - 删除用户\n");
    printf("groupadd   <group name>                     - 添加用户组\n");
    printf("groupdel   <group name>                     - 删除用户组\n");
    printf("df                                          - 查看磁盘使用情况\n");
    printf("show       <file name>                      - 打印文件内容\n");
    printf("vi         <file name>                      - 用编辑器打开文件\n");
    printf("win2fs	   <win file name> <fs file name>   - 将Windows文件内容复制到FS文件系统文件\n");
    printf("fs2win	   <fs file name> <win file name>   - 将FS文件系统文件内容复制到Windows文件\n");
    printf("help                                        - 显示命令清单\n");
    printf("cls                                         - 清屏\n");
    printf("exit                                        - 退出系统\n");
}

/// @brief 列目录
void FileSystem::ls()
{
}

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
        cout << "目录不存在！" << endl;
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

    // 对缓存相关内容进行初始化
    this->bufManager = new BufferManager();

    // 读取超级块
    Buf *buf = this->bufManager->Bread(POSITION_SUPERBLOCK);
    this->spb = char2SuperBlock(buf->b_addr);

    // 读取根目录Inode
    buf = this->bufManager->Bread(POSITION_DISKINODE);
    this->rootDirInode = new Inode();
    this->rootDirInode->ICopy(buf, ROOT_DIR_INUMBER);
    this->curDirInode = this->rootDirInode;
    this->curDir = "/";

    // 读取用户信息表
    File *userTableFile = this->fopen("/etc/userTable.txt");
    char *buffer;
    this->fread(userTableFile, buffer, userTableFile->f_inode->i_size);
    this->userTable = char2UserTable(buffer);
    this->fclose(userTableFile);
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
            cout << "用户不存在！" << endl;
            continue;
        }
        else
            break;
    }
    cout << "登陆成功！" << endl;
    this->curId = id;
    this->curName = name;
    return name;
}

void FileSystem::fun()
{
    vector<string> input;
    string strIn;
    while (true)
    {
        cout << this->curName << "\\" << this->curDir << ">";
        getline(cin, strIn);
        input = stringSplit(strIn, ' ');
        if (input.size() == 0)
            continue;

        // if ()
    }
}