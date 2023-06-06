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
	static size_t index = 0;//index��ʾm_buffer���ж��ٸ��ֽ�, ���ÿ�ε���recv()��CPacket(const BYTE* pData, size_t& nSize)�������index
	InitSocket();
	while (m_client != INVALID_SOCKET)
	{
		if (m_lstSend.size() > 0)
		{
			TRACE("lstSend size: %d\r\n", m_lstSend.size());
			CPacket& head = m_lstSend.front();
			if (Send(head) == false)
			{
				TRACE("�������ݰ�ʧ��\r\n");
				continue;
			}

			auto it = m_mapAck.find(head.hEvent);
			auto it0 = m_mapAutoClosed.find(head.hEvent);

			if (it != m_mapAck.end())
			{
				//if (it0->second == true)
				//{
				//	size_t len = recv(m_client, pbuffer + index, BUFFER_SIZE - index, 0);
				//	index += len;
				//	if (len > 0 || index > 0)
				//	{
				//		size_t tmp = index;
				//		CPacket pack((BYTE*)pbuffer, tmp);//tmp��ʾ��m_buffer��ȡ�������ݰ��ж��ٸ��ֽ�, ���tmp����0, �����δȡ���κ�����
				//		if (tmp > 0)//����TCP����, ��һ���ܴ�m_buffer�г�һ�����ݰ�, ��ʱ��Ҫ����ִ��ѭ��, ȥrecv()����
				//		{
				//			memmove(pbuffer, pbuffer + tmp, index - tmp);//����ȡ����һ�����ݰ�, �����Ҫ����m_buffer
				//			index -= tmp;
				//			pack.hEvent = head.hEvent;
				//			it->second.push_back(pack);
				//			SetEvent(it->first);
				//		}
				//	}
				//}
				//else
				//{
				//	while (true)
				//	{
				//		size_t len = recv(m_client, pbuffer + index, BUFFER_SIZE - index, 0);
				//		index += len;
				//		if (len > 0 || index > 0)
				//		{
				//			size_t tmp = index;
				//			CPacket pack((BYTE*)pbuffer, tmp);//tmp��ʾ��m_buffer��ȡ�������ݰ��ж��ٸ��ֽ�, ���tmp����0, �����δȡ���κ�����
				//			if (tmp > 0)//����TCP����, ��һ���ܴ�m_buffer�г�һ�����ݰ�, ��ʱ��Ҫ����ִ��ѭ��, ȥrecv()����
				//			{
				//				memmove(pbuffer, pbuffer + tmp, index - tmp);//����ȡ����һ�����ݰ�, �����Ҫ����m_buffer
				//				index -= tmp;
				//				pack.hEvent = head.hEvent;
				//				it->second.push_back(pack);
				//			}
				//		}
				//		else
				//			break;
				//	}
				//	SetEvent(it->first);
				//}
				do {
					size_t len = recv(m_client, pbuffer + index, BUFFER_SIZE - index, 0);
					index += len;
					if (len > 0 || index > 0)
					{
						size_t tmp = index;
						CPacket pack((BYTE*)pbuffer, tmp);//tmp��ʾ��m_buffer��ȡ�������ݰ��ж��ٸ��ֽ�, ���tmp����0, �����δȡ���κ�����
						if (tmp > 0)//����TCP����, ��һ���ܴ�m_buffer�г�һ�����ݰ�, ��ʱ��Ҫ����ִ��ѭ��, ȥrecv()����
						{
							memmove(pbuffer, pbuffer + tmp, index - tmp);//����ȡ����һ�����ݰ�, �����Ҫ����m_buffer
							index -= tmp;
							pack.hEvent = head.hEvent;
							it->second.push_back(pack);
						}
					}
					else
						break;
				} while (it0->second == false);
				SetEvent(it->first);
			}
			m_lstSend.pop_front();
			CloseSocket();
			//InitSocket();
		}
	}
	CloseSocket();
}
