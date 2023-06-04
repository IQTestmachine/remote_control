#pragma once
#include <Windows.h>
#include <string>
#include <atlimage.h>

class CIQtestmachineTool
{
public:
	static int Bytes2Image(CImage& image, const std::string& strData)//工具函数, 实现将字符串数据转换为图片数据
	{
		BYTE* pData = (BYTE*)strData.c_str();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == nullptr)
		{
			TRACE("客户端内存不足, 无法缓存截图!\r\n");
			Sleep(1);
			return -1;
		}
		IStream* pStream = nullptr;
		HRESULT ret = CreateStreamOnHGlobal(hMem, true, &pStream);
		if (ret == S_OK)
		{
			ULONG length = 0;
			pStream->Write(pData, strData.size(), &length);
			//TRACE("length = %d", length);
			LARGE_INTEGER bg = { 0 };
			pStream->Seek(bg, STREAM_SEEK_SET, nullptr);
			if ((HBITMAP)image != nullptr)
				image.Destroy();
			image.Load(pStream);
		}
		return ret;
	}
};

