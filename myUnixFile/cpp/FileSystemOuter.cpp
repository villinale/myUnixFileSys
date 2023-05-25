/*
 * @Author: yingxin wang
 * @Date: 2023-05-21 16:44:37
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-25 20:19:00
 * @Description: FileSystem类在main中可以调用的可交互的函数,尽量做到只输出
 */

#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

void FileSystem::help()
{
    // fformat\ls\mkdir\fcreat\fopen\fclose\fread\fwrite\flseek\fdelete
    printf("下列命令中带'<>'的项是必须的，带'[]'的项是可选择的\n");
    printf("请注意：本系统中路径用'/'分隔，windows系统路径用'\\'分割\n");
    printf("        VS中默认编码是GBK,想要正确输出文件内容，请保持编码一致\n");
    cout << "        \033[31m请勿随便关掉控制台,想要正确退出系统一定要输入exit\033[0m"; // 设置用红色字打印出来
    printf("--------------目录相关---------------\n");
    printf("ls                                      查看当前目录下的子目录\n");
    printf("cd    <dir-name>                        打开在当前目录下名称为dir-name的子目录\n");
    printf("mkdir <dir-name>                        创建在当前目录下名称为dir-name的子目录\n");
    printf("rmdir <dir-name>                        删除在当前目录下名称为dir-name的子目录\n");
    printf("--------------文件相关---------------\n");
    printf("touch <file-name>                       在当前目录下创建名称为file-name的文件\n");
    printf("open  <file-name>                       打开当前目录里名称为file-name的文件\n");
    printf("close <file-name>                       关闭当前目录里名称为file-name的文件\n");
    printf("print <file-name>                       读取并打印当前目录里名称为file-name的文件内容(需要先打开文件)\n");
    printf("write <file-name> [offset] [mode]       在当前目录里名称为file-name的文件里,选择从offset位置开始写入(需要先打开文件)\n");
    printf("                                        offset可选,输入整数,表示偏移量\n");
    printf("                                        mode可选,有三种模式:0表示从文件头+offset位置开始写,\n");
    printf("                                        1表示从文件指针位置+offset开始写,2表示从文件尾-offset开始写,默认从头开始写\n");
    printf("                                        输入后进入写入模式,输入写入内容,按ESC键表示结束\n");
    printf("cpfwin <win-path>                       将windows系统电脑上路径为win-path的文件复制到当前目录中\n");
    printf("cpffs  <file-name> <win-path>           将本系统上当前目录中名称为file-name的文件复制到电脑上路径为win-path的文件里(需要先打开文件)\n");
    printf("listopen                                打印已打开文件列表\n");
    printf("--------------用户相关---------------\n");
    printf("relogin                                 重新登录,会关闭所有的文件,完成之前所有的任务\n");
    printf("adduser                                 添加新用户,但是只能由root用户操作\n");
    printf("deluser                                 删除用户,但是只能由root用户操作\n");
    printf("----------------其他----------------\n");
    printf("format                                  格式化文件系统\n");
    printf("exit                                    退出系统\n");
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

        this->curDir.erase(this->curDir.find_last_of('/'));                        // 删除最后一个'/'
        this->curDir = this->curDir.substr(0, this->curDir.find_last_of('/') + 1); // 截取最后一个'/'
        Inode *p = this->curDirInode;
        // 回退父目录的Inode
        this->curDirInode = this->IGet(this->curDirInode->GetParentInumber());
        this->IPut(p); // 释放当前目录的Inode

        return;
    }
    else if (subname == ".") // 当前目录情况
    {
        return;
    }

    // 普通情况，进入子文件夹中
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
        cout << "目录不存在!" << endl;
        return;
    }
    this->curDir += subname + "/";
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

/// @brief 创建子目录
/// @param subname 子目录名称
void FileSystem::mkdirout(string subname)
{
    if (subname.find_first_of("/") != -1)
    {
        cout << "目录名不能包含'/'!" << endl;
        return;
    }

    // 普通情况，创建子目录
    int res = this->mkdir(this->curDir + subname);
}

void FileSystem::openFile(string path)
{
    int fd = this->fopen(path);
    if (fd == -1)
    {
        cout << "打开文件失败!" << endl;
        return;
    }
    else
    {
        this->openFileMap[this->GetAbsolutionPath(path)] = fd + 1;
        cout << "成功打开文件!" << endl;
    }
}

void FileSystem::closeFile(string path)
{
    int fd = this->openFileMap[this->GetAbsolutionPath(path)];
    if (fd == 0)
    {
        cout << "文件未打开!" << endl;
        return;
    }
    else
    {
        this->fclose(&(this->openFileTable[fd - 1]));
        this->openFileMap.erase(this->GetAbsolutionPath(path));
        cout << "成功关闭文件!" << endl;
    }
}

void FileSystem::createFile(string path)
{
    if (path.find_first_of("/") != -1)
    {
        cout << "文件名不能包含'/'!" << endl;
        return;
    }
    if (path == "." || path == "..")
    {
        cout << "文件名不规范!" << endl;
        return;
    }

    int res = this->fcreate(path);
}

void FileSystem::printFile(string path)
{
    int fd = this->openFileMap[this->GetAbsolutionPath(path)];
    if (fd == 0)
    {
        cout << "文件未打开!请先使用open指令打开文件" << endl;
        return;
    }

    File *fp = &(this->openFileTable[fd - 1]);
    char *buffer = NULL;
    int count = fp->f_inode->i_size;
    int oldoffset = fp->f_offset; // 记录旧的文件指针位置
    fp->f_offset = 0;
    this->fread(fp, buffer, count);
    fp->f_offset = oldoffset;
    if (buffer == NULL)
    {
        cout << "为空!" << endl;
        return;
    }
    cout << "文件内容为:" << endl;
    cout << "\033[31m" << buffer << "\033[0m"; // 设置用红色字打印出来
    cout << endl
         << "文件结束!" << endl;
}

void FileSystem::writeFile(string path, int offset, int mode)
{
    int fd = this->openFileMap[this->GetAbsolutionPath(path)];
    if (fd == 0)
    {
        cout << "文件未打开!请先使用open指令打开文件" << endl;
        return;
    }

    File *fp = &(this->openFileTable[fd - 1]);
    if (this->Access(fp->f_inode, FileMode::WRITE) == 0)
    {
        cout << "文件没有写权限!" << endl;
        return;
    }

    if (this->fseek(fp, offset, mode) == -1)
    {
        cout << "文件指针移动失败!" << endl;
        return;
    }

    cout << "开始输入字符(按ESC键退出):" << endl;
    string input;
    int i = 0;
    while (true)
    {
        if (_kbhit())
        {                       // 检查是否有按键按下
            char ch = _getch(); // 获取单个字符输入

            if (ch == 27)
            {          // 检查是否按下 ESC 键
                break; // 退出循环
            }

            if (ch == '\r')
            {                  // 检查是否输入回车符
                input += '\n'; // 将回车符转换为换行符并添加到输入字符串中
                cout << endl;  // 输出换行符
            }
            else
            {
                input += ch; // 将字符添加到输入字符串中
                cout << ch;  // 显示当前输入的字符
            }
            i++;
        }
    }
    cout << endl
         << "本次输入字符个数：" << i << endl;

    this->fwrite(input.c_str(), input.size(), fp);
}

void FileSystem::cpfwin(string path)
{
    fstream fd;
    fd.open(path, ios::in | ios::binary);
    if (!fd.is_open())
    {
        cout << "无法打开文件" << path << endl;
        return;
    }
    fd.seekg(0, fd.end);
    int filesize = fd.tellg(); // 获取文件大小
    fd.seekg(0, fd.beg);
    char *buffer = new char[filesize];
    fd.read(buffer, filesize); // 读取文件内容
    fd.close();

    vector<string> paths = stringSplit(path, '\\'); // win系统上的路径分割符为'\'
    string filename = paths[paths.size() - 1];      // 获取文件名

    // 创建文件
    int res = this->fcreate(filename);
    if (res == 0)
    {
        int fileloc = this->fopen(filename);
        File *filep = &(this->openFileTable[fileloc]);
        this->fwrite(buffer, filesize, filep);
        this->fclose(filep);
        cout << "成功导入文件" << filename << ",写入大小为" << filesize << endl;
    }
}

void FileSystem::cpffs(string filename, string winpath)
{
    fstream fd;
    fd.open(winpath, ios::out);
    if (!fd.is_open())
    {
        cout << "无法打开文件" << winpath << endl;
        return;
    }

    // 打开fs中的文件
    int fileloc = this->fopen(filename);
    File *fp = &(this->openFileTable[fileloc]);
    char *buffer = NULL;
    int count = fp->f_inode->i_size;
    int oldoffset = fp->f_offset; // 记录旧的文件指针位置
    fp->f_offset = 0;
    this->fread(fp, buffer, count);
    fp->f_offset = oldoffset;
    this->fclose(fp);

    if (buffer == NULL)
    {
        cout << "为空!" << endl;
        return;
    }
    fd.write(buffer, count); // 写入文件内容
    fd.close();
    cout << "成功导出文件" << filename << ",写入大小为" << count << endl;
}

void FileSystem::prin0penFileList()
{
    cout << "当前打开文件列表:" << endl;
    if (this->openFileMap.empty())
    {
        cout << "无打开文件!" << endl;
        return;
    }
    cout << "文件名路径\t\t文件描述符\t\t文件指针" << endl;
    for (const auto &pair : this->openFileMap)
        cout << pair.first << "\t\t" << pair.second << "\t\t" << this->openFileTable[pair.second - 1].f_offset << endl;
    cout << endl;
}

void FileSystem::login()
{
    string name, pswd;
    short id;
    while (true)
    {
        cout << "请输入用户名:";
        getline(cin, name);
        cout << "请输入密码:";
        getline(cin, pswd);
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
    cout << "登陆成功!" << endl
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
             << "请输入用户名:";
        getline(cin, name);
        cout << "请输入密码:";
        getline(cin, pswd);
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
    cout << "登陆成功!" << endl
         << endl;
    this->curId = id;
    this->curName = name;
}

void FileSystem::adduser()
{
    string name, pswd, temp;
    short uid, id;
    cout << "请输入用户名:";
    getline(cin, name);
    cout << "请输入密码:";
    getline(cin, pswd);
    cout << "请输入组id(root组id为0,unix组id为1):";
    getline(cin, temp);
    if (name.empty() || pswd.empty() || temp.empty())
    {
        cout << "输入非法!" << endl;
        return;
    }
    uid = atoi(temp.c_str());
    this->userTable->AddUser(this->curId, name.c_str(), pswd.c_str(), uid);
    return;
}

void FileSystem::deluser()
{
    string name;
    cout << "请输入想要删除的用户名:";
    getline(cin, name);
    if (name.empty())
    {
        cout << "输入非法!" << endl;
        return;
    }
    this->userTable->DeleteUser(this->curId, name.c_str());
    return;
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

    cout << "输入help可以查看命令清单" << endl
         << endl;
    vector<string> input;
    string strIn;
    while (true)
    {
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

            // 目录管理
            if (input[0] == "ls") // 查看子目录
            {
                if (input.size() > 1)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->ls();
            }
            else if (input[0] == "cd") // 进入子目录
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->cd(input[1]);
            }
            else if (input[0] == "rmdir") // 删除子目录
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->rmdir(input[1]);
            }
            else if (input[0] == "mkdir")
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->mkdirout(input[1]);
            }

            // 文件管理
            else if (input[0] == "touch")
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->createFile(input[1]);
            }
            else if (input[0] == "open")
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->openFile(input[1]);
            }
            else if (input[0] == "close")
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->closeFile(input[1]);
            }
            else if (input[0] == "write")
            {
                if (input.size() < 2 || input.size() > 4)
                {
                    cout << "输入非法!" << endl;
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
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->printFile(input[1]);
            }
            else if (input[0] == "cpfwin")
            {
                if (input.size() < 2 || input.size() > 2)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->cpfwin(input[1]);
            }
            else if (input[0] == "cpffs")
            {
                if (input.size() < 3 || input.size() > 3)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->cpffs(input[1], input[2]);
            }
            else if (input[0] == "listopen")
            {
                if (input.size() < 1 || input.size() > 1)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->prin0penFileList();
            }

            else if (input[0] == "relogin")
            {
                if (input.size() < 1 || input.size() > 1)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->relogin();
            }
            else if (input[0] == "adduser")
            {
                if (input.size() < 1 || input.size() > 1)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->adduser();
            }
            else if (input[0] == "deluser")
            {
                if (input.size() < 1 || input.size() > 1)
                {
                    cout << "输入非法!" << endl;
                    continue;
                }
                this->deluser();
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
                cout << "指令不存在，请通过help查阅支持功能" << endl;
            }
        }
        catch (int &e)
        {
            cout << "error code：" << e << endl;
            cout << "与linux错误码保持一致" << endl
                 << endl;
        }
    }
}