/*
 * @Author: yingxin wang
 * @Date: 2023-05-08 17:37:16
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-08 21:21:05
 * @Description: User��ز���
 */
#include "../h/header.h"

UserTable::UserTable()
{
	for (int i = 0; i < NUM_USER; i++)
	{
		this->u_id[i] = -1;
		this->u_gid[i] = -1;
	}
}

/// <summary>
/// ����û�
/// </summary>
/// <param name="id">���в������û�id</param>
/// <param name="name">��Ҫ��ӵ��û���</param>
/// <param name="password">��Ҫ��ӵ��û�����</param>
/// <param name="givengid">��Ҫ��ӵ��û�gid</param>
void UserTable::AddUser(const short id, const char *name, const char *password, const short givengid)
{
	if (id != ROOT_ID)
	{
		cout << "��ǰ�û���Ȩ�޴����û���ֻ��root�û�����ִ�иò���" << endl;
		throw(EPERM);
		return;
	}

	for (int i = 1; i < NUM_USER; i++)
	{
		if (this->u_id[i] == -1)
		{
			strcpy(this->u_name[i], name);
			strcpy(this->u_password[i], password);
			this->u_id[i] = i;
			this->u_gid[i] = givengid;
			cout << "�����ɹ���" << endl;
			throw(EUSERS);
			return;
		}
	}

	cout << "����ʧ�ܣ��û�����" << endl;
	return;
}

/// <summary>
/// ɾ��ָ���û�
/// </summary>
/// <param name="id">���в������û�id</param>
/// <param name="name">��Ҫɾ�����û���</param>
void UserTable::DeleteUser(const short id, const char *name)
{
	if (id != ROOT_ID)
	{
		cout << "��ǰ�û���Ȩ��ɾ���û���ֻ��root�û�����ִ�иò���" << endl;
		throw(EPERM);
		return;
	}

	if (strcmp(name, "root") == 0)
	{
		cout << "root�û������Ա�ɾ��" << endl;
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
			cout << "ɾ���ɹ���" << endl;
			return;
		}
	}

	if (isfind == false)
		cout << "ɾ��ʧ�ܣ��޸��û�����" << endl;
	return;
}

/// @brief ͨ��idѰ��gid
/// @param id �û�id
/// @return �����û�gid��û�ҵ�����-1
unsigned short UserTable::GetGId(const unsigned short id)
{
	for (int i = 0; i < NUM_USER; i++)
		if (this->u_id[i] == id)
			return this->u_gid[i];
	return -1;
}