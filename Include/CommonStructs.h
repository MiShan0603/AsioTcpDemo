#pragma once

#include <Windows.h>
#include <string>
#include <vector>

enum CmdType
{
	CmdType_Unknown = 0x0,
	CmdType_HeartBeat = 0x1,
	CmdType_SyncTime, 
	CmdType_MouseInfo,
};

/// <summary>
/// ����ͷ
/// </summary>
typedef struct Cmd_Header
{
	int cmdLen;
	CmdType cmdType;

	Cmd_Header()
	{
		cmdLen = 0;
		cmdType = CmdType_Unknown;
	}
};

/// <summary>
/// ����-ͬ��ʱ��
/// </summary>
struct Cmd_SnycTime
{
	int64_t time;

	Cmd_SnycTime()
	{
		time = 0;
	}
};

/// <summary>
/// ���ԣ������Ϣ
/// </summary>
struct Cmd_MouseInfo
{
	int button;
	int x;
	int y;
};