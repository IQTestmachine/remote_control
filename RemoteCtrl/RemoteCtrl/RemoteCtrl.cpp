// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
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
#include <mutex>

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

//#define IOCP_LIST_PUSH 1
//#define IOCP_LIST_POP 2
//#define IOCP_LIST_EMPTY 0

//enum {
//    IocpListEmpty,
//    IocpListPush,
//    IocpListPop
//};
//
//typedef struct IocpParam
//{
//    int nOperator;//操作
//    std::string strData;//数据
//    _beginthread_proc_type cbFunc;//回调
//    IocpParam(int np, const char* sData, _beginthread_proc_type cb = NULL)
//    {
//        nOperator = np;
//        strData = sData;
//        cbFunc = cb;
//    }
//    IocpParam()
//    {
//        nOperator = -1;
//    }
//}IOCP_PARAM;
//
//void threadmain(HANDLE& hIOCP)
//{
//    int counts1_push = 0, counts1_pop = 0;
//    std::list<std::string> lstString;
//    DWORD dwTransferred = 0;
//    ULONG_PTR CompletionKey = 0;
//    OVERLAPPED* pOverlapped = NULL;
//    while (GetQueuedCompletionStatus(hIOCP, &dwTransferred, &CompletionKey, &pOverlapped, INFINITE))
//    {
//        if (dwTransferred == 0 && CompletionKey == NULL)
//        {
//            printf("thread is prepare to exit!\r\n");
//            break;
//        }
//        IOCP_PARAM* pParam = (IOCP_PARAM*)CompletionKey;
//        if (pParam->nOperator == IocpListPush)
//        {
//            lstString.push_back(pParam->strData);
//            counts1_push++;
//        }          
//        else if (pParam->nOperator == IocpListPop)
//        {
//            std::string* pStr = nullptr;
//            if (lstString.size() > 0)
//            {
//                pStr = new std::string(lstString.front());
//                lstString.pop_front();
//            }
//            if (pParam->cbFunc)
//            {
//                pParam->cbFunc(pStr);
//            }
//            counts1_pop++;
//        }
//        else if (pParam->nOperator == IocpListEmpty)
//            lstString.clear();
//
//        delete pParam;
//    }
//    printf("counts1_push = %d, counts1_pop = %d\r\n", counts1_push, counts1_pop);
//}
//void threadQueueEntry(HANDLE hIOCP)
//{
//    //开辟线程一般采用这种方式: 一个线程入口函数, 一个线程处理函数, 这样能够保障局部对象的析构函数被调用
//    //如果只有一个函数作为入口的同时也作为处理函数, 则在结束线程时并不能保证局部对象被释放, 会引起内存泄露
//    threadmain(hIOCP);
//    _endthread();
//}
//
//void func(void* arg)
//{
//    std::string* pstr = (std::string*)arg;
//    if (pstr != nullptr)
//        printf("pop from list: %s\r\n", pstr->c_str());
//    else
//        printf("list is empty, no data\r\n");
//    delete pstr;
//}

/*测试的几个步骤
* 1.bug测试/功能测试
* 2.关键因素测试(内存泄露, 运行的稳定性, 条件性)
* 3.压力测试(可靠性测试)
* 4.性能测试
*/
//利用IOCP自定义线程安全队列测试的性能测试
void test()
{
    CIQtestmachineQueue<std::string> lstString;

    ULONGLONG tick = GetTickCount64();
    ULONGLONG tick0 = GetTickCount64();
    ULONGLONG total = GetTickCount64();
    while (GetTickCount64() - total <= 1000/*_kbhit() == 0*/)//完成端口, 把请求与实现(读与写)进行了分离
    {
        /*if (GetTickCount64() - tick0 > 10)
        {
            lstString.PushBack("hello world");
            tick0 = GetTickCount64();
        }*/
        lstString.PushBack("hello world");
        tick0 = GetTickCount64();
    }
    size_t counts = lstString.Size();
    printf("push: %d/s\r\n", counts);
    total = GetTickCount64();
    while (GetTickCount64() - total <= 1000)
    {
        std::string str;
        lstString.PopFront(str);
        tick = GetTickCount64();
    }
    printf("pop %d/s\r\n", counts - lstString.Size());
    lstString.Clear();
    printf("exit done!\r\n");

    std::list<std::string> lst_STL;//STL容器list本身的性能
    total = GetTickCount64();
    while (GetTickCount64() - total <= 1000)
    {
        lst_STL.push_back("hello world");
    }
    counts = lst_STL.size();
    printf("STL: list push %d/s\r\n", lst_STL.size());
    total = GetTickCount64();
    while (GetTickCount64() - total <= 200)
    {
        lst_STL.pop_front();
    }
    printf("STL: pop %d/s\r\n", (counts - lst_STL.size()) * 5);

    std::list<std::string> lst_mutex;//使用互斥锁的性能
    std::mutex lock_test;
    total = GetTickCount64();
    while (GetTickCount64() - total <= 1000)
    {
        lock_test.lock();
        lst_STL.push_back("hello world");
        lock_test.unlock();
    }
    counts = lst_STL.size();
    printf("mutex: list push %d/s\r\n", lst_STL.size());
    total = GetTickCount64();
    while (GetTickCount64() - total <= 200)
    {
        lock_test.lock();
        lst_STL.pop_front();
        lock_test.unlock();
    }
    printf("mutex: pop %d/s\r\n", (counts - lst_STL.size()) * 5);
}

int main()
{
    if (!CIQtestmachineTool::Init())
        return 1;
    //printf("press any key exit ...\r\n");

    for (int i = 0; i < 10; i++)
    {
        test();
    }

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
