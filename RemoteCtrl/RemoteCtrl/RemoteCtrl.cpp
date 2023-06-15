﻿// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"
#include "Command.h"
#include "IQtestmachineTool.h"
#include <list>
#include <conio.h>
#include "IQtestmachineQueue.h"
#include <MSWSock.h>
#include "IQtestmachineServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup") //更改子系统和入口点, 程序运行不再弹出黑框
//#pragma comment(linker, "/subsystem:windows /entry:WinMainCRTStartup")
//#pragma comment(linker, "/subsystem:console /entry:WinMainCRTStartup")
//#pragma comment(linker, "/subsystem:console /entry:mainCRTStartup")
// 
//#define INVOKE_PATH _T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe")//第一种开机启动方式
#define INVOKE_PATH _T("C:\\Users\\31760\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe")//第二种开机启动方式

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

bool ChooseAutoInvoke(const CString& strPath)
{
    TCHAR wcsSystem[MAX_PATH] = _T("");
    if (PathFileExists(strPath))
        return true;
    CString strInfo = _T("该程序只允许用于合法用途\n");
    strInfo += _T("继续运行该程序, 这台计算机将处于被监控状态!\n");
    strInfo += _T("如果你不希望这样, 请点击\"取消\"按钮\n");
    strInfo += _T("点击\"是\"按钮, 该程序将被复制到该计算机上, 并随系统启动而自动运行!\n");
    strInfo += _T("点击\"否\"按钮, 该程序只运行一次, 并且不会在系统内留下任何东西!\n");

    int ret = MessageBox(nullptr, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
    if (ret == IDYES)
    {
        //WriteRegisterTable();//第一种开机启动方式
        if (!CIQtestmachineTool::WriteStartupDir(strPath))//第二种开机启动方式
            return false;
    }
    else if (ret == IDCANCEL)
       return false;
    return true;
}

void iocp();

int main()
{
    if (!CIQtestmachineTool::Init())
        return 1;
    
    iocp();
  

    //if (CIQtestmachineTool::IsAdmin())
    //{
    //    if (!CIQtestmachineTool::Init())
    //        return 1;
    //    if (ChooseAutoInvoke(INVOKE_PATH))
    //    {
    //        // 从比较难的技术开始, 本项目从服务端(被控制端)的实现开始
    //        // 1.进度的可控性 2.对接的方便性 3.可行性评估,提早暴露风险 
    //        CCommand cmd;//命令处理模块
    //        int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);//网络处理模块
    //        switch (ret)
    //        {
    //        case -1:
    //        {
    //            MessageBox(nullptr, _T("网络初始化异常, 请检查网络状态!"), _T("网络初始化失败!"), MB_OK | MB_ICONERROR);
    //            break;
    //        }
    //        case -2:
    //        {
    //            MessageBox(nullptr, _T("多次无法正常接入用户, 程序结束!"), _T("接入用户失败!"), MB_OK | MB_ICONERROR);
    //            break;
    //        }
    //        }
    //    }
    //} 
    //else
    //{
    //    if (!CIQtestmachineTool::RunAsAdmin())
    //        return 0;
    //}   
    
    return 0;
}

class COverlapped
{
public:
    OVERLAPPED m_overlapped;
    DWORD m_operator;
    char m_buffer[4096];
    COverlapped()
    {
        m_operator = 0;
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        memset(m_buffer, 0, sizeof(m_buffer));
    }
};

void iocp()
{
    //SOCKET server = socket(AF_INET, SOCK_STREAM, 0);//普通TCP连接套接字
    //server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);//重叠IO的TCP套接字
    //if (server == INVALID_SOCKET)
    //{
    //    CIQtestmachineTool::ShowError();
    //    return;
    //}
    //HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, server, 4);
    //SOCKET client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    //CreateIoCompletionPort((HANDLE)server, hIOCP, 0, 0);
    //sockaddr_in addr;
    //addr.sin_family = PF_INET;
    //addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    //addr.sin_port = htons(9527);
    //bind(server, (sockaddr*)&addr, sizeof(addr));
    //listen(server, 5);

    //COverlapped overlapped;
    //overlapped.m_operator = 1;
    //memset(&overlapped, 0, sizeof(OVERLAPPED));
    ////高并发accept()不合适
    //char buffer[4096] = "";
    //DWORD received = 0;
    //if (AcceptEx(server, client, overlapped.m_buffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &received, &overlapped.m_overlapped) == false)
    //    CIQtestmachineTool::ShowError();

    //overlapped.m_operator = 2;
    //WSASend();
    //overlapped.m_operator = 3;
    //WSARecv();
    ////开启线程
    //while (true)//代表一个线程
    //{
    //    LPOVERLAPPED pOverlapped = NULL;
    //    DWORD transferred = 0;
    //    DWORD key = 0;
    //    if (GetQueuedCompletionStatus(hIOCP, &transferred, &key, &pOverlapped, INFINITY))
    //    {
    //        COverlapped* po = CONTAINING_RECORD(pOverlapped, COverlapped, m_overlapped);
    //        switch (po->m_operator)
    //        {
    //        case 1:
    //            //处理accept操作
    //        }

    //    }
    //}

    CIQtestmachineServer server;
    server.StartServer();
    getchar();
}
