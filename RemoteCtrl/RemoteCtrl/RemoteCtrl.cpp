// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"
#include "Command.h"
#include <Conio.h>

#pragma warning(disable : 4996)

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
void initsock();
void clearsock();

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

    initsock();

    if (argc == 1)
    {
        char strDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, strDir);
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        memset(&si, 0, sizeof(si));
        memset(&pi, 0, sizeof(pi));
        string strCmd = argv[0];
        strCmd += " 1";//注意! 1前面要有空格
        BOOL bRet = CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE, 0, NULL, strDir, &si, &pi);
        if (bRet)
        {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            TRACE("进程ID: %d\r\n", pi.dwProcessId);
            TRACE("线程ID: %d\r\n", pi.dwThreadId);
            strCmd += " 2";//注意! 2前面要有空格
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
    else if (argc == 2)//主客户端(在本项目中相当于被监控端)
    {
        udp_client();
        printf("主客户端进程即将结束\r\n");
    }  
    else//从客户端代码(在本项目中相当于监控端)
    {
        udp_client(false);
        printf("从客户端进程即将结束\r\n");
    }
    
    clearsock();
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

void initsock()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
}

void clearsock()
{
    WSACleanup();
}

#include "IQSocket.h"
#include "IQNetwork.h"
int RecvFromCB(void* arg, const IQBuffer& buffer, IQSockaddrIn addr)
{
    IQServer* server = (IQServer*)arg;
    return server->Sendto(addr, buffer);
}
int SendToCB(void* arg, const IQSockaddrIn& addr, int ret)
{
    IQServer* server = (IQServer*)arg;
    printf("sendto done!%p\r\n", server);
    return 0;
}

void udp_server()
{
    std::list<IQSockaddrIn> lstclients;
    printf("%s(%d): %s\r\n", __FILE__, __LINE__, __FUNCTION__);
    IQServerParameter param("127.0.0.1", 20000, IQSocket::IQTypeUDP, NULL, NULL, NULL, RecvFromCB, SendToCB);
    IQServer server(param);
    server.Invoke(&server);
    printf("%s(%d): %s ERROR!\r\n", __FILE__, __LINE__, __FUNCTION__);

    getchar();
    return;
}

void udp_client(bool ishost)
{
    Sleep(2000);
    sockaddr_in server, client;
    int len = sizeof(client);
    server.sin_family = AF_INET;
    server.sin_port = htons(20000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        printf("%s(%d): %s ERROR!\r\n", __FILE__, __LINE__, __FUNCTION__);
        return;
    }
    if (ishost)//主客户端代码
    {
        printf("%s(%d): %s\r\n", __FILE__, __LINE__, __FUNCTION__);
        IQBuffer msg = "hello world!\n";
        int ret = sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&server, sizeof(server));
        if (ret > 0)
        {
            msg.resize(1024);
            memset((char*)msg.c_str(), 0, msg.size());
            ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
            printf("host %s(%d): %s ERROR(%d)! ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
            if (ret > 0)
            {
                printf("%s(%d): %s ip: %08X, port: %d\r\n", __FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
                printf("%s(%d): %s msg = %d\r\n", __FILE__, __LINE__, __FUNCTION__, msg.size());
            }
            ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
            printf("host %s(%d): %s ERROR(%d)! ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
            if (ret > 0)
            {
                printf("%s(%d): %s ip: %08X, port: %d\r\n", __FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
                printf("%s(%d): %s msg = %s\r\n", __FILE__, __LINE__, __FUNCTION__, msg.c_str());
            }
        }
    }
    else//从客户端代码
    {
        printf("%s(%d): %s\r\n", __FILE__, __LINE__, __FUNCTION__);
        std::string msg = "hello world!\n";
        int ret = sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&server, sizeof(server));
        if (ret > 0)
        {
            msg.resize(1024);
            memset((char*)msg.c_str(), 0, msg.size());
            ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
            printf("client %s(%d): %s ERROR(%d)! ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
            if (ret > 0)
            {
                sockaddr_in addr;
                memcpy(&addr, msg.c_str(), sizeof(addr));
                sockaddr_in* paddr = (sockaddr_in*)&addr;
                printf("%s(%d): %s ip: %08X, port: %d\r\n", __FILE__, __LINE__, __FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
                printf("%s(%d): %s msg = %d\r\n", __FILE__, __LINE__, __FUNCTION__, msg.size());
                printf("%s(%d): %s ip: %08X, port: %d\r\n", __FILE__, __LINE__, __FUNCTION__, paddr->sin_addr.s_addr, ntohs(paddr->sin_port));
                msg = "hello, I am client!\r\n";
                ret = sendto(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)paddr, sizeof(sockaddr_in));
                printf("%s(%d): %s ip: %08X, port: %d\r\n", __FILE__, __LINE__, __FUNCTION__, paddr->sin_addr.s_addr, ntohs(paddr->sin_port));
                printf("client %s(%d): %s ERROR(%d)! ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
            }
        }
    }
    closesocket(sock);
}
