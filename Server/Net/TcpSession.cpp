#include "TcpSession.h"

#define SMS_BUFFER_SIZE                 1024 * 1024 * 4

TcpSession::TcpSession(boost::asio::ip::tcp::socket s) :
	m_socket(std::move(s))
	, m_buffer(SMS_BUFFER_SIZE)
{
	m_cmdHeader = (Cmd_Header*)m_buffer.data();
	m_bufferDataUsedLen += sizeof(Cmd_Header);

	m_mouseInfo = (Cmd_MouseInfo*)(m_buffer.data() + m_bufferDataUsedLen);
	m_bufferDataUsedLen += sizeof(Cmd_MouseInfo);

	m_UnknownCmd = (void*)(m_buffer.data() + m_bufferDataUsedLen);

	AsyncReadHead();
}

void TcpSession::AsyncReadHead()
{
	if (m_lastErrorCode)
	{
		return;
	}

	// auto self(shared_from_this());
	boost::asio::async_read(
		m_socket, boost::asio::buffer(m_cmdHeader, sizeof(Cmd_Header)),
		boost::bind(&TcpSession::OnHeaderReceived, this,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void TcpSession::OnHeaderReceived(const boost::system::error_code& ec, size_t recvSize)
{
	if (ec)
	{
		m_lastErrorCode = ec;
		return;
	}

	switch (m_cmdHeader->cmdType)
	{
	case CmdType_HeartBeat:
		OutputDebugStringA("heart ...");
		AsyncReadHead();
		break;
	case CmdType_MouseInfo:
		AsyncReadMoseInfo();
		break;
	default:
		AsyncReadUnknownCmd();
		break;
	}
}

void TcpSession::AsyncReadMoseInfo()
{
	boost::asio::async_read(
		m_socket, boost::asio::buffer(m_mouseInfo, m_cmdHeader->cmdLen),
		boost::bind(&TcpSession::OnMouseInfoReceived, this,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void TcpSession::OnMouseInfoReceived(const boost::system::error_code& ec, size_t recvSize)
{
	if (ec)
	{
		m_lastErrorCode = ec;
		return;
	}

	char msg[128];
	sprintf(msg, "client mouse info %d, %d", m_mouseInfo->x, m_mouseInfo->y);
	OutputDebugStringA(msg);

	AsyncReadHead();
}


void TcpSession::AsyncReadUnknownCmd()
{
	boost::asio::async_read(
		m_socket, boost::asio::buffer(m_UnknownCmd, m_cmdHeader->cmdLen),
		boost::bind(&TcpSession::OnUnknownCmdReceived, this,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void TcpSession::OnUnknownCmdReceived(const boost::system::error_code& ec, size_t recvSize)
{
	if (ec)
	{
		m_lastErrorCode = ec;
		return;
	}

	AsyncReadHead();
}





void TcpSession::write(std::string& msg)
{
	if (m_lastErrorCode)
	{
		return;
	}

	boost::asio::streambuf out_stream_;
	boost::asio::const_buffers_1;

	auto self(shared_from_this());
	boost::asio::async_write(
		m_socket, boost::asio::buffer(msg.c_str(), msg.size()),
		[this, self](const boost::system::error_code& ec, size_t sendsize)
		{
			if (!ec)
			{
				// std::cout << "send " << sendsize << " byte success." << std::endl;
			}
			else
			{
				// std::cout << "send fail." << std::endl;
				m_lastErrorCode = ec;
				
				/*int ecVal = ec.value();
				switch (ec.value())
				{
				case boost::asio::error::eof:
					return;

				case boost::asio::error::connection_reset:
				case boost::asio::error::connection_aborted:
				case boost::asio::error::access_denied:
				case boost::asio::error::address_family_not_supported:
				case boost::asio::error::address_in_use:
				case boost::asio::error::already_connected:
				case boost::asio::error::connection_refused:
				case boost::asio::error::bad_descriptor:
				case boost::asio::error::fault:
				case boost::asio::error::host_unreachable:
				case boost::asio::error::in_progress:
				case boost::asio::error::interrupted:
				case boost::asio::error::invalid_argument:
				case boost::asio::error::message_size:
				case boost::asio::error::name_too_long:
				case boost::asio::error::network_down:
				case boost::asio::error::network_reset:
				case boost::asio::error::network_unreachable:
				case boost::asio::error::no_descriptors:
				case boost::asio::error::no_buffer_space:
				case boost::asio::error::no_protocol_option:
				case boost::asio::error::not_connected:
				case boost::asio::error::not_socket:
				case boost::asio::error::operation_not_supported:
				case boost::asio::error::shut_down:
				case boost::asio::error::timed_out:
				case boost::asio::error::would_block:
					break;
				default:
					break;
				}*/
			}

		}
	);
}

void TcpSession::write(void* cmd, int cmdLen)
{
	if (m_lastErrorCode)
	{
		return;
	}

	auto self(shared_from_this());
	boost::asio::async_write(
		m_socket, boost::asio::buffer(cmd, cmdLen),
		[this, self](const boost::system::error_code& ec, size_t sendsize)
		{
			if (!ec)
			{
				// std::cout << "send " << sendsize << " byte success." << std::endl;
			}
			else
			{
				// std::cout << "send fail." << std::endl;
				m_lastErrorCode = ec;
			}

		}
	);
}

boost::system::error_code TcpSession::GetLastError()
{
	return m_lastErrorCode;
}