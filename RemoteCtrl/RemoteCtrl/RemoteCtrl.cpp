// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"
#include <direct.h>
#include <atlimage.h> //用于发送屏幕截图的头文件

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup") //更改子系统和入口点, 程序运行不再弹出黑框
//#pragma comment(linker, "/subsystem:windows /entry:WinMainCRTStartup")
//#pragma comment(linker, "/subsystem:console /entry:WinMainCRTStartup")
//#pragma comment(linker, "/subsystem:console /entry:mainCRTStartup")


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;
void Dump(BYTE* pData, size_t nSize)
{
    std::string strOut;
    for (size_t i = 0; i < nSize; i++)
    {
        char buf[8] = "";
        if (i > 0 && (i % 16) == 0)
            strOut += "\n";
        snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
        strOut += buf;
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());
}

int MakeDriverInfo() // 获取磁盘分区
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
    CPacket pack(1, (BYTE*)result.c_str(), result.size());//生成磁盘分区的数据包
    Dump((BYTE*)pack.Data(), pack.Size());
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

#include <io.h>
//#include <list>

int MakeDirectoryInfo()
{
    std::string strPath;
    //std::list<FILEINFO> lstFileInfos;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false)
    {
        OutputDebugString(_T("当前命令, 不是获取文件列表, 命令解析错误!"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0)
    {
        FILEINFO finfo;
        finfo.HasNext = false;
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);//拿到一个文件就发送信息到控制端
        //lstFileInfos.push_back(finfo); 
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
        CServerSocket::getInstance()->Send(pack);
        return -3;
    }
    do {
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
        TRACE("%s\r\n", finfo.szFileName);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);//拿到一个文件就发送信息到控制端
        //lstFileInfos.push_back(finfo); 采用链表获取全部文件和文件夹,可能因文件和文件夹太多导致迟迟不能发送, 舍弃链表的方式
    } while (!_findnext(hfind, &fdata));
    FILEINFO finfo;
    finfo.HasNext = false;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int RunFile()
{
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    ShellExecuteA(nullptr, nullptr, strPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    CPacket pack(3, nullptr, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int DownloadFile()
{
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    long long data = 0;
    FILE* pFile = nullptr;
    errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
    if (err != 0)
    {
        CPacket pack(4, (BYTE*)&data, 0);
        CServerSocket::getInstance()->Send(pack);
        return -1;
    }
    if (pFile != nullptr)
    {
        fseek(pFile, 0, SEEK_END);
        data = _ftelli64(pFile);
        CPacket head(4, (BYTE*)&data, 8);
        fseek(pFile, 0, SEEK_SET);
        char buffer[1024] = "";
        size_t rlen = 0;
        do {
            rlen = fread(buffer, 1, 1024, pFile);
            CPacket pack(4, (BYTE*)buffer, rlen);
            CServerSocket::getInstance()->Send(pack);
        } while (rlen >= 1024);
        fclose(pFile);
    }
    CPacket pack(4, nullptr, 0);
    CServerSocket::getInstance()->Send(pack);
    
    return 0;
}

int MouseEvent()
{
    MOUSEEV mouse;
    if (CServerSocket::getInstance()->GetMouseEvent(mouse))
    {
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
        CServerSocket::getInstance()->Send(pack);
    }
    else
    {
        OutputDebugString(_T("获取鼠标操作参数失败!!!"));
        return -1;
    }
    return 0;
}

int SendScreen()
{
    CImage screen;//GDI
    HDC hScreen = ::GetDC(nullptr);//获取设备上下文
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);//获取一个像素点所占的位宽(大小)RGB: 24bit ARGB888: 32bit RGB565: 16bit 
    int nWidth = GetDeviceCaps(hScreen, HORZRES);
    int nHeight = GetDeviceCaps(hScreen, VERTRES);
    screen.Create(nWidth, nHeight, nBitPerPixel);
    BitBlt(screen.GetDC(), 0, 0, 1920, 1020, hScreen, 0, 0, SRCCOPY);//获取截图
    ReleaseDC(nullptr, hScreen);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
    if (hMem == nullptr)
        return -1;
    IStream* pStream = nullptr;
    HRESULT ret = CreateStreamOnHGlobal(hMem, true, &pStream);
    if (ret == S_OK)
    {
        screen.Save(pStream, Gdiplus::ImageFormatJPEG);//把截图的数据信息保存在一个输入流中
        LARGE_INTEGER bg = { 0 };
        pStream->Seek(bg, STREAM_SEEK_SET, nullptr);//将流重新定位到保存信息的开头位置
        PBYTE pData = PBYTE(GlobalLock(hMem));
        SIZE_T nSize = GlobalSize(hMem);
        CPacket pack(6, nullptr, nSize);
        CServerSocket::getInstance()->Send(pack);
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

#include "LockDialog.h"
CLockDialog dlg;
unsigned threadid = 0;

unsigned __stdcall threadLockDlg(void* arg)//线程处理函数
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
    dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//置顶, 对话框置于图层最前方
    ShowCursor(false);//限制鼠标, 使其在对话框内消失
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), nullptr), SW_HIDE);//隐藏任务栏
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
    ShowCursor(true);
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), nullptr), SW_SHOW);//恢复任务栏
    dlg.DestroyWindow();
    _endthreadex(0);
    return 0;
}

int LockMachine()
{
    if (dlg.m_hWnd == nullptr || dlg.m_hWnd == INVALID_HANDLE_VALUE)
    {
        //_beginthread(threadLockDlg, 0, nullptr);
        _beginthreadex(nullptr, 0, threadLockDlg, nullptr, 0, &threadid);
        TRACE("threadid = %d\r\n", threadid);
    }
    CPacket pack(7, nullptr, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int UnlockMachine()
{
    //dlg.SendMessage(WM_KEYDOWN, 0x41, 0x01E001);
    //::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x41, 0x01E0001);
    PostThreadMessage(threadid, WM_KEYDOWN, 0x1B, 0);
    CPacket pack(7, nullptr, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int TestConnect()
{
    CPacket pack(1981, nullptr, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int ExcuteCommand(int nCmd)
{
    int ret = 0;
    switch (nCmd)
    {
    case 1://查看磁盘分区
        ret = MakeDriverInfo();
        break;
    case 2://查看指定目录下的文件
        ret = MakeDirectoryInfo();
        break;
    case 3://打开文件
        ret = RunFile();
        break;
    case 4://下载文件
        ret = DownloadFile();
        break;
    case 5://鼠标操作
        ret = MouseEvent();
        break;
    case 6://发送屏幕内容==>发送屏幕截图
        ret = SendScreen();
        break;
    case 7://锁机
        ret = LockMachine();
        break;
    case 8://解锁
        ret = UnlockMachine();
        break;
    case 1981:
        ret = TestConnect();
        break;
    }
    return ret;
    //Sleep(5000);
    //UnlockMachine();
    ///*while (dlg.m_hWnd != nullptr && dlg.m_hWnd != INVALID_HANDLE_VALUE)
    //    Sleep(1000);*/
    //TRACE("m_hWnd = %08X\r\n", dlg.m_hWnd);
    //while (dlg.m_hWnd != nullptr)
    //{
    //    Sleep(10);
    //}

}

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误   
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            //// 从比较难的技术开始, 本项目从服务端(被控制端)的实现开始
            //// 1.进度的可控性 2.对接的方便性 3.可行性评估,提早暴露风险
            //// TODO: socket, blind, listen, accept, read, close
            //// 套接字初始化          

            CServerSocket* pserver = CServerSocket::getInstance();
            if (pserver->InitSocket() == false)
            {
                MessageBox(nullptr, _T("网络初始化异常, 请检查网络状态!"), _T("网络初始化失败!"), MB_OK | MB_ICONERROR);
                exit(0);
            }

            int count = 0;
            while (CServerSocket::getInstance() != nullptr)
            {
                if (pserver->AcceptClient() == false)
                {
                    if (count >= 3)
                    {
                        MessageBox(nullptr, _T("多次无法正常接入用户, 程序结束!"), _T("接入用户失败!"), MB_OK | MB_ICONERROR);
                        exit(0);
                    }
                    MessageBox(nullptr, _T("无法正常接入用户, 自动重试..."), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                    count++;
                }
                int ret = pserver->DealCommand();
                // ToDO: 处理命令
                if (ret > 0)
                {
                    //ExcuteCommand(ret); 这样不也可以吗? 为何还要定义GetPacket()来获得sCmd？
                    ret = ExcuteCommand(pserver->GetPacket().sCmd);
                    if (ret != 0)
                        TRACE("执行命令失败: %d ret = %d\r\n", pserver->GetPacket().sCmd, ret);
                    pserver->CloseClient();
                }                
            }
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
