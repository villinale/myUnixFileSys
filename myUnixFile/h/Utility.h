#pragma once
#include "header.h"

/*
* 定义一些工具类和工具函数
*/
class Utility
{
public:
	//初始化
	static void Init();

	//读出User块
	static void ReadUser(User& user);

	//写User块
	static void WriteUser(User user);
};