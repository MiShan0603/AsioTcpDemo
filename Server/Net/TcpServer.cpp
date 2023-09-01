#include "TcpServer.h"
#include <WinDef.h>

#pragma region ntdll_NtDelayExecution（精度更高一些的Sleep）

typedef LONG(__stdcall* __PFN_NtDelayExecution)(BOOLEAN Alertable, PLARGE_INTEGER Interval);

void DSSleep(DWORD dwMs)
{
	static __PFN_NtDelayExecution fn = 0;
	static bool bInited = false;

	if (!bInited)
	{
		HMODULE hnt = ::GetModuleHandleW(L"ntdll.dll");
		fn = (__PFN_NtDelayExecution)(::GetProcAddress(hnt, "NtDelayExecution"));
		bInited = true;
	}

	if (fn)
	{
		LARGE_INTEGER ll;
		ll.QuadPart = -1 * ((LONGLONG)dwMs) * 10000;
		fn(TRUE, &ll);
	}
	else
	{
		::Sleep(dwMs);
	}
}

void USSleep(DWORD dwUs)
{
	static __PFN_NtDelayExecution fn = 0;
	static bool bInited = false;

	if (!bInited)
	{
		HMODULE hnt = ::GetModuleHandleW(L"ntdll.dll");
		fn = (__PFN_NtDelayExecution)(::GetProcAddress(hnt, "NtDelayExecution"));
		bInited = true;
	}

	if (fn)
	{
		LARGE_INTEGER ll;
		ll.QuadPart = -1 * ((LONGLONG)dwUs) * 10;
		fn(TRUE, &ll);
	}
	else
	{
		::Sleep(dwUs);
	}
}

#pragma endregion ntdll_NtDelayExecution（精度更高一些的Sleep）



void  CALLBACK ClockSync_TimeCallBack(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	TcpServer* pSrv = (TcpServer*)dwUser;
	pSrv->DoTimeCallback();
}


TcpServer::TcpServer() :
	m_sock(m_ioservice),
	m_acceptor(m_ioservice)
{

}

TcpServer::~TcpServer()
{
	Stop();
}

bool TcpServer::Start(unsigned short port)
{
	m_acceptor.open(boost::asio::ip::tcp::v4(), m_errorCode);
	if (m_errorCode)
	{
		std::cout << "open error :" << m_errorCode << std::endl;
		return false;
	}

	m_acceptor.set_option(boost::asio::socket_base::reuse_address(true));
	m_acceptor.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port), m_errorCode);
	if (m_errorCode)
	{
		auto msg = m_errorCode.message();
		std::cout << "bind error :" << m_errorCode << std::endl;
		return false;
	}

	std::cout << "port:" << port << std::endl;
	m_acceptor.listen();

	async_accept();

#pragma region 时间同步

	m_mmResultClockSync = timeSetEvent(ClockSync_TimeSpan, 1, ClockSync_TimeCallBack, (DWORD_PTR)this, TIME_PERIODIC);

#pragma endregion 时间同步

	m_workerThread = std::make_shared<std::thread>([&]() {
		m_ioservice.run();
	});

	if (!m_workerThread)
		return false;

	return true;
}

bool TcpServer::Stop()
{
	m_sock.close();
	m_acceptor.close();

	if (m_mmResultClockSync)
	{
		timeKillEvent(m_mmResultClockSync);
		m_mmResultClockSync = NULL;
	}

	m_ioservice.stop();

	if (m_workerThread)
	{
		m_workerThread->join();
		m_workerThread.reset();
	}

	return true;
}

void TcpServer::write(void* cmd, int cmdLen)
{
	std::lock_guard<std::mutex> lck(m_sessionsMutex);
	SessionPtrList::iterator it = m_sessions.begin();
	while (it != m_sessions.end())
	{
		if ((*it)->GetLastError())
		{
			it = m_sessions.erase(it);
		}
		else
		{
			(*it)->write(cmd, cmdLen);
			it++;
		}
	}
}


void TcpServer::async_accept()
{
	m_acceptor.async_accept(m_sock, std::bind(&TcpServer::on_accept, shared_from_this(), std::placeholders::_1));
}

void TcpServer::on_accept(const boost::system::error_code& ec)
{
	if (!ec)
	{
		std::lock_guard<std::mutex> lck(m_sessionsMutex);
		boost::shared_ptr<TcpSession> sessionPtr = boost::make_shared<TcpSession>(std::move(m_sock));
		// ptr->async_read();
		m_sessions.push_back(sessionPtr);
	}

	//重新获取连接
	async_accept();
}


void TcpServer::DoTimeCallback()
{
	if (m_clockSyncInfo.lastTime == 0)
	{
		m_clockSyncInfo.lastTime = m_clock.getTime64();
	}

	auto curTime = m_clock.getTime64();

	struct SyncTime
	{
		Cmd_Header header;
		Cmd_SnycTime time;
	};

	SyncTime time;
	time.header.cmdLen = sizeof(Cmd_SnycTime);
	time.header.cmdType = CmdType_SyncTime;
	time.time.time = curTime;
	std::lock_guard<std::mutex> lck(m_sessionsMutex);
	SessionPtrList::iterator it = m_sessions.begin();
	while (it != m_sessions.end())
	{
		if ((*it)->GetLastError())
		{
			it = m_sessions.erase(it);
		}
		else
		{
			(*it)->write((void**)&time, sizeof(SyncTime));
			it++;
		}
	}
}