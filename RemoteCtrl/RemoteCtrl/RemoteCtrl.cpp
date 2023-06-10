﻿// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
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

//服务端开机启动函数
//开机启动的时候, 程序能否启动(即是已经在注册表里注册)与启动用户的权限有关, 例如程序的启动权限是管理员级别, 而启动用户的权限是普通用户, 则不会启动程序
//开机启动对环境变量有影响, 如果依赖dll(动态库), 则可能启动失败
//若要使用动态库, 复制这些dll到文件夹system32(!!!多是64位程序)下面sysWOW64(!!!多是32位程序)下面
void ChooseAutoInvoke()
{
    TCHAR wcsSystem[MAX_PATH] = _T("");
    CString strPath = CString(_T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));
    if (PathFileExists(strPath))
        return;
    CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    CString strInfo = _T("该程序只允许用于合法用途\n");
    strInfo += _T("继续运行该程序, 这台计算机将处于被监控状态!\n");
    strInfo += _T("如果你不希望这样, 请点击\"取消\"按钮\n");
    strInfo += _T("点击\"是\"按钮, 该程序将被复制到该计算机上, 并随系统启动而自动运行!\n");
    strInfo += _T("点击\"否\"按钮, 该程序只运行一次, 并且不会在系统内留下任何东西!\n");

    int ret = MessageBox(nullptr, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
    if (ret == IDYES)
    {
        char sSys[MAX_PATH] = "";
        char sPath[MAX_PATH] = "";
        std::string strExe = "\\RemoteCtrl.exe ";
        GetCurrentDirectoryA(MAX_PATH, sPath);
        GetSystemDirectoryA(sSys, sizeof(sSys));
        std::string strCmd = "mklink " + std::string(sSys) +strExe + std::string(sPath) + strExe;
        system(strCmd.c_str());
        HKEY hKey = nullptr;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_WRITE | KEY_WOW64_64KEY, &hKey);//打开注册表下的指定路径
        if (ret != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败! 是否权限不足?\r\n程序启动失败!"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            exit(0);
        }
        CString strPath = CString(_T("%SystemRoot%\\SysWOW64\\RemoteCtrl.exe"));
        ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));//添加到注册表的指定路径下
        if (ret != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("设置自动开机启动失败! 是否权限不足?\r\n程序启动失败!"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            exit(0);
        }
        RegCloseKey(hKey);
    }
    else if (ret == IDCANCEL)
        exit(0);

}

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
            // 从比较难的技术开始, 本项目从服务端(被控制端)的实现开始
            // 1.进度的可控性 2.对接的方便性 3.可行性评估,提早暴露风险         
            ChooseAutoInvoke();
            CCommand cmd;//命令处理模块
            int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);//网络处理模块
            switch (ret)
            {
            case -1:
            {
                MessageBox(nullptr, _T("网络初始化异常, 请检查网络状态!"), _T("网络初始化失败!"), MB_OK | MB_ICONERROR);
                exit(0);
                break;
            }
            case -2:
            {
                MessageBox(nullptr, _T("多次无法正常接入用户, 程序结束!"), _T("接入用户失败!"), MB_OK | MB_ICONERROR);
                exit(0);
                break;
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
