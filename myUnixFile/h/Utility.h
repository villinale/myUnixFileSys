#pragma once
#include "header.h"

/*
* ����һЩ������͹��ߺ���
*/
class Utility
{
public:
	//��ʼ��
	static void Init();

	//����User��
	static void ReadUser(User& user);

	//дUser��
	static void WriteUser(User user);
};