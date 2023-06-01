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

#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup") //更改子系统和入口点, 程序运行不再弹出黑框
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

            CCommand cmd;
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
                    ret = cmd.ExcuteCommand(pserver->GetPacket().sCmd);
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
