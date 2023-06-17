#pragma once
#include "IQSocket.h"
#include "IQtestmachineThread.h"

class IQNetwork
{

};

typedef int (*AcceptFunc)(void* arg, IQSOCKET& client);
typedef int (*RecvFunc)(void* arg, const IQBuffer& buffer);
typedef int (*SendFunc)(void* arg, IQSOCKET& client, int ret);
typedef int (*RecvFromFunc)(void* arg, const IQBuffer& buffer, IQSockaddrIn addr);
typedef int (*SendToFunc)(void* arg, const IQSockaddrIn& addr, int ret);

class IQServerParameter
{
public:
	IQServerParameter(const std::string& ip = "0.0.0.0",
		short port = 9527, 
		IQSocket::IQTYPE type = IQSocket::IQTYPE::IQTypeTCP,
		AcceptFunc acceptf = NULL,
		RecvFunc recvf = NULL,
		SendFunc sendf = NULL,
		RecvFromFunc recvfromf = NULL,
		SendToFunc sendtof = NULL);
	IQServerParameter(const IQServerParameter& param);

	IQServerParameter& operator=(const IQServerParameter& param);

	//输入
	IQServerParameter& operator<<(AcceptFunc func);
	IQServerParameter& operator<<(RecvFunc func);
	IQServerParameter& operator<<(SendFunc func);
	IQServerParameter& operator<<(RecvFromFunc func);
	IQServerParameter& operator<<(SendToFunc func);
	IQServerParameter& operator<<(std::string& ip);
	IQServerParameter& operator<<(short port);
	IQServerParameter& operator<<(IQSocket::IQTYPE type);
	//输出
	IQServerParameter& operator>>(AcceptFunc& func);
	IQServerParameter& operator>>(RecvFunc& func);
	IQServerParameter& operator>>(SendFunc& func);
	IQServerParameter& operator>>(RecvFromFunc& func);
	IQServerParameter& operator>>(SendToFunc& func);
	IQServerParameter& operator>>(std::string& ip);
	IQServerParameter& operator>>(short& port);
	IQServerParameter& operator>>(IQSocket::IQTYPE& type);

	std::string m_ip;
	short m_port;
	IQSocket::IQTYPE m_type;
	AcceptFunc m_accept;
	RecvFunc m_recv;
	SendFunc m_send;
	RecvFromFunc m_recvfrom;
	SendToFunc m_sendto;
};

class IQServer : public ThreadFuncBase
{
public:
	IQServer(const IQServerParameter& param);//合适设置关键参数(比如本项目的IP与端口), 需要根据实际需求去调整
	~IQServer();
	int Invoke(void* arg);
	int Send(IQSOCKET& client, const IQBuffer& buffer);
	int Sendto(IQSockaddrIn& addr, const IQBuffer& buffer);
	int Stop();
private:
	int threadFunc();
	int threadUDPFunc();
	int threadTCPFunc();
private:
	IQServerParameter m_params;
	CIQtestmachineThread m_thread;
	void* m_args;
	IQSOCKET m_sock;
	std::atomic<bool> m_stop;
};