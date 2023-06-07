#include "pch.h"
#include "CClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket::Helper CClientSocket::m_helper;
std::map<UINT, CClientSocket::MSGFUNC> CClientSocket::m_mapFunc;

CClientSocket* pserver = CClientSocket::getInstance();

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
	if (m_hThread == INVALID_HANDLE_VALUE)
		m_hThread = (HANDLE)_beginthreadex(nullptr, 0, &CClientSocket::threadEntry, this, 0, &m_nThreadID);
	UINT nMode = isAutoClosed ? CM_AUTOCLOSED : 0;
	std::string strOut;
	pack.Data(strOut);
	return PostThreadMessage(m_nThreadID, WM_SEND_PACK, (WPARAM)new PACKET_DATA(strOut.c_str(), strOut.size(), nMode, wParam), (LPARAM)hWnd);
}

//bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosed)
//{
//	
//	if (m_client == INVALID_SOCKET && m_hThread == INVALID_HANDLE_VALUE)
//	{
//		m_hThread = (HANDLE)_beginthread(&CClientSocket::threadEntry, 0, this);
//	}
//	m_lock.lock();
//	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, lstPacks));
//	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));
//	m_lstSend.push_back(pack);
//	m_lock.unlock();
//	WaitForSingleObject(pack.hEvent, INFINITE);
//	std::map<HANDLE, std::list<CPacket>&>::iterator it;
//	it = m_mapAck.find(pack.hEvent);
//	if (it != m_mapAck.end())
//	{
//		m_mapAck.erase(it);
//		return true;
//	}
//	return false;
//}

unsigned CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc2();
	_endthread();
	return 0;
}

//void CClientSocket::threadFunc()
//{
//	std::string strBuffer;
//	strBuffer.resize(BUFFER_SIZE);
//	char* pbuffer = (char*)strBuffer.c_str();
//	static size_t index = 0;//index表示m_buffer中有多少个字节, 因此每次调用recv()和CPacket(const BYTE* pData, size_t& nSize)都需调整index
//	InitSocket();
//	while (m_client != INVALID_SOCKET)
//	{
//		if (m_lstSend.size() > 0)
//		{
//			//TRACE("lstSend size: %d\r\n", m_lstSend.size());
//			m_lock.lock();
//			CPacket& head = m_lstSend.front();
//			m_lock.unlock();
//			if (Send(head) == false)
//			{
//				TRACE("发送数据包失败\r\n");
//				continue;
//			}
//
//			auto it = m_mapAck.find(head.hEvent);
//			auto it0 = m_mapAutoClosed.find(head.hEvent);
//
//			if (it != m_mapAck.end() && it0 != m_mapAutoClosed.end())
//			{
//				//if (it0->second == true)
//				//{
//				//	size_t len = recv(m_client, pbuffer + index, BUFFER_SIZE - index, 0);
//				//	index += len;
//				//	if (len > 0 || index > 0)
//				//	{
//				//		size_t tmp = index;
//				//		CPacket pack((BYTE*)pbuffer, tmp);//tmp表示从m_buffer中取出的数据包有多少个字节, 如果tmp等于0, 则代表并未取出任何数据
//				//		if (tmp > 0)//采用TCP连接, 不一定能从m_buffer中出一个数据包, 此时就要继续执行循环, 去recv()数据
//				//		{
//				//			memmove(pbuffer, pbuffer + tmp, index - tmp);//由于取出了一个数据包, 因此需要调整m_buffer
//				//			index -= tmp;
//				//			pack.hEvent = head.hEvent;
//				//			it->second.push_back(pack);
//				//			SetEvent(it->first);
//				//		}
//				//	}
//				//}
//				//else
//				//{
//				//	while (true)
//				//	{
//				//		size_t len = recv(m_client, pbuffer + index, BUFFER_SIZE - index, 0);
//				//		index += len;
//				//		if (len > 0 || index > 0)
//				//		{
//				//			size_t tmp = index;
//				//			CPacket pack((BYTE*)pbuffer, tmp);//tmp表示从m_buffer中取出的数据包有多少个字节, 如果tmp等于0, 则代表并未取出任何数据
//				//			if (tmp > 0)//采用TCP连接, 不一定能从m_buffer中出一个数据包, 此时就要继续执行循环, 去recv()数据
//				//			{
//				//				memmove(pbuffer, pbuffer + tmp, index - tmp);//由于取出了一个数据包, 因此需要调整m_buffer
//				//				index -= tmp;
//				//				pack.hEvent = head.hEvent;
//				//				it->second.push_back(pack);
//				//			}
//				//		}
//				//		else
//				//			break;
//				//	}
//				//	SetEvent(it->first);
//				//}
//				do {
//					size_t len = recv(m_client, pbuffer + index, BUFFER_SIZE - index, 0);
//					index += len;
//					if (len > 0 || index > 0)
//					{
//						size_t tmp = index;
//						CPacket pack((BYTE*)pbuffer, tmp);//tmp表示从m_buffer中取出的数据包有多少个字节, 如果tmp等于0, 则代表并未取出任何数据
//						if (tmp > 0)//采用TCP连接, 不一定能从m_buffer中出一个数据包, 此时就要继续执行循环, 去recv()数据
//						{
//							memmove(pbuffer, pbuffer + tmp, index - tmp);//由于取出了一个数据包, 因此需要调整m_buffer
//							index -= tmp;
//							pack.hEvent = head.hEvent;
//							it->second.push_back(pack);
//						}
//					}
//					else
//						break;
//				} while (it0->second == false);
//				SetEvent(head.hEvent);
//				if (it0 != m_mapAutoClosed.end())
//					m_mapAutoClosed.erase(head.hEvent);
//			}
//			m_lock.lock();
//			m_lstSend.pop_front();
//			m_lock.unlock();
//			InitSocket();
//		}
//		Sleep(1);//如果不休眠1ms, 监控屏幕的功能会卡死
//	}
//	CloseSocket();
//}

void CClientSocket::threadFunc2()
{
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

void CClientSocket::SendPack(UINT msg, WPARAM wParam, LPARAM lParam)
{
	PACKET_DATA data = *(PACKET_DATA*)wParam;
	delete (PACKET_DATA*)wParam;
	HWND hWnd = (HWND)lParam;
	if (InitSocket() == true)
	{
		int ret = send(m_client, (char*)data.strData.c_str(), (int)data.nMode, 0);
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
						::SendMessage(hWnd, WM_SEND_PACK_ACK, (WPARAM)new CPacket(pack), data.wParam);
						if (data.nMode & CM_AUTOCLOSED)
						{
							CloseSocket();
							TRACE("成功接收服务端执行命令号: %d发送的一个数据包", pack.sCmd);
							return;
						}

					}
					index -= tmp;
					memmove(pBuffer, pBuffer + index, tmp);
				}
				else
				{
					CloseSocket();
					::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, 1);
					TRACE("成功接收服务端执行命令发送的全部数据包");
				}
			}
		}
		else
		{
			CloseSocket();
			::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -1);
			TRACE("发送命令失败");
		}
			
	}
	else
	{
		//TODO: 错误处理
		::SendMessage(hWnd, WM_SEND_PACK_ACK, NULL, -2);
		TRACE("连接服务端失败");
	}
}
