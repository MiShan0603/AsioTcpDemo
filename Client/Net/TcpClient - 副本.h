#pragma once

#pragma warning(disable:4996)

#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/thread.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/function.hpp>
#include <vector>
#include <queue>
#include <boost/date_time/posix_time/posix_time.hpp> //定时器

#include "Logdef.h"
#include "CommonStructs.h"

using namespace std;

enum TCP_CLIENT_EVENT
{
    TCP_EVENT_CONNECTED = 0,
    TCP_EVENT_CONNECT_FAILED,
    TCP_EVENT_WRITE_FAILED,
    TCP_EVENT_READ_FAILED
};

typedef boost::function<void(TCP_CLIENT_EVENT, string)> TcpClientEventCallBack;

class TcpClient : public boost::enable_shared_from_this<TcpClient>
{
    typedef boost::asio::ip::tcp::endpoint endpoint_type;
    typedef boost::asio::ip::address address_type;
    typedef boost::asio::ip::tcp::socket socket_type;
    typedef boost::shared_ptr<socket_type> sock_ptr;
    typedef boost::asio::io_service io_service_type;

public:
    TcpClient();
    ~TcpClient();

    /// <summary>
    /// 设置事件回调函数，事件类型见TCP_EVENT
    /// </summary>
    /// <param name="cb"></param>
    void SetEventCallBack(TcpClientEventCallBack cb);

    /// <summary>
    /// 异步发起连接，连接结果在事件回调时通知 成功时:TCP_EVENT_CONNECTED 失败时:TCP_EVENT_CONNECT_FAILED
    /// </summary>
    /// <param name="ip"></param>
    /// <param name="port"></param>
    /// <returns></returns>
    bool AsyncConnect(string ip, unsigned short port);

private:
    /// <summary>
    /// 连接回调
    /// </summary>
    /// <param name="ec"></param>
    void OnAsyncConnect(const boost::system::error_code& ec);

public:
    /// <summary>
    /// 发送数据
    /// </summary>
    void write(void* cmd, int cmdLen);

    // 关闭连接
    bool Close();

private:
    /// <summary>
    /// 先接收头
    /// </summary>
    void AsyncReadHead();

    void OnHeaderReceived(const boost::system::error_code& ec, size_t recvSize);

    /// <summary>
    /// 接收命令体
    /// </summary>
    void AsyncReadSyncTime();

    void OnSyncTimeReceived(const boost::system::error_code& ec, size_t recvSize);

    /// <summary>
    /// 鼠标测试
    /// </summary>
    void AsyncReadMoseInfo();

    void OnMouseInfoReceived(const boost::system::error_code& ec, size_t recvSize);


    /// <summary>
    /// 未知的一些指令，考虑兼容以后
    /// </summary>
    void AsyncReadUnknownCmd();

    void OnUnknownCmdReceived(const boost::system::error_code& ec, size_t recvSize);

private:
    boost::asio::io_service m_ioservice;
    boost::shared_ptr<boost::asio::io_service::work> m_dummy_work;
        
    endpoint_type m_endPoint;
    sock_ptr m_socketPtr;            // 用于普通的TCP通信

    /// <summary>
    /// for boost::asio::io_service::run()
    /// </summary>
    std::shared_ptr<std::thread> m_workerThread = nullptr;

    bool m_bConnected; // 是否成功建立连接

    // 回调函数
    TcpClientEventCallBack m_eventCb; // 事件回调


    /// <summary>
    /// 使用 timer 发测试数据，测试学习发送
    /// </summary>
    std::shared_ptr< boost::asio::steady_timer> m_heartTimer = nullptr;
      

    /// <summary>
    /// 测试发送心跳线程
    /// </summary>
    void heartbeat();

private:
    std::vector<uint8_t> m_buffer;
    int m_bufferDataUsedLen = 0;

    Cmd_Header *m_cmdHeader;
    Cmd_SnycTime *m_cmdSnycTime;
    Cmd_MouseInfo* m_mouseInfo;
    void* m_UnknownCmd;
};

typedef boost::shared_ptr<TcpClient> TcpClientPtr;


