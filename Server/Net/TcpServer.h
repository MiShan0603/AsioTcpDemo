#pragma once

/*
* Copyright (c) 2013 3598392@qq.com
* All rights reserved.
*
* ��ǰ�汾��1.0
* �� �ߣ�MiShan0603
* ������ڣ�2013��9��1��
*
*/

#include <iostream>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/detail/thread.hpp>
#include <Mmsystem.h>
#pragma comment(lib, "Winmm.lib")
#include "TcpSession.h"
#include "Utils/Clock.h"

#pragma region ʱ��ͬ��

const int ClockSync_Fps = /*60*/50;
const int ClockSync_TimeSpan = 1000 / ClockSync_Fps;

#pragma endregion ʱ��ͬ��


class TcpServer : public std::enable_shared_from_this<TcpServer>
{
public:
	TcpServer();
	~TcpServer();

	bool Start(unsigned short port);
	bool Stop();

	void write(void* cmd, int cmdLen);

	boost::system::error_code GetErrorCode()
	{
		return m_errorCode;
	};
	
private:
	void async_accept();

	void on_accept(const boost::system::error_code& ec);

#pragma region ִ��ʱ��ͬ��

private:
	struct ClockSync_Info {

		/// <summary>
		/// 1�뷢��ͬ����ʱ����
		/// </summary>
		int fps = 0;

		// �洢��һ�ο�ʼͬ����ʱ���
		int64_t lastTime = 0;
	};

	ClockSync_Info m_clockSyncInfo;

public:
	void DoTimeCallback();

#pragma endregion ִ��ʱ��ͬ��

private:
	boost::asio::/*io_context*/io_service m_ioservice;
	std::shared_ptr<std::thread> m_workerThread = nullptr;

	boost::asio::ip::tcp::socket m_sock;
	boost::asio::ip::tcp::acceptor m_acceptor;
	boost::system::error_code m_errorCode;

	std::mutex m_sessionsMutex;
	typedef std::vector<boost::shared_ptr<TcpSession>> SessionPtrList;
	SessionPtrList m_sessions;

#pragma region ʱ��ͬ��

	MMRESULT	m_mmResultClockSync = NULL;
	Utils::Clock m_clock;

#pragma endregion ʱ��ͬ��
};