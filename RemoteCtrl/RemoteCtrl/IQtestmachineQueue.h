#pragma once
#include "pch.h"
#include <atomic>
#include <list>

template <class T>//线程安全的队列模板类(利用IOCP实现)
class CIQtestmachineQueue
{
private:
    std::list<T> m_lstData;
    HANDLE m_hCompletionPort;
    HANDLE m_hThread;
    std::atomic<bool> m_lock;//队列正在析构
public:
    enum {
        IQNone,
        IQPush,
        IQPop,
        IQSize,
        IQClear
    };
    typedef struct IocpParam
    {
        size_t nOperator;//操作
        T sData;//数据
        HANDLE hEvent;//用于pop操作
        IocpParam(int np, const T& data, HANDLE hEve = NULL)
        {
            nOperator = np;
            sData = data;
            hEvent = hEve;
        }
        IocpParam()
        {
            nOperator = IQNone;
        }
    }PPARAM;//Post Parameter 用于投递消息的结构体
    
public:
    CIQtestmachineQueue()
    {
        m_lock = false;
        m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
        m_hThread = INVALID_HANDLE_VALUE;
        if (m_hCompletionPort != nullptr)
        {
            m_hThread = (HANDLE)_beginthread(&CIQtestmachineQueue<T>::threadEntry, 0, this);
        }
    }

    ~CIQtestmachineQueue()
    {
        if (m_lock)
            return;
        m_lock = true;
        
        PostQueuedCompletionStatus(m_hCompletionPort, 0, NULL, NULL);
        WaitForSingleObject(m_hThread, INFINITE);
        if (m_hCompletionPort != nullptr)
        {
            HANDLE hTmp = m_hCompletionPort;
            m_hCompletionPort = nullptr;
            CloseHandle(hTmp);
        }
    }

	bool PushBack(const T& data)
    {
        IocpParam* pParam = new IocpParam(IQPush, data);
        if (m_lock == true)
        {
            delete pParam;
            return false;
        }
        bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
        if (ret == false)
            delete pParam;
        return ret;
    }

	bool PopFront(T& data)
    {
        if (m_lock == true)
            return false;
        HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        IocpParam pParam(IQPop, data, hEvent);
        bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM), (ULONG_PTR)&pParam, NULL);
        if (ret == false)
        {
            CloseHandle(hEvent);
            return false;
        }
        ret = (WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0);
        if (ret)
        {
            data = pParam.sData;
        }

        return ret;
    }

    size_t Size()
    {
        HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        IocpParam pParam(IQSize, T(), hEvent);
        if (m_lock)
		{
            if (hEvent)
                CloseHandle(hEvent);
			return -1;
		}
        bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM), (ULONG_PTR)&pParam, NULL);
        if (ret == false)
        {
            CloseHandle(hEvent);
            return -1;
        }
        ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;
        if (ret)
        {
            return pParam.nOperator;
        }
        return ret;
    }

    bool Clear()
    {
        if (m_lock == true)
            return false;
        IocpParam* pParam = new IocpParam(IQPush, T());
        bool ret = PostQueuedCompletionStatus(m_hCompletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
        if (ret == false)
            delete pParam;
        return ret;
    }
private:
    void DealParam(PPARAM* pParam)
    {
        switch (pParam->nOperator)
        {
        case IQPush:
            m_lstData.push_back(pParam->sData);
            delete pParam;
            break;
        case IQPop:
            if (m_lstData.size() > 0)
            {
                pParam->sData = m_lstData.front();
                m_lstData.pop_front();
            }
            if (pParam->hEvent != nullptr)
                SetEvent(pParam->hEvent);
            break;
        case IQSize:
            pParam->nOperator = m_lstData.size();
            if (pParam->hEvent != nullptr)
                SetEvent(pParam->hEvent);
            break;
        case IQClear:
            m_lstData.clear();
            delete pParam;
            break;
        default:
            OutputDebugStringA("unknown operator!\r\n");
        }
    }

    static void threadEntry(void* arg)
    {
        CIQtestmachineQueue<T>* thiz = (CIQtestmachineQueue<T>*)arg;
        thiz->threadMain();
        _endthread();
    }
    void threadMain()
    {
        PPARAM* pParam = nullptr;
        DWORD dwTransferred = 0;
        ULONG_PTR CompletionKey = 0;
        OVERLAPPED* pOverlapped = NULL;
        while (GetQueuedCompletionStatus(m_hCompletionPort, &dwTransferred, &CompletionKey, &pOverlapped, INFINITE))
        {
            if (dwTransferred == 0 && CompletionKey == NULL)
            {
                printf("thread is prepare to exit!\r\n");
                break;
            }
            pParam = (PPARAM*)CompletionKey;
            DealParam(pParam);

            while (GetQueuedCompletionStatus(m_hCompletionPort, &dwTransferred, &CompletionKey, &pOverlapped, 0))
            {
                pParam = (PPARAM*)CompletionKey;
                DealParam(pParam);
            }
        }
        HANDLE hTmp = m_hCompletionPort;
        m_hCompletionPort = nullptr;
        CloseHandle(hTmp);
    }
};

