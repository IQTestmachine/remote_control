// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"
#include "Command.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup") //更改子系统和入口点, 程序运行不再弹出黑框
//#pragma comment(linker, "/subsystem:windows /entry:WinMainCRTStartup")
//#pragma comment(linker, "/subsystem:console /entry:WinMainCRTStartup")
//#pragma comment(linker, "/subsystem:console /entry:mainCRTStartup")


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

//int ExcuteCommand(int nCmd)
//{
//    int ret = 0;
//    switch (nCmd)
//    {
//    case 1://查看磁盘分区
//        ret = MakeDriverInfo();
//        break;
//    case 2://查看指定目录下的文件
//        ret = MakeDirectoryInfo();
//        break;
//    case 3://打开文件
//        ret = RunFile();
//        break;
//    case 4://下载文件
//        ret = DownloadFile();
//        break;
//    case 5://鼠标操作
//        ret = MouseEvent();
//        break;
//    case 6://发送屏幕内容==>发送屏幕截图
//        ret = SendScreen();
//        break;
//    case 7://锁机
//        ret = LockMachine();
//        break;
//    case 8://解锁
//        ret = UnlockMachine();
//        break;
//    case 9://删除文件
//        ret = DeleteLocalFile();
//        break;
//    case 1981:
//        ret = TestConnect();
//        break;
//    }
//    return ret;
//    //Sleep(5000);
//    //UnlockMachine();
//    ///*while (dlg.m_hWnd != nullptr && dlg.m_hWnd != INVALID_HANDLE_VALUE)
//    //    Sleep(1000);*/
//    //TRACE("m_hWnd = %08X\r\n", dlg.m_hWnd);
//    //while (dlg.m_hWnd != nullptr)
//    //{
//    //    Sleep(10);
//    //}
//}

void udp_server();
void udp_client(bool ishost = true);

int main(int argc, char* argv[])
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
    }

    if (argc == 1)
    {
        char strDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, strDir);
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        memset(&si, 0, sizeof(si));
        memset(&pi, 0, sizeof(pi));
        string strCmd = argv[0];
        strCmd += "1";
        BOOL bRet = CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE, 0, NULL, strDir, &si, &pi);
        printf("%d\r\n", GetLastError());
        if (bRet)
        {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            TRACE("进程ID: %d\r\n", pi.dwProcessId);
            TRACE("线程ID: %d\r\n", pi.dwThreadId);
            strCmd += "2";
            bool bRet = CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE, 0, NULL, strDir, &si, &pi);
            if (bRet)
            {
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
                TRACE("进程ID: %d\r\n", pi.dwProcessId);
                TRACE("线程ID: %d\r\n", pi.dwThreadId);
                udp_server();//公网服务器
            }
        }
    }
    else if (argc == 2)//主客户端
    {
        udp_client();
    }  
    else//从客户端代码
    {
        udp_client(false);
    }
      
    return 0;

    //int nRetCode = 0;

    //HMODULE hModule = ::GetModuleHandle(nullptr);

    //if (hModule != nullptr)
    //{
    //    // 初始化 MFC 并在失败时显示错误   
    //    if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
    //    {
    //        // TODO: 在此处为应用程序的行为编写代码。
    //        wprintf(L"错误: MFC 初始化失败\n");
    //        nRetCode = 1;
    //    }
    //    else
    //    {
    //        // 从比较难的技术开始, 本项目从服务端(被控制端)的实现开始
    //        // 1.进度的可控性 2.对接的方便性 3.可行性评估,提早暴露风险         
    //        
    //        CCommand cmd;//命令处理模块
    //        int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);//网络处理模块
    //        switch (ret)
    //        {
    //        case -1:
    //        {
    //            MessageBox(nullptr, _T("网络初始化异常, 请检查网络状态!"), _T("网络初始化失败!"), MB_OK | MB_ICONERROR);
    //            exit(0);
    //            break;
    //        }
    //        case -2:
    //        {
    //            MessageBox(nullptr, _T("多次无法正常接入用户, 程序结束!"), _T("接入用户失败!"), MB_OK | MB_ICONERROR);
    //            exit(0);
    //            break;
    //        }
    //        }
    //    }
    //}
    //else
    //{
    //    // TODO: 更改错误代码以符合需要
    //    wprintf(L"错误: GetModuleHandle 失败\n");
    //    nRetCode = 1;
    //}

    //return nRetCode;
}

void udp_server()
{
    printf("%s(%d): %s\r\n", __FILE__, __FILE__, __FUNCTION__);
    getchar();
}

void udp_client(bool ishost)
{
    if (ishost)//主客户端代码
        printf("%s(%d): %s\r\n", __FILE__, __FILE__, __FUNCTION__);
    else//从客户端代码
        printf("%s(%d): %s\r\n", __FILE__, __FILE__, __FUNCTION__);
}
