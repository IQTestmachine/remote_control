#pragma once
#include "pch.h"
#include "framework.h"
#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) { }
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize)//服务端把要发送的信息打包成一个数据包
	{
		sHead = 0xFEFE;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0)
		{
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else
		{
			strData.clear();
		}
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	CPacket(const CPacket& pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize)//从客户端读取来的数据流中打包出一个数据包
	{
		size_t i = 0;
		for (; i < nSize; i++)
		{
			if (*(WORD*)(pData + i) == 0xFEFE)
			{
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize)// 数据包可能不全, 或包头未能全部接收到 
		{
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);
		i += 4;
		if (nLength + i > nSize)// 未能完全接收到数据包, 则返回, 解析失败
		{
			nSize = 0;
		}
		sCmd = *(WORD*)(pData + i);
		i += 2;
		if (nLength > 4)
		{
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum)
		{
			nSize = i;
			return;
		}
		else
		{
			nSize = 0;
		}
	}

	CPacket& operator=(const CPacket& pack)
	{
		if (this == &pack)
			return *this;
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
		return *this;
	}

	int Size()//数据包的大小
	{
		return nLength + 6;
	}
	const char* Data()
	{
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;
		pData += 2;
		*(DWORD*)pData = nLength;
		pData += 4;
		*(WORD*)pData = sCmd;
		pData += 2;
		memcpy(pData, strData.c_str(), strData.size());
		pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

	~CPacket() { }
public:
	WORD sHead;//包头, 固定位FE FF
	DWORD nLength;//包长度(从控制命令开始,到校验和结束)
	WORD sCmd;//控制命令
	std::string strData;//包数据
	WORD sSum;//和校验
	std::string strOut;// 整个数据包
};
#pragma pack(pop)

typedef struct MouseEvent
{
	MouseEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击, 移动, 双击
	WORD nButton;//左键, 右键, 中键
	POINT ptXY;//坐标
}MOUSEEV, * PMOUSEEV;

typedef struct file_info
{
	file_info()
	{
		IsInvalid = false;
		IsDirectory = -1;
		bool HasNext = true;
		memset(szFileName, 0, sizeof(szFileName));
	}
	bool IsInvalid;//路径是否有效
	bool IsDirectory;//是否为目录 0:否 1:是
	bool HasNext;//是否还有后续 0:否 1:是
	char szFileName[256];//文件名
}FILEINFO, * PFILEINFO;