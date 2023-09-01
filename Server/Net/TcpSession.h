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
#include <vector>
#include <queue>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "CommonStructs.h"

class TcpSession : public boost::enable_shared_from_this<TcpSession>
{
public:
	TcpSession(boost::asio::ip::tcp::socket s);

	/// <summary>
	/// for test
	/// </summary>
	/// <param name="msg"></param>
	void write(std::string &msg);

	void write(void *cmd, int cmdLen);

	boost::system::error_code GetLastError();

private:
	/// <summary>
	/// �Ƚ���ͷ
	/// </summary>
	void AsyncReadHead();

	void OnHeaderReceived(const boost::system::error_code& ec, size_t recvSize);

	/// <summary>
	/// ������
	/// </summary>
	void AsyncReadMoseInfo();

	void OnMouseInfoReceived(const boost::system::error_code& ec, size_t recvSize);

	/// <summary>
	/// δ֪��һЩָ����Ǽ����Ժ�
	/// </summary>
	void AsyncReadUnknownCmd();

	void OnUnknownCmdReceived(const boost::system::error_code& ec, size_t recvSize);

public:
	boost::asio::ip::tcp::socket m_socket;

	boost::system::error_code m_lastErrorCode;

	std::vector<uint8_t> m_buffer;
	int m_bufferDataUsedLen = 0;
	Cmd_Header* m_cmdHeader;
	Cmd_MouseInfo* m_mouseInfo;

	void* m_UnknownCmd;
};

