#include "pch.h"
#include "CClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket::Helper CClientSocket::m_helper;

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

void CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();
	_endthread();
}

void CClientSocket::threadFunc()
{
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pbuffer = (char*)strBuffer.c_str();
	static size_t index = 0;//index表示m_buffer中有多少个字节, 因此每次调用recv()和CPacket(const BYTE* pData, size_t& nSize)都需调整index
	while (m_client != INVALID_SOCKET)
	{
		if (m_lstSend.size() > 0)
		{
			TRACE("lstSend size: %d\r\n", m_lstSend.size());
			CPacket& head = m_lstSend.front();
			if (Send(head) == false)
			{
				TRACE("发送数据包失败\r\n");
				continue;
			}
				

			auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(head.hEvent, std::list<CPacket>()));
			size_t len = recv(m_client, pbuffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0 && index <= 0)
			{
				CloseSocket();
				//return;
			}
			index += len;
			size_t tmp = index;
			CPacket pack((BYTE*)pbuffer, tmp);//tmp表示从m_buffer中取出的数据包有多少个字节, 如果tmp等于0, 则代表并未取出任何数据
			//TRACE("解包的长度是%lld %d\r\n", *(long long*)m_packet.strData.c_str(), m_packet.nLength);
			if (tmp > 0)//采用TCP连接, 不一定能从m_buffer中出一个数据包, 此时就要继续执行循环, 去recv()数据
			{
				memmove(pbuffer, pbuffer + tmp, index - tmp);//由于取出了一个数据包, 因此需要调整m_buffer
				index -= tmp;
				pack.hEvent = head.hEvent;
				pr.first->second.push_back(pack);
				SetEvent(head.hEvent);
			}
			m_lstSend.pop_front();
		}
	}
}
