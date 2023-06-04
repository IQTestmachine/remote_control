#pragma once
#include <Windows.h>
#include <string>
#include <atlimage.h>

class CIQtestmachineTool
{
public:
	static int Bytes2Image(CImage& image, const std::string& strData)//���ߺ���, ʵ�ֽ��ַ�������ת��ΪͼƬ����
	{
		BYTE* pData = (BYTE*)strData.c_str();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == nullptr)
		{
			TRACE("�ͻ����ڴ治��, �޷������ͼ!\r\n");
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

