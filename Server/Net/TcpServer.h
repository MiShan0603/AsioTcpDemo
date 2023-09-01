#pragma once

/*
* Copyright (c) 2013 3598392@qq.com
* All rights reserved.
*
* 当前版本：1.0
* 作 者：MiShan0603
* 完成日期：2013年9月1日
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

#pragma region 时间同步

const int ClockSync_Fps = /*60*/50;
const int ClockSync_TimeSpan = 1000 / ClockSync_Fps;

#pragma endregion 时间同步


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

#pragma region 执行时间同步

private:
	struct ClockSync_Info {

		/// <summary>
		/// 1秒发送同步的时间数
		/// </summary>
		int fps = 0;

		// 存储上一次开始同步的时间点
		int64_t lastTime = 0;
	};

	ClockSync_Info m_clockSyncInfo;

public:
	void DoTimeCallback();

#pragma endregion 执行时间同步

private:
	boost::asio::/*io_context*/io_service m_ioservice;
	std::shared_ptr<std::thread> m_workerThread = nullptr;

	boost::asio::ip::tcp::socket m_sock;
	boost::asio::ip::tcp::acceptor m_acceptor;
	boost::system::error_code m_errorCode;

	std::mutex m_sessionsMutex;
	typedef std::vector<boost::shared_ptr<TcpSession>> SessionPtrList;
	SessionPtrList m_sessions;

#pragma region 时间同步

	MMRESULT	m_mmResultClockSync = NULL;
	Utils::Clock m_clock;

#pragma endregion 时间同步
};