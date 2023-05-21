/*
 * @Author: yingxin wang
 * @Date: 2023-05-08 17:37:16
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-16 16:34:32
 * @Description: User��ز���
 */
#include "../h/header.h"

UserTable::UserTable()
{
	for (int i = 0; i < NUM_USER; i++)
	{
		this->u_id[i] = -1;
		this->u_gid[i] = -1;
		strcpy(this->u_name[i], "");
		strcpy(this->u_password[i], "");
	}
}

/// @brief ���root�û�
void UserTable::AddRoot()
{
	this->u_id[0] = ROOT_ID;
	this->u_gid[0] = ROOT_GID;
	strcpy(this->u_name[0], "root");
	strcpy(this->u_password[0], "root");
}

/// @brief ����û�
/// @param id  ���в������û�id
/// @param name  ��Ҫ��ӵ��û���
/// @param password ��Ҫ��ӵ��û�����
/// @param givengid ��Ҫ��ӵ��û�������id
void UserTable::AddUser(const short id, const char *name, const char *password, const short givengid)
{
	if (id != ROOT_ID)
	{
		cout << "û��Ȩ���½��û�!ֻ��root�û��ſ���" << endl;
		throw(EPERM);
		return;
	}

	if (strcmp(name, "root") == 0)
	{
		cout << "��������root�û�����" << endl;
		throw(EPERM);
		return;
	}

	for (int i = 1; i < NUM_USER; i++)
	{
		if (strcmp(this->u_name[i], name) == 0)
		{
			cout << "��" << name << "�û�����" << endl;
			throw(EPERM);
			return;
		}
		else if (this->u_id[i] == -1)
		{
			strcpy(this->u_name[i], name);
			strcpy(this->u_password[i], password);
			this->u_id[i] = i;
			this->u_gid[i] = givengid;
			cout << "�����ɹ�!" << endl;
			throw(EUSERS);
			return;
		}
	}

	cout << "�û�����!" << endl;

	return;
}

/// @brief ɾ���û�
/// @param id  ���в������û�id
/// @param name  ��Ҫɾ�����û���
void UserTable::DeleteUser(const short id, const char *name)
{
	if (id != ROOT_ID)
	{
		cout << "û��Ȩ��ɾ���û�!ֻ��root�û��ſ���" << endl;
		throw(EPERM);
		return;
	}

	if (strcmp(name, "root") == 0)
	{
		cout << "root���ɱ�ɾ��!" << endl;
		throw(EPERM);
		return;
	}

	bool isfind = false;
	for (int i = 1; i < NUM_USER; i++)
	{
		if (strcmp(this->u_name[i], name) == 0)
		{
			isfind = true;
			this->u_id[i] = -1;
			this->u_gid[i] = -1;
			strcpy(this->u_name[i], "");
			strcpy(this->u_password[i], "");
			cout << "ɾ���ɹ�!" << endl;
			return;
		}
	}

	if (isfind == false)
		cout << "û�и��û���Ϣ!" << endl;
	return;
}

/// @brief �����û�id��ȡ�û�������id
/// @param id  �û�id
/// @return  �����û�������id
short UserTable::GetGId(const short id)
{
	for (int i = 0; i < NUM_USER; i++)
		if (this->u_id[i] == id)
			return this->u_gid[i];
	return -1;
}