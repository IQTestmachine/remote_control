#pragma once
#include "Resource.h"
#include <atlimage.h>//用于发送屏幕截图的头文件
#include <direct.h>
#include "Packet.h"
#include <map>
#include <list>
#include "CServerSocket.h"
#include "IQtestmachineTool.h"
#include "LockDialog.h"
#include <io.h>
//#include <list>

class CCommand
{
public:
	CCommand();
	~CCommand() { }
	int ExcuteCommand(int nCmd, std::list<CPacket>& lstPackets, CPacket& inPacket);
    static void RunCommand(void* arg, int status, std::list<CPacket>& lstPackets, CPacket& inPacket)
    {
		CCommand* thiz = (CCommand*)arg;
		if (status > 0)
		{
			int ret = thiz->ExcuteCommand(status, lstPackets, inPacket);
			if (ret != 0)
			{
				TRACE("执行命令失败: %d ret = %d\r\n", status, ret);
			}
		}
		else
		{
			MessageBox(nullptr, _T("无法正常接入用户, 自动重试..."), _T("接入用户失败"), MB_OK | MB_ICONERROR);
		}
    }
protected:
	typedef int (CCommand::* CMDFUNC)(std::list<CPacket>&, CPacket& inPacket);//成员函数指针
	std::map<int, CMDFUNC> m_mapFunction;//从命令号到功能的映射
    CLockDialog dlg;
    unsigned threadid;
protected:
    static unsigned __stdcall threadLockDlg(void* arg)//线程处理函数
    {
        CCommand* thiz = (CCommand*)arg;
        thiz->threadLockDlgMain();
        _endthreadex(0);
        return 0;
    }

    void threadLockDlgMain()
    {
        TRACE("%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
        dlg.Create(IDD_DIALOG_INFO, nullptr);
        dlg.ShowWindow(SW_SHOW);//非模态
        CRect rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
        rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
        dlg.MoveWindow(rect);//遮蔽后台窗口
        CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
        if (pText)
        {
            CRect rtText;
            pText->GetWindowRect(rtText);
            int nWidth = rtText.Width();
            int x = (rect.right - nWidth) / 2;
            int nHeight = rtText.Height();
            int y = (rect.bottom - nHeight) / 2;
            pText->MoveWindow(x, y, nWidth, nHeight);
        }
        dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//窗口置顶, 对话框置于图层最前方
        ShowCursor(false);//限制鼠标, 使其在对话框内消失
        ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), nullptr), SW_HIDE);//隐藏任务栏
        dlg.GetWindowRect(rect);//限制鼠标活动范围
        rect.left = 0;
        rect.top = 0;
        rect.right = 1;
        rect.bottom = 1;
        ClipCursor(rect);//限制鼠标的活动范围, 此时限定为只能在一个点
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_KEYDOWN)
            {
                TRACE("msg:%08X wparam:%08X lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
                if (msg.wParam == 0x1B)//按下esc退出
                {
                    break;
                }
            }
        }
        ClipCursor(nullptr);
        ShowCursor(true);
        ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), nullptr), SW_SHOW);//恢复任务栏
        dlg.DestroyWindow();
    }

    int MakeDriverInfo(std::list<CPacket>& lstPackets, CPacket& inPacket) // 获取磁盘分区
    {
        std::string result;
        for (int i = 1; i <= 26; i++)
        {
            if (_chdrive(i) == 0)//1==>A 2==>B 3==>C ... 26==>Z)
            {
                if (result.size() > 0)
                    result += ',';
                result += 'A' + i - 1;
            }
        }
        lstPackets.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
        //CPacket pack(1, (BYTE*)result.c_str(), result.size());//生成磁盘分区的数据包
        //CIQtestmachineTool::Dump((BYTE*)pack.Data(), pack.Size());
        //CServerSocket::getInstance()->Send(pack);
        return 0;
    }

    int MakeDirectoryInfo(std::list<CPacket>& lstPackets, CPacket& inPacket)
    {
        std::string strPath = inPacket.strData;
        //std::list<FILEINFO> lstFileInfos;

        /*if (CServerSocket::getInstance()->GetFilePath(strPath) == false)
        {
            OutputDebugString(_T("当前命令, 不是获取文件列表, 命令解析错误!"));
            return -1;
        }*/
        if (_chdir(strPath.c_str()) != 0)
        {
            FILEINFO finfo;
            finfo.HasNext = false;
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            lstPackets.push_back(pack);
            OutputDebugString(_T("文件路径无效或没有权限访问目录!!!"));
            return -2;
        }
        _finddata_t fdata;
        int hfind = 0;
        if ((hfind = _findfirst("*", &fdata)) == -1)
        {
            OutputDebugString(_T("指定路径下没有任何文件!!!"));
            FILEINFO finfo;
            finfo.HasNext = false;
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            lstPackets.push_back(pack);
            return -3;
        }
        int counts = 0;//服务端发送了多少个文件夹和文件
        do {
            FILEINFO finfo;
            finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
            memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
            //TRACE("%s\r\n", finfo.szFileName);
            CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
            lstPackets.push_back(pack);
            //ServerSocket::getInstance()->Send(pack);//拿到一个文件就发送信息到控制端
            //lstFileInfos.push_back(finfo); 采用链表获取全部文件和文件夹,可能因文件和文件夹太多导致迟迟不能发送, 舍弃链表的方式
            counts++;
        } while (!_findnext(hfind, &fdata));

        //TRACE("server: counts = %d\r\n", counts);
        FILEINFO finfo;
        finfo.HasNext = false;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        lstPackets.push_back(pack);
        return 0;
    }

    int RunFile(std::list<CPacket>& lstPackets, CPacket& inPacket)
    {
        std::string strPath = inPacket.strData;
        ShellExecuteA(nullptr, nullptr, strPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        CPacket pack(3, nullptr, 0);
        lstPackets.push_back(pack);
        return 0;
    }

    int DownloadFile(std::list<CPacket>& lstPackets, CPacket& inPacket)
    {
        std::string strPath = inPacket.strData;
        //TRACE(("服务端即将传输的文件路径是%s\r\n"), strPath);
        long long data = 0;
        FILE* pFile = nullptr;
        errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
        if (err != 0)
        {
            CPacket pack(4, (BYTE*)&data, 0);
            lstPackets.push_back(pack);
            return -1;
        }
        if (pFile != nullptr)
        {
            fseek(pFile, 0, SEEK_END);
            data = _ftelli64(pFile);
            CPacket head(4, (BYTE*)&data, 8);
            //TRACE("服务端即将发送的文件的长度是%lld\r\n", *(long long*)head.strData.c_str());
            lstPackets.push_back(head);
            fseek(pFile, 0, SEEK_SET);
            char buffer[1024] = "";
            size_t rlen = 0;
            do {
                rlen = fread(buffer, 1, 1024, pFile);
                CPacket pack(4, (BYTE*)buffer, rlen);
                lstPackets.push_back(pack);
            } while (rlen >= 1024);
            fclose(pFile);
        }
        CPacket pack(4, nullptr, 0);
        lstPackets.push_back(pack);

        return 0;
    }

    int MouseEvent(std::list<CPacket>& lstPackets, CPacket& inPacket)
    {
        MOUSEEV mouse;
        memcpy(&mouse, inPacket.strData.c_str(), sizeof(MOUSEEV));
		DWORD nFlags = 0;
		switch (mouse.nButton)
		{
		case 0://左键
			nFlags = 1;
			break;
		case 1://右键
			nFlags = 2;
			break;
		case 2://中键
			nFlags = 4;
			break;
		case 4://没有按键
			nFlags = 8;
			break;
		default:
			break;
		}
		if (nFlags != 8)
			SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
		switch (mouse.nAction)
		{
		case 0://单击
			nFlags |= 0x10;
			break;
		case 1://双击
			nFlags |= 0x20;
			break;
		case 2://按下不松手
			nFlags |= 0x40;
			break;
		case 3://放开
			nFlags |= 0x80;
			break;
		default:
			nFlags = 0x08;
			break;
		}
		switch (nFlags)
		{
		case 0x21://左键双击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11://左键单击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41://左键按下
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81://左键放开
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22://右键双击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12://右键单击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42://右键按下
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82://右键放开
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24://中键双击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x14://中键单击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44://中键按下
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84://中键放下
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x08://单纯的移动鼠标
			mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
			break;
		}
		CPacket pack(5, nullptr, 0);
		lstPackets.push_back(pack);
        return 0;
    }

    int SendScreen(std::list<CPacket>& lstPackets, CPacket& inPacket)
    {
        CImage screen;//GDI
        HDC hScreen = ::GetDC(nullptr);//获取设备上下文
        int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);//获取一个像素点所占的位宽(大小)RGB: 24bit ARGB888: 32bit RGB565: 16bit 
        int nWidth = GetDeviceCaps(hScreen, HORZRES);
        int nHeight = GetDeviceCaps(hScreen, VERTRES);
        //TRACE("服务端发送的宽高是: nWidth = %d, nHeight = %d", nWidth, nHeight);
        screen.Create(nWidth, nHeight, nBitPerPixel);
        BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);//获取截图
        ReleaseDC(nullptr, hScreen);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
        if (hMem == nullptr)
            return -1;
        IStream* pStream = nullptr;
        HRESULT ret = CreateStreamOnHGlobal(hMem, true, &pStream);
        if (ret == S_OK)
        {
            screen.Save(pStream, Gdiplus::ImageFormatPNG);//把截图的数据信息保存在一个输入流中
            LARGE_INTEGER bg = { 0 };
            pStream->Seek(bg, STREAM_SEEK_SET, nullptr);//将流重新定位到保存信息的开头位置
            PBYTE pData = PBYTE(GlobalLock(hMem));
            SIZE_T nSize = GlobalSize(hMem);
            CPacket pack(6, pData, nSize);
            lstPackets.push_back(pack);
            GlobalUnlock(hMem);
        }
        pStream->Release();
        GlobalFree(hMem);
        screen.ReleaseDC();
        //下面比较了png与jpg的内存消耗和生成说用的CPU时间
        /*DWORD tick = GetTickCount64();
        screen.Save(_T("test2023.png"), Gdiplus::ImageFormatPNG);
        TRACE("png %d\r\n", GetTickCount64() - tick);
        tick = GetTickCount64();
        screen.Save(_T("test2023.jpg"), Gdiplus::ImageFormatJPEG);
        TRACE("jpg %d\r\n", GetTickCount64() - tick);
        screen.ReleaseDC();*/
        return 0;
    }

    int LockMachine(std::list<CPacket>& lstPackets, CPacket& inPacket)
    {
        if (dlg.m_hWnd == nullptr || dlg.m_hWnd == INVALID_HANDLE_VALUE)
        {
            //_beginthread(threadLockDlg, 0, nullptr);
            _beginthreadex(nullptr, 0, &CCommand::threadLockDlg, this, 0, &threadid);
            TRACE("threadid = %d\r\n", threadid);
        }
        CPacket pack(7, nullptr, 0);
        lstPackets.push_back(pack);
        return 0;
    }

    int UnlockMachine(std::list<CPacket>& lstPackets, CPacket& inPacket)
    {
        //dlg.SendMessage(WM_KEYDOWN, 0x41, 0x01E001);
        //::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x41, 0x01E0001);
        PostThreadMessage(threadid, WM_KEYDOWN, 0x1B, 0);
        CPacket pack(8, nullptr, 0);
        lstPackets.push_back(pack);
        return 0;
    }

    int TestConnect(std::list<CPacket>& lstPackets, CPacket& inPacket)
    {
        CPacket pack(1981, nullptr, 0);
        lstPackets.push_back(pack);
        return 0;
    }

    int DeleteLocalFile(std::list<CPacket>& lstPackets, CPacket& inPacket)
    {
        std::string strPath = inPacket.strData;
        TCHAR sPath[MAX_PATH] = _T("");
        //#pragma warning(suppress : 4996)
        //    mbstowcs(sPath, strPath.c_str(), strPath.size());//这种方式中文容易乱码
        MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(), sPath, sizeof(sPath) / sizeof(TCHAR));
        DeleteFileA(strPath.c_str());
        CPacket pack(9, nullptr, 0);
        lstPackets.push_back(pack);
        return 0;
    }
};

