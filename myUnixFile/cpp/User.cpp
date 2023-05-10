/*
 * @Author: yingxin wang
 * @Date: 2023-05-08 17:37:16
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-08 21:21:05
 * @Description: User��ز���
 */
#include "../h/header.h"
#include "../h/Utility.h"

User::User()
{
	for (int i = 0; i < NUM_USER; i++)
	{
		this->u_id[i] = -1;
		this->u_gid[i] = -1;
	}
}

void AddUser(const short id, const char* name, const char* password, const short givengid)
{
	if (id != ROOT_ID)
	{
		cout << "��ǰ�û���Ȩ�޴����û���ֻ��root�û�����ִ�иò���" << endl;
		throw(EPERM);
		return;
	}

	User user;
	Utility::ReadUser(user);

	for (int i = 0; i < NUM_USER; i++)
	{
		if (user.u_id[i] == -1)
		{
			strcpy(user.u_name[i], name);
			strcpy(user.u_password[i], password);
			user.u_id[i] = i;
			user.u_gid[i] = givengid;
			Utility::WriteUser(user);
			cout << "�����ɹ���" << endl;
			throw(EUSERS);
			return;
		}
	}

	cout << "����ʧ�ܣ��û�����" << endl;
	return;
}


void DeleteUser(const short id, const char* name)
{
	if (id != ROOT_ID)
	{
		cout << "��ǰ�û���Ȩ��ɾ���û���ֻ��root�û�����ִ�иò���" << endl;
		throw(EPERM);
		return;
	}

	User user;
	Utility::ReadUser(user);

	bool isfind = false;
	for (int i = 0; i < NUM_USER; i++)
	{
		if (strcmp(user.u_name[i], name) == 0)
		{
			isfind = true;
			user.u_id[i] = -1;
			user.u_gid[i] = -1; 
			strcpy(user.u_name[i], "");
			strcpy(user.u_password[i], "");
			Utility::WriteUser(user);
			cout << "ɾ���ɹ���" << endl;
			return;
		}
	}

	if (isfind == false)
		cout << "ɾ��ʧ�ܣ��޸��û�����" << endl;
	return;
}