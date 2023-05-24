/*
 * @Author: yingxin wang
 * @Date: 2023-05-12 08:12:28
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-24 19:35:59
 * @Description: FileSystem�������ĸ��ຯ������Outter�ļ��еĺ�������
 */
#include "../h/header.h"
#include "../h/errno.h"
#include "../h/Utility.h"

/// @brief �����ļ�
/// @param path �ļ�·��
/// @return int �����ɹ�Ϊ0������Ϊ-1
int FileSystem::fcreate(string path)
{
	vector<string> paths = stringSplit(path, '/');
	if (paths.size() == 0)
	{
		cout << "·����Ч!" << endl;
		throw(EINVAL);
		return -1;
	}
	string name = paths[paths.size() - 1];
	if (name.size() > NUM_FILE_NAME)
	{
		cout << "�ļ�������!" << endl;
		throw(ENAMETOOLONG);
		return -1;
	}
	else if (name.size() == 0)
	{
		cout << "�ļ�������Ϊ��!" << endl;
		throw(EINVAL);
		return -1;
	}

	Inode *fatherInode;

	// ��·����ɾ���ļ���
	path.erase(path.size() - name.size(), name.size());
	// �ҵ���Ҫ�������ļ��ĸ��ļ�����Ӧ��Inode
	fatherInode = this->NameI(path);

	// û���ҵ���Ӧ��Inode
	if (fatherInode == NULL)
	{
		cout << "û���ҵ���Ӧ���ļ���Ŀ¼!" << endl;
		throw(ENOENT);
		return -1;
	}

	// ����ҵ����жϴ������ļ��ĸ��ļ����ǲ����ļ�������
	if (!(fatherInode->i_mode & Inode::INodeMode::IDIR))
	{
		cout << "����һ����ȷ��Ŀ¼��!" << endl;
		throw(ENOTDIR);
		return -1;
	}

	// ����ҵ����ж��Ƿ���Ȩ��д�ļ�
	if (this->Access(fatherInode, FileMode::WRITE) == 0)
	{
		cout << "û��Ȩ��д�ļ�!" << endl;
		throw(EACCES);
		return -1;
	}

	bool isFull = true;
	int iinDir = 0;
	// ����Ȩ��д�ļ�ʱ���ж��Ƿ��������ļ����Ҳ鿴�Ƿ��п��е���Ŀ¼����д
	// ����Ҫ���������̿��
	// ����Ŀ¼�ļ�ֻռһ���̿飬����ֻ��һ�Ϊ��
	int blkno = fatherInode->Bmap(0);
	// ��ȡ���̵�����
	Buf *fatherBuf = this->bufManager->Bread(blkno);
	// ������תΪĿ¼�ṹ
	Directory *fatherDir = char2Directory(fatherBuf->b_addr);
	// ѭ������Ŀ¼�е�ÿ��Ԫ��
	for (int i = 0; i < NUM_SUB_DIR; i++)
	{
		// ����ҵ���Ӧ��Ŀ¼
		if (name == fatherDir->d_filename[i])
		{
			cout << "�ļ��Ѵ���!" << endl;
			throw(EEXIST);
			return -1;
		}
		if (isFull && fatherDir->d_inodenumber[i] == 0)
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

	// ��ſ�ʼ�����µ��ļ�
	// ����һ���µ��ڴ�Inode
	Inode *newinode = this->IAlloc();
	newinode->i_mode = Inode::INodeMode::IFILE |
					   Inode::INodeMode::OWNER_R | Inode::INodeMode::OWNER_W | Inode::INodeMode::OWNER_X |
					   Inode::INodeMode::GROUP_R | Inode::INodeMode::GROUP_X |
					   Inode::INodeMode::OTHER_R | Inode::INodeMode::OTHER_X;
	newinode->i_nlink = 1;
	newinode->i_uid = this->curId;
	newinode->i_gid = this->userTable->GetGId(this->curId);
	newinode->i_size = 0;
	newinode->i_mtime = unsigned int(time(NULL));
	newinode->i_atime = unsigned int(time(NULL));

	// ���µ�Inodeд�ش�����
	newinode->WriteI();

	// ���ļ�д��Ŀ¼����
	fatherDir->mkdir(name.c_str(), newinode->i_number);
	// ����Ŀ¼д�ش����У���Ϊһ��Ŀ¼�����һ���̿��С��ֱ���޸���b_addr����ʵ������ȫ
	fatherBuf->b_addr = directory2Char(fatherDir);
	this->bufManager->Bwrite(fatherBuf);
	// this->bufManager->bwrite(directory2Char(fatherDir), POSITION_BLOCK + fatherBuf->b_blkno, sizeof(fatherDir));

	// �ͷ�����Inode
	if (fatherInode != this->rootDirInode && fatherInode != this->curDirInode)
		this->IPut(fatherInode);
	this->IPut(newinode);

	return 0;
}

/// @brief �����ļ���
/// @param path �ļ���·��
/// @return int �����ɹ�Ϊ0������Ϊ-1
int FileSystem::mkdir(string path)
{
	vector<string> paths = stringSplit(path, '/');
	if (paths.size() == 0)
	{
		cout << "·����Ч!" << endl;
		throw(EINVAL);
		return -1;
	}

	string name = paths[paths.size() - 1];
	if (name.size() > NUM_FILE_NAME)
	{
		cout << "�½�Ŀ¼������!" << endl;
		throw(ENAMETOOLONG);
		return -1;
	}
	else if (name.size() == 0)
	{
		cout << "�½�Ŀ¼������Ϊ��!" << endl;
		throw(EINVAL);
		return -1;
	}

	Inode *fatherInode;

	// ��·����ɾ���ļ�����
	path.erase(path.size() - name.size(), name.size());
	// �ҵ���Ҫ�������ļ������ĸ��ļ�����Ӧ��Inode
	fatherInode = this->NameI(path);

	// û���ҵ���Ӧ��Inode
	if (fatherInode == NULL)
	{
		cout << "û���ҵ���Ӧ���ļ���Ŀ¼!" << endl;
		throw(ENOENT);
		return -1;
	}

	// ����ҵ����жϴ������ļ��еĸ��ļ����ǲ����ļ�������
	if (!(fatherInode->i_mode & Inode::INodeMode::IDIR))
	{
		cout << "����һ����ȷ��Ŀ¼��!" << endl;
		throw(ENOTDIR);
		return -1;
	}

	// ����ҵ����ж��Ƿ���Ȩ��д�ļ�
	if (this->Access(fatherInode, FileMode::WRITE) == 0)
	{
		cout << "û��Ȩ��д�ļ�!" << endl;
		throw(EACCES);
		return -1;
	}

	bool isFull = true;
	// ����Ȩ��д�ļ�ʱ���ж��Ƿ��������ļ����Ҳ鿴�Ƿ��п��е���Ŀ¼����д
	// ����Ҫ���������̿��
	// ����Ŀ¼�ļ�ֻռһ���̿飬����ֻ��һ�Ϊ��
	int blkno = fatherInode->Bmap(0);
	// ��ȡ���̵�����
	Buf *fatherBuf = this->bufManager->Bread(blkno);
	// ������תΪĿ¼�ṹ
	Directory *fatherDir = char2Directory(fatherBuf->b_addr);
	// ѭ������Ŀ¼�е�ÿ��Ԫ��
	for (int i = 0; i < NUM_SUB_DIR; i++)
	{
		// ����ҵ���Ӧ��Ŀ¼
		if (name == fatherDir->d_filename[i])
		{
			cout << "�ļ��Ѵ���!" << endl;
			throw(EEXIST);
			return -1;
		}
		if (isFull && fatherDir->d_inodenumber[i] == 0)
		{
			isFull = false;
		}
	}

	// ���Ŀ¼����
	if (isFull)
	{
		cout << "Ŀ¼����!" << endl;
		throw(ENOSPC);
		return -1;
	}

	// ��ſ�ʼ�����µ��ļ���
	// ����һ���µ��ڴ�Inode
	Inode *newinode = this->IAlloc();
	newinode->i_mode = Inode::INodeMode::IDIR |
					   Inode::INodeMode::OWNER_R | Inode::INodeMode::OWNER_W | Inode::INodeMode::OWNER_X |
					   Inode::INodeMode::GROUP_R | Inode::INodeMode::GROUP_X |
					   Inode::INodeMode::OTHER_R | Inode::INodeMode::OTHER_X;
	newinode->i_nlink = 1;
	newinode->i_uid = this->curId;
	newinode->i_gid = this->userTable->GetGId(this->curId);
	newinode->i_size = 0;
	newinode->i_mtime = unsigned int(time(NULL));
	newinode->i_atime = unsigned int(time(NULL));
	// �����ļ����������Ŀ¼��
	Directory *newDir = new Directory();
	newDir->mkdir(".", newinode->i_number);		// �����Լ�
	newDir->mkdir("..", fatherInode->i_number); // ��������
	// �����ļ��з��������̿��
	Buf *newBuf = this->Alloc();
	newBuf->b_addr = directory2Char(newDir);
	newinode->i_size = sizeof(Directory) / NUM_SUB_DIR * 2; // ���ļ��д�С������Ŀ¼��
	newinode->i_addr[0] = newBuf->b_blkno;
	// delete newDir;

	// �����ļ���д���丸�׵�Ŀ¼����
	fatherDir->mkdir(name.c_str(), newinode->i_number);
	fatherInode->i_size += sizeof(Directory) / NUM_SUB_DIR; // ���׵Ĵ�С����һ��Ŀ¼��
	fatherBuf->b_addr = directory2Char(fatherDir);

	// ͳһд�أ���Ŀ¼inode����Ŀ¼inode����Ŀ¼���ݿ顢��Ŀ¼���ݿ�
	fatherInode->WriteI();
	newinode->WriteI();
	// this->bufManager->bwrite(directory2Char(fatherDir), POSITION_BLOCK + fatherBuf->b_blkno, sizeof(fatherDir));
	// this->bufManager->bwrite(directory2Char(newDir), POSITION_BLOCK + newBuf->b_blkno, sizeof(newDir));

	this->bufManager->Bwrite(fatherBuf);
	this->bufManager->Bwrite(newBuf);
	// ��FS���rootInode��curInode���ͷ�����Inode
	if (fatherInode != this->rootDirInode && fatherInode != this->curDirInode)
		this->IPut(fatherInode);
	this->IPut(newinode);

	return 0;
}

/// @brief �˳�ϵͳ
void FileSystem::exit()
{
	// ��superblockд�ش���
	this->WriteSpb();
	// TODO:��userTableд�ش���

	// �����б���ӳ�д�����ݶ�д�ش���
	this->bufManager->SaveAll();
}

/// @brief ��ʼ���ļ�ϵͳ
void FileSystem::fformat()
{
	fstream fd(DISK_PATH, ios::out);
	fd.close();
	fd.open(DISK_PATH, ios::out | ios::in | ios::binary);
	// ���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open())
	{
		cout << "�޷���һ���ļ�myDisk.img" << endl;
		throw(errno);
	}
	fd.close();

	// �ȶ��û����г�ʼ��
	this->userTable = new UserTable();
	this->userTable->AddRoot(); // ���root�û�
	this->curId = ROOT_ID;
	this->curName = "root";
	this->userTable->AddUser(this->curId, "unix", "1", ROOT_GID + 1); // ���unix�û�

	// �Ի���������ݽ��г�ʼ��
	this->bufManager = new BufferManager();
	this->spb = new SuperBlock();

	// ���ܶ�superblock���г�ʼ������Ϊ����ú���
	this->spb->Init();
	// ��superblockд�ش���
	this->WriteSpb();

	// ���ڶ�Ŀ¼���г�ʼ��
	// ����һ�����е����Inode��������Ŀ¼
	this->rootDirInode = this->IAlloc();
	this->rootDirInode->i_uid = ROOT_ID;
	this->rootDirInode->i_gid = this->userTable->GetGId(ROOT_ID);
	this->rootDirInode->i_mode = Inode::INodeMode::IDIR |
								 Inode::INodeMode::OWNER_R | Inode::INodeMode::OWNER_W | Inode::INodeMode::OWNER_X |
								 Inode::INodeMode::GROUP_R | Inode::INodeMode::GROUP_X |
								 Inode::INodeMode::OTHER_R | Inode::INodeMode::OTHER_X;
	this->rootDirInode->i_nlink = 1;
	this->rootDirInode->i_size = 0;
	this->rootDirInode->i_mtime = unsigned int(time(NULL));
	this->rootDirInode->i_atime = unsigned int(time(NULL));
	this->curDirInode = this->rootDirInode;
	// ����һ�������̿��Ÿ�Ŀ¼����
	Directory *rootDir = new Directory();
	rootDir->mkdir(".", this->rootDirInode->i_number);	// �����Լ�
	rootDir->mkdir("..", this->rootDirInode->i_number); // �������ף���Ŀ¼�ĸ��׾����Լ�����Ҳ��Ϊʲô����ֱ�ӵ���mkdir������ԭ��
	this->curDir = "/";

	// ��root�ļ��з��������̿�Ų���д�ش�����������
	Buf *newBuf = this->Alloc();
	newBuf->b_addr = directory2Char(rootDir);
	this->bufManager->Bwrite(newBuf);
	// ��Inodeд��������λ��
	this->rootDirInode->i_size = sizeof(Directory) / NUM_SUB_DIR * 2;
	this->rootDirInode->i_addr[0] = newBuf->b_blkno;
	// delete rootDir; û�㶮��������ɾ��

	// ����Ҫ�����Ŀ¼
	this->mkdir("/bin");
	this->mkdir("/etc");
	this->mkdir("/home");
	this->mkdir("/dev");
	// ��rootInodeд�ش�����
	this->rootDirInode->WriteI();

	// ������д���û���
	this->fcreate("/etc/userTable.txt");
	int filoc = fopen("/etc/userTable.txt");
	File *userTableFile = &this->openFileTable[filoc];
	this->fwrite(userTable2Char(this->userTable), sizeof(UserTable), userTableFile); // ��Ҫȫ��д��
	this->fclose(userTableFile);
}

/// @brief ���ļ�
/// @param path �ļ�·��
/// @return File* ���ش��ļ���ָ��
int FileSystem::fopen(string path)
{
	Inode *pinode = this->NameI(path);

	// û���ҵ���Ӧ��Inode
	if (pinode == NULL)
	{
		cout << "û���ҵ���Ӧ���ļ���Ŀ¼!" << endl;
		throw(ENOENT);
		return NULL;
	}

	// ����ҵ����ж���Ҫ�ҵ��ļ��ǲ����ļ�����
	if (!(pinode->i_mode & Inode::INodeMode::IFILE))
	{
		cout << "����һ����ȷ���ļ�!" << endl;
		throw(ENOTDIR);
		return NULL;
	}

	// ����ҵ����ж��Ƿ���Ȩ�޴��ļ�
	if (this->Access(pinode, FileMode::EXC) == 0)
	{
		cout << "û��Ȩ�޴��ļ�!" << endl;
		throw(EACCES);
		return NULL;
	}

	// ������ļ����ƿ�File�ṹ
	int fileloc = 0;
	File *pFile = this->FAlloc(fileloc);
	if (NULL == pFile)
	{
		cout << "��̫���ļ�!" << endl;
		throw(ENFILE);
		return NULL;
	}
	pFile->f_inode = pinode;
	pFile->f_offset = 0;
	pFile->f_uid = this->curId;
	pFile->f_gid = this->userTable->GetGId(this->curId);

	// �޸ķ���ʱ��
	pinode->i_atime = unsigned int(time(NULL));

	return fileloc;
}

/// @brief д�ļ�
/// @param buffer д�������
/// @param count д����ֽ���
/// @param fp �ļ�ָ��
void FileSystem::fwrite(const char *buffer, int count, File *fp)
{
	cout << *buffer << endl;
	if (fp == NULL)
	{
		cout << "�ļ�ָ��Ϊ��!" << endl;
		throw(EBADF);
		return;
	}

	// ����ҵ����ж��Ƿ���Ȩ�޴��ļ�
	if (this->Access(fp->f_inode, FileMode::WRITE) == 0)
	{
		cout << "û��Ȩ��д�ļ�!" << endl;
		throw(EACCES);
		return;
	}

	if (count + fp->f_offset > SIZE_BLOCK * NUM_I_ADDR)
	{
		cout << "д���ļ�̫��!" << endl;
		throw(EFBIG);
		return;
	}
	// ��ȡ�ļ���Inode
	Inode *pInode = fp->f_inode;

	// д�ļ������������
	// 1. д�����ʼλ��Ϊ�߼������ʼ��ַ��д���ֽ���Ϊ512-------�첽д
	// 2. �� д�����ʼλ��Ϊ�߼������ʼ��ַ��д���ֽ���Ϊ512----��Bread
	//  2.1 д������ĩβ----------------------------------------�첽д
	//	2.2 û��д������ĩβ-------------------------------------�ӳ�д

	int pos = 0; // �Ѿ�д����ֽ���
	while (pos < count)
	{
		// ���㱾��д��λ�����ļ��е�λ��
		int startpos = fp->f_offset + pos;
		// ���㱾��д�������̿�ţ�����ļ���С�����Ļ����������·��������̿�
		int blkno = pInode->Bmap(startpos % SIZE_BLOCK);
		// ���㱾��д��Ĵ�С
		int size = SIZE_BLOCK - startpos % SIZE_BLOCK;
		if (size > count - pos)
			size = count - pos; // ����д��Ĵ�С

		// ���д�����ʼλ��Ϊ�߼������ʼ��ַ��д���ֽ���Ϊ512-------�첽д
		if (startpos % SIZE_BLOCK == 0 && size == SIZE_BLOCK)
		{
			// ���뻺��
			Buf *pBuf = this->bufManager->GetBlk(blkno);
			// ������д�뻺��
			memcpy(pBuf->b_addr, buffer + pos, size);
			// ����������д�����
			this->bufManager->Bwrite(pBuf);
		}
		else
		{ // �� д�����ʼλ��Ϊ�߼������ʼ��ַ��д���ֽ���Ϊ512----��Bread
			// ���뻺��
			Buf *pBuf = this->bufManager->Bread(blkno);
			// ������д�뻺��
			memcpy(pBuf->b_addr + startpos % SIZE_BLOCK, buffer + pos, size);

			// д������ĩβ---�첽д
			if (startpos % SIZE_BLOCK + size == SIZE_BLOCK)
				this->bufManager->Bwrite(pBuf);
			else // û��д������ĩβ---�ӳ�д
				this->bufManager->Bdwrite(pBuf);
		}

		pos += size;
		fp->f_offset += size;
	}

	if (pInode->i_size < fp->f_offset)
		pInode->i_size = fp->f_offset;
}

// ����fd�ر��ļ�
void FileSystem::fclose(File *fp)
{
	// �ͷ��ڴ���
	this->IPut(fp->f_inode);
	fp->Clean();
}

/// @brief ��ȡ��ǰĿ¼�µ�Ŀ¼��
/// @return Directory ���ص�ǰĿ¼�µ�Ŀ¼��
Directory FileSystem::getDir()
{
	// �������Ŀ¼�ļ�
	if (this->curDirInode->i_mode & Inode::INodeMode::IFILE)
		return Directory();

	// �����Ŀ¼�ļ�
	int blkno = this->curDirInode->Bmap(0);
	// ��ȡ���̵�����
	Buf *pbuf = this->bufManager->Bread(blkno);
	// ������תΪĿ¼�ṹ
	Directory *dir = char2Directory(pbuf->b_addr);
	return *dir;
}

/// @brief ���ļ����ַ�����
/// @param fp �ļ�ָ��
/// @param buffer ��ȡ������Ҫ��ŵ��ַ���
/// @param count  ��ȡ���ֽ���
void FileSystem::fread(File *fp, char *buffer, int count)
{
	if (fp == NULL)
	{
		cout << "�ļ�ָ��Ϊ��!" << endl;
		throw(EBADF);
		return;
	}

	// ����ҵ����ж��Ƿ���Ȩ�޴��ļ�
	if (this->Access(fp->f_inode, FileMode::READ) == 0)
	{
		cout << "û��Ȩ�޶��ļ�!" << endl;
		throw(EACCES);
		return;
	}

	// ��ȡ�ļ���Inode
	Inode *pInode = fp->f_inode;
	buffer = new char(count);
	int pos = 0; // �Ѿ���ȡ���ֽ���
	while (pos < count)
	{
		// ���㱾��ȡλ�����ļ��е�λ��
		int startpos = fp->f_offset + pos;
		if (startpos >= pInode->i_size) // ��ȡλ�ó����ļ���С
			break;
		// ���㱾�ζ�ȡ�����̿�ţ�������һ���ж�,�����ж�ȡλ�ó����ļ���С������
		int blkno = pInode->Bmap(startpos % SIZE_BLOCK);
		// ���㱾�ζ�ȡ�Ĵ�С
		int size = SIZE_BLOCK - startpos % SIZE_BLOCK;
		if (size > count - pos)
			size = count - pos; // ������ȡ�Ĵ�С

		Buf *pBuf = this->bufManager->Bread(blkno);
		// TODO:���д��ļ�������Ҫ��
		memcpy(buffer + pos, pBuf->b_addr + startpos % SIZE_BLOCK, size);
		pos += size;
		fp->f_offset += size;
	}
}

FileSystem::~FileSystem()
{
	this->exit();
	delete this->bufManager;
	delete this->spb;
	delete this->userTable;
	delete this->curDirInode;
	delete this->rootDirInode;
}