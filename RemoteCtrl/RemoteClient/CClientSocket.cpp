#include "pch.h"
#include "CClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket::Helper CClientSocket::m_helper;

CClientSocket* pclient = CClientSocket::getInstance();

std::string GetErrorInfo(int wsaErrCode)
{
	std::string ret;
	LPVOID lpMsgBuf = nullptr;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		nullptr,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, nullptr);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}

bool CClientSocket::SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed, WPARAM wParam)
{
	UINT nMode = isAutoClosed ? CM_AUTOCLOSED : 0;
	std::string strOut;
	pack.Data(strOut);
	PACKET_DATA* packet_data = new PACKET_DATA(strOut.c_str(), strOut.size(), nMode, wParam);
	int ret = PostThreadMessage(m_nThreadID, WM_SEND_PACK, (WPARAM)packet_data, (LPARAM)hWnd);	
	return ret;
}

//该线程在客户端套接字创建时即启动, 专门用于处理客户端UI界面发送的各种消息
unsigned CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc2();
	_endthread();
	return 0;
}

void CClientSocket::threadFunc2()
{
	SetEvent(m_eventInvoke);
	MSG msg;
	while (::GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (m_mapFunc.find(msg.message) != m_mapFunc.end())
		{
			(this->*m_mapFunc[msg.message])(msg.message, msg.wParam, msg.lParam);
		}
	}
}

//该函数调用套接字的send和recv函数，实现了客户端命令包的发送以及接收服务端处理一个相应命令的发送的全部数据包
void CClientSocket::SendPack(UINT msg, WPARAM wParam, LPARAM lParam)
{
	PACKET_DATA data = *(PACKET_DATA*)wParam;
	delete (PACKET_DATA*)wParam;
	HWND hWnd = (HWND)lParam;
	if (InitSocket() == true)
	{
		int ret = send(m_client, (char*)data.strData.c_str(), data.strData.size(), 0);
		if (ret > 0)
		{
			size_t index = 0;
			std::string strBuffer;
			strBuffer.resize(BUFFER_SIZE);
			char* pBuffer = (char*)strBuffer.c_str();//这里采用了RAII思想
			while (m_client != INVALID_SOCKET)
			{
				size_t len = recv(m_client, pBuffer + index, BUFFER_SIZE - index, 0);
				if (len > 0 || index > 0)
				{
					index += len;
					size_t tmp = index;
					CPacket pack((BYTE*)pBuffer, tmp);
					if (tmp > 0)
					{
						CPacket* pack_ack = new CPacket(pack);
						::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)pack_ack, data.wParam);
						if (GetLastError() > 0)
							TRACE("错误码 %d(未能成功投递消息)\r\n)", GetLastError());
						/*if (ret > 0)
							delete pack_ack;*/
						if (data.nMode == CM_AUTOCLOSED)
						{
							CloseSocket();
							//TRACE("成功接收服务端执行命令号: %d发送的一个数据包\r\n", pack.sCmd);
							break;
						}

					}
					index -= tmp;
					memmove(pBuffer, pBuffer + tmp, index);
				}
				else
				{
					CloseSocket();
					::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, 1);
					break;
					//TRACE("成功接收服务端执行命令发送的全部数据包\r\n");
				}
			}
		}
		else
		{
			CloseSocket();
			::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -1);
			//TRACE("发送命令失败\r\n");
		}
			
	}
	else
	{
		::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -2);
		//TRACE("连接服务端失败\r\n");
	}
}
