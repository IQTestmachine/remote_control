// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"
#include <direct.h>

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
    //CServerSocket::getInstance()->Send(pack);
    return 0;
}

#include <io.h>
//#include <list>

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
    

}FILEINFO, *PFILEINFO;

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
        finfo.IsInvalid = true;
        finfo.IsDirectory = true;
        finfo.HasNext = false;
        memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        //lstFileInfos.push_back(finfo);
        OutputDebugString(_T("没有权限访问目录!!!"));
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if ((hfind = _findfirst("*", &fdata)) == -1)
    {
        OutputDebugString(_T("指定路径无效或指定路径下没有任何文件!!!"));
        return -3;
    }
    do {
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        //lstFileInfos.push_back(finfo);
    } while (!_findnext(hfind, &fdata));
    //发送信息到控制端
    FILEINFO finfo;
    finfo.HasNext = false;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    CServerSocket::getInstance()->Send(pack);
    return 0;
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

            //CServerSocket* pserver = CServerSocket::getInstance();
            //if (pserver->InitSocket() == false)
            //{
            //    MessageBox(nullptr, _T("网络初始化异常, 请检查网络状态!"), _T("网络初始化失败!"), MB_OK | MB_ICONERROR);
            //    exit(0);
            //}

            //int count = 0;
            //while (CServerSocket::getInstance() != nullptr)
            //{
            //    if (pserver->AcceptClient() == false)
            //    {
            //        if (count >= 3)
            //        {
            //            MessageBox(nullptr, _T("多次无法正常接入用户, 程序结束!"), _T("接入用户失败!"), MB_OK | MB_ICONERROR);
            //            exit(0);
            //        }
            //        MessageBox(nullptr, _T("无法正常接入用户, 自动重试..."), _T("接入用户失败"), MB_OK | MB_ICONERROR);
            //        count++;
            //    }
            //    int ret = pserver->DealComment();
            //    // ToDO: 处理命令
            //}

            int nCmd = 1;
            switch (nCmd)
            {
            case 1://查看磁盘分区
				MakeDriverInfo();
				break;
            case 2://查看指定目录下的文件
                MakeDirectoryInfo();
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
