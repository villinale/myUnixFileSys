/*
 * @Author: yingxin wang
 * @Date: 2023-05-08 17:37:16
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-16 16:34:32
 * @Description: User相关操作
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

/// @brief 添加root用户
void UserTable::AddRoot()
{
	this->u_id[0] = ROOT_ID;
	this->u_gid[0] = ROOT_GID;
	strcpy(this->u_name[0], "root");
	strcpy(this->u_password[0], "root");
}

/// @brief 添加用户
/// @param id  进行操作的用户id
/// @param name  所要添加的用户名
/// @param password 所要添加的用户密码
/// @param givengid 所要添加的用户所在组id
void UserTable::AddUser(const short id, const char *name, const char *password, const short givengid)
{
	if (id != ROOT_ID)
	{
		cout << "没有权限新建用户!只有root用户才可以" << endl;
		throw(EPERM);
		return;
	}

	if (strcmp(name, "root") == 0)
	{
		cout << "不可以与root用户重名" << endl;
		throw(EPERM);
		return;
	}

	for (int i = 1; i < NUM_USER; i++)
	{
		if (strcmp(this->u_name[i], name) == 0)
		{
			cout << "与" << name << "用户重名" << endl;
			throw(EPERM);
			return;
		}
		else if (this->u_id[i] == -1)
		{
			strcpy(this->u_name[i], name);
			strcpy(this->u_password[i], password);
			this->u_id[i] = i;
			this->u_gid[i] = givengid;
			cout << "创建成功!" << endl;
			throw(EUSERS);
			return;
		}
	}

	cout << "用户已满!" << endl;

	return;
}

/// @brief 删除用户
/// @param id  进行操作的用户id
/// @param name  所要删除的用户名
void UserTable::DeleteUser(const short id, const char *name)
{
	if (id != ROOT_ID)
	{
		cout << "没有权限删除用户!只有root用户才可以" << endl;
		throw(EPERM);
		return;
	}

	if (strcmp(name, "root") == 0)
	{
		cout << "root不可被删除!" << endl;
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
			cout << "删除成功!" << endl;
			return;
		}
	}

	if (isfind == false)
		cout << "没有该用户信息!" << endl;
	return;
}

/// @brief 根据用户id获取用户所在组id
/// @param id  用户id
/// @return  返回用户所在组id
short UserTable::GetGId(const short id)
{
	for (int i = 0; i < NUM_USER; i++)
		if (this->u_id[i] == id)
			return this->u_gid[i];
	return -1;
}