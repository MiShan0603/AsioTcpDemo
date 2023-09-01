#include "TcpClient.h"
#include <WinDef.h>

#include <iostream>
#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

#define SMS_BUFFER_SIZE                 1024 * 1024 * 4

TcpClient::TcpClient()
    : m_bConnected(false)
    , m_buffer(SMS_BUFFER_SIZE)
{
    m_cmdHeader = (Cmd_Header*)m_buffer.data();
    m_bufferDataUsedLen += sizeof(Cmd_Header);

    m_cmdSnycTime = (Cmd_SnycTime*)(m_buffer.data() + m_bufferDataUsedLen);
    m_bufferDataUsedLen += sizeof(Cmd_SnycTime);

    m_mouseInfo = (Cmd_MouseInfo*)(m_buffer.data() + m_bufferDataUsedLen);
    m_bufferDataUsedLen += sizeof(Cmd_MouseInfo);

    m_UnknownCmd = (void*)(m_buffer.data() + m_bufferDataUsedLen);

    m_dummy_work = boost::make_shared<boost::asio::io_service::work>(m_ioservice);
    m_workerThread = std::make_shared<std::thread>([&]() {
        m_ioservice.run();
    });

}

TcpClient::~TcpClient()
{
    if (m_heartTimer)
    {
        m_heartTimer->cancel();
        m_heartTimer = nullptr;
    }

    if (m_socketPtr != NULL)
    {
        boost::system::error_code call_ec;
        m_socketPtr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, call_ec);
        m_socketPtr->close(call_ec);
    }

    m_ioservice.stop();
    m_dummy_work.reset();

    if (m_workerThread)
    {
        m_workerThread->join();
        m_workerThread.reset();
    }
}

void TcpClient::SetEventCallBack(TcpClientEventCallBack cb)
{
    m_eventCb = cb;
}

bool TcpClient::AsyncConnect(string ip, unsigned short port)
{
    if (m_bConnected)
    {
        X_PRINT("socket has established");
        return false;
    }

    {
        boost::asio::ip::tcp::endpoint peer_ep(boost::asio::ip::address::from_string(ip), short(port));
        m_endPoint = peer_ep;
        m_socketPtr.reset(new socket_type(m_ioservice));
        m_socketPtr->async_connect(m_endPoint, 
            boost::bind(&TcpClient::OnAsyncConnect, shared_from_this(), boost::asio::placeholders::error));
    }


    return true;
}

void TcpClient::OnAsyncConnect(const boost::system::error_code& ec)
{
    if (ec)
    {
        // 连接出现错误
        X_PRINT("connect error:%s", ec.message().c_str());
        if (!m_eventCb.empty())
        {
            m_eventCb(TCP_EVENT_CONNECT_FAILED, ec.message());
        }

        m_bConnected = false;

        {
            boost::system::error_code call_ec;
            if (m_socketPtr->is_open())
            {
                m_socketPtr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, call_ec);
                m_socketPtr->close(call_ec);
            }
        }
        return;
    }

    m_bConnected = true;

    AsyncReadHead();



#if 0
    m_heartTimer = asio::steady_timer(m_ioservice, std::chrono::seconds(3));
    m_heartTimer.async_wait(std::bind(&TcpClient::heartbeat, this));
#else
    m_heartTimer = std::make_shared<asio::steady_timer>(m_ioservice, asio::chrono::seconds(3));
    m_heartTimer->async_wait(boost::bind(&TcpClient::heartbeat, shared_from_this()));
#endif
}


void TcpClient::heartbeat()
{
    if (!m_bConnected)
    {
        return;
    }

    Cmd_Header header;
    header.cmdLen = 0;
    header.cmdType = CmdType_HeartBeat;

    write(&header, sizeof(Cmd_Header));


    m_heartTimer->expires_after(std::chrono::seconds(5));
    m_heartTimer->async_wait(boost::bind(&TcpClient::heartbeat, shared_from_this()));
}


void TcpClient::write(void* cmd, int cmdLen)
{
    auto self(shared_from_this());

    boost::asio::async_write(
        *m_socketPtr, boost::asio::buffer(cmd, cmdLen),
        [this, self](const boost::system::error_code& ec, size_t sendsize)
        {
            if (ec)
            {
                if (!m_eventCb.empty())
                {
                    m_eventCb(TCP_EVENT_WRITE_FAILED, ec.message());
                }

                if (!m_bConnected)
                {
                    // 避免多次执行关闭操作而导致崩溃
                    return;
                }

                m_bConnected = false;

                {
                    boost::system::error_code call_ec;
                    if (m_socketPtr->is_open())
                    {
                        m_socketPtr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, call_ec);
                        m_socketPtr->close(call_ec);
                    }
                }
            }

        }
    );
}


void TcpClient::AsyncReadHead()
{
    boost::asio::async_read(
        *m_socketPtr, boost::asio::buffer(m_cmdHeader, sizeof(Cmd_Header)),
        boost::bind(&TcpClient::OnHeaderReceived, shared_from_this(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}


void TcpClient::OnHeaderReceived(const boost::system::error_code& ec, size_t recvSize)
{
    if (ec)
    {
        X_PRINT("read error:%s", ec.message().c_str());
        if (!m_eventCb.empty())
        {
            m_eventCb(TCP_EVENT_READ_FAILED, ec.message());
        }

        if (!m_bConnected)
        {
            return;
        }

        m_bConnected = false;

        if (0)
        {
            // 不在这里关闭
            boost::system::error_code call_ec;
            if (m_socketPtr->is_open())
            {
                m_socketPtr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, call_ec);
                m_socketPtr->close(call_ec);
            }
        }

        return;
    }

    switch (m_cmdHeader->cmdType)
    {
    case CmdType_SyncTime:
        AsyncReadSyncTime();
        break;
    case CmdType_MouseInfo:
        AsyncReadMoseInfo();
        break;
    default:
        AsyncReadUnknownCmd();
        break;
    }
}

void TcpClient::AsyncReadSyncTime()
{
    boost::asio::async_read(
        *m_socketPtr, boost::asio::buffer(m_cmdSnycTime, sizeof(Cmd_SnycTime)),
        boost::bind(&TcpClient::OnSyncTimeReceived, shared_from_this(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void TcpClient::OnSyncTimeReceived(const boost::system::error_code& ec, size_t recvSize)
{
    if (ec)
    {
        if (!m_eventCb.empty())
        {
            m_eventCb(TCP_EVENT_READ_FAILED, ec.message());
        }

        if (!m_bConnected)
        {
            return;
        }

        m_bConnected = false;

        return;
    }

    //char msg[128];
    //sprintf(msg, "%d -> %lld", GetCurrentProcessId(), m_cmdSnycTime->time);
    //OutputDebugStringA(msg);


    AsyncReadHead();
}

void TcpClient::AsyncReadMoseInfo()
{
    boost::asio::async_read(
        *m_socketPtr, boost::asio::buffer(m_mouseInfo, m_cmdHeader->cmdLen),
        boost::bind(&TcpClient::OnMouseInfoReceived, shared_from_this(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void TcpClient::OnMouseInfoReceived(const boost::system::error_code& ec, size_t recvSize)
{
    if (ec)
    {
        if (!m_eventCb.empty())
        {
            m_eventCb(TCP_EVENT_READ_FAILED, ec.message());
        }

        if (!m_bConnected)
        {
            return;
        }

        m_bConnected = false;

        return;
    }

    char msg[128];
    sprintf(msg, "srv mouse info %d, %d", m_mouseInfo->x, m_mouseInfo->y);
    OutputDebugStringA(msg);

    AsyncReadHead();
}

void TcpClient::AsyncReadUnknownCmd()
{
    boost::asio::async_read(
        *m_socketPtr, boost::asio::buffer(m_UnknownCmd, m_cmdHeader->cmdLen),
        boost::bind(&TcpClient::OnUnknownCmdReceived, shared_from_this(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void TcpClient::OnUnknownCmdReceived(const boost::system::error_code& ec, size_t recvSize)
{
    if (ec)
    {
        X_PRINT("read error:%s", ec.message().c_str());
        if (!m_eventCb.empty())
        {
            m_eventCb(TCP_EVENT_READ_FAILED, ec.message());
        }

        if (!m_bConnected)
        {
            return;
        }

        m_bConnected = false;

        if (0)
        {
            // 不在这里关闭
            boost::system::error_code call_ec;
            if (m_socketPtr->is_open())
            {
                m_socketPtr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, call_ec);
                m_socketPtr->close(call_ec);
            }
        }

        return;
    }

    AsyncReadHead();
}
