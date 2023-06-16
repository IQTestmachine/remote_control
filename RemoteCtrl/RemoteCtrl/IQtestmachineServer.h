#pragma once
#include <MSWSock.h>
#include "IQtestmachineThread.h"
#include "IQtestmachineQueue.h"
#include "IQtestmachineTool.h"
#include <map>
#include <vector>
#include <memory>
#pragma warning(disable : 4996)

enum IQtestmachineOperator{
    IQNone,
    IQAccept,
    IQRecv,
    IQSend,
    IQError
};

class IQtestmachineOverlapped;
class IQtestmachineClient;
class CIQtestmachineServer;
template<IQtestmachineOperator> class AcceptOverlapped;
typedef AcceptOverlapped<IQAccept> ACCEPTOVERLAPPED;
typedef std::shared_ptr<IQtestmachineClient> PCLIENT;
template<IQtestmachineOperator> class RecvOverlapped;
typedef RecvOverlapped<IQRecv> RECVOVERLAPPED;
template<IQtestmachineOperator> class SendOverlapped;
typedef SendOverlapped<IQSend> SENDOVERLAPPED;

class IQtestmachineOverlapped
{
public:
    OVERLAPPED m_overlapped;
    DWORD m_operator;//操作 参见IQtestmachineOperator
    std::vector<char> m_buffer;//缓冲区
    CThreadWorker m_worker;//处理函数
    CIQtestmachineServer* m_serv;//服务端对象
    IQtestmachineClient* m_clnt;//对应的客户端
    WSABUF m_wsabuffer;
    virtual ~IQtestmachineOverlapped()
    { 
        m_buffer.clear();
    }

};

class IQtestmachineClient : public ThreadFuncBase
{
public:
    IQtestmachineClient();
    ~IQtestmachineClient()
    {
        m_buffer.clear();
        closesocket(m_client);
        m_recv.reset();
        m_send.reset();
        m_overlapped.reset();
        m_vecSend.Clear();
       
    }

    void  SetOverlapped(PCLIENT& ptr);

    operator SOCKET()
    {
        return m_client;
    }
    operator PVOID()
    {
        return &m_buffer[0];
    }
    operator LPOVERLAPPED();
    operator LPDWORD()
    {
        return &m_received;
    }
    LPWSABUF RecvWSABuffer();
    LPWSAOVERLAPPED RecvOverlapped();
    LPWSABUF SendWSABuffer();
    LPWSAOVERLAPPED SendOverlapped();

    sockaddr_in* GetLocalAddr()
    {
        return &m_laddr;
    }
    sockaddr_in* GetRemoteAddr()
    {
        return &m_raddr;
    }
    size_t GetBufferSize() const
    {
        return m_buffer.size();
    }
    DWORD& flags()
    {
        return m_flags;
    }

    int Recv();

    int Send(void* buffer, size_t nSize);
    int SendData(std::vector<char>& data);
private:
    SOCKET m_client;
    DWORD m_received;
    DWORD m_flags;
    std::shared_ptr<ACCEPTOVERLAPPED> m_overlapped;
    std::shared_ptr<RECVOVERLAPPED> m_recv;
    std::shared_ptr<SENDOVERLAPPED> m_send;
    std::vector<char> m_buffer;
    size_t m_used;//已经使用的缓冲区大小
    sockaddr_in m_laddr;//本地sockaddr_in
    sockaddr_in m_raddr;//远程sockaddr_in
    bool m_isbusy;
    IQtestmachineSendQueue<std::vector<char>> m_vecSend;//发送数据的线程安全队列
};

template<IQtestmachineOperator>
class AcceptOverlapped : public IQtestmachineOverlapped, ThreadFuncBase
{
public:
    AcceptOverlapped(); 
    int AcceptWorker();
};

template<IQtestmachineOperator>
class RecvOverlapped : public IQtestmachineOverlapped, ThreadFuncBase
{
public:
    RecvOverlapped(); 
    int RecvWorker()
    {
        int ret = m_clnt->Recv();
        return ret;
    }
};

template<IQtestmachineOperator>
class SendOverlapped : public IQtestmachineOverlapped, ThreadFuncBase
{
public:
    SendOverlapped(); 
    int SendWorker()
    {
        //TODO:1.Send可能不会立即完成
        //2.
        return -1;
    }
};

template<IQtestmachineOperator>
class ErrorOverlapped : public IQtestmachineOverlapped, ThreadFuncBase
{
public:
    ErrorOverlapped() : m_operator(IQAccept), m_worker(this, &ErrorOverlapped::AcceeptWorker)
    {
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        m_buffer.resize(1024);
    }
    int ErrorWorker()
    {
        //TODO:
        return -1;
    }
};
typedef ErrorOverlapped<IQError> ERROROVERLAPPED;

class CIQtestmachineServer : public ThreadFuncBase
{
public:
    CIQtestmachineServer(const std::string& ip = "127.0.0.1", short port = 9527) : m_pool(5)
    {
        m_hIOCP = INVALID_HANDLE_VALUE;
        m_server = INVALID_SOCKET;
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }

    ~CIQtestmachineServer();

    bool StartServer();
    bool NewAccept();
    bool BindNewSocket(SOCKET s);
private:
    void CreateSocket();
    int threadIocp();
private:
    IQtestmachinePool m_pool;
    HANDLE m_hIOCP;
    SOCKET m_server;
    sockaddr_in m_addr;
    std::map<SOCKET, std::shared_ptr<IQtestmachineClient>> m_client;
};

