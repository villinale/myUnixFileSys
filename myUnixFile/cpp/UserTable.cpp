/*
 * @Author: yingxin wang
 * @Date: 2023-05-08 17:37:16
 * @LastEditors: yingxin wang
 * @LastEditTime: 2023-05-08 21:21:05
 * @Description: User相关操作
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
/// 添加用户
/// </summary>
/// <param name="id">进行操作的用户id</param>
/// <param name="name">所要添加的用户名</param>
/// <param name="password">所要添加的用户密码</param>
/// <param name="givengid">所要添加的用户gid</param>
void UserTable::AddUser(const short id, const char *name, const char *password, const short givengid)
{
	if (id != ROOT_ID)
	{
		cout << "当前用户无权限创建用户，只有root用户可以执行该操作" << endl;
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
			cout << "创建成功！" << endl;
			throw(EUSERS);
			return;
		}
	}

	cout << "创建失败！用户已满" << endl;
	return;
}

/// <summary>
/// 删除指定用户
/// </summary>
/// <param name="id">进行操作的用户id</param>
/// <param name="name">所要删除的用户名</param>
void UserTable::DeleteUser(const short id, const char *name)
{
	if (id != ROOT_ID)
	{
		cout << "当前用户无权限删除用户，只有root用户可以执行该操作" << endl;
		throw(EPERM);
		return;
	}

	if (strcmp(name, "root") == 0)
	{
		cout << "root用户不可以被删除" << endl;
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
			cout << "删除成功！" << endl;
			return;
		}
	}

	if (isfind == false)
		cout << "删除失败！无该用户存在" << endl;
	return;
}

/// @brief 通过id寻找gid
/// @param id 用户id
/// @return 返回用户gid，没找到返回-1
unsigned short UserTable::GetGId(const unsigned short id)
{
	for (int i = 0; i < NUM_USER; i++)
		if (this->u_id[i] == id)
			return this->u_gid[i];
	return -1;
}