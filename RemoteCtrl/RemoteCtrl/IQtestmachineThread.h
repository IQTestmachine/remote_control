#pragma once
#include "pch.h"
#include <atomic>
#include <vector>
#include <mutex>
#include <Windows.h>

class ThreadFuncBase
{
};

typedef int (ThreadFuncBase::* FUNCTYPE)();
class CThreadWorker
{
public:
	CThreadWorker() : thiz(nullptr), func(nullptr) { }
	CThreadWorker(void* obj, FUNCTYPE f) : thiz((ThreadFuncBase*)obj), func(f) { }
	CThreadWorker(const CThreadWorker& worker)
	{
		thiz = worker.thiz;
		func = worker.func;
	}
	CThreadWorker& operator=(const CThreadWorker& worker)
	{
		if (this != &worker)
		{
			thiz = worker.thiz;
			func = worker.func;
		}
		return *this;
	}
	int operator()()
	{
		if (IsValid())
			return (thiz->*func)();
		return -1;
	}
	bool IsValid() const
	{
		return thiz != NULL && func != NULL;
	}
private:
	ThreadFuncBase* thiz;
	FUNCTYPE func;
};

class CIQtestmachineThread
{
public:
	CIQtestmachineThread()
	{
		m_hThread = NULL;
		m_bStatus = false;
	}

	~CIQtestmachineThread()
	{
		Stop();
	}

	bool Start()
	{
		m_bStatus = true;
		m_hThread = (HANDLE)_beginthread(&CIQtestmachineThread::ThreadEntry, 0, this);
		if (!IsValid())
			m_bStatus = false;
		return m_bStatus;
	}
	bool IsValid()//监控线程状态 返回true代表线程有效, 返回false代表线程异常或者异常终止
	{
		if (m_hThread == NULL || m_hThread == INVALID_HANDLE_VALUE)
			return false;
		return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
	}

	bool Stop()
	{
		if (m_hThread == NULL || m_hThread == INVALID_HANDLE_VALUE)
			return false;
		m_bStatus = false;
		DWORD ret = WaitForSingleObject(m_hThread, 1000);
		if (ret == WAIT_TIMEOUT)
		{
			TerminateThread(m_hThread, -1);
		}
		UpdataWorker();
		return ret == WAIT_OBJECT_0;
	}

	void UpdataWorker(const ::CThreadWorker& worker = ::CThreadWorker())
	{
		if (m_worker.load() != NULL && m_worker.load() != &worker)
		{
			CThreadWorker* pWorker = m_worker.load();
			TRACE("delete pWorker = %08X, m_worker = %08X\r\n", pWorker, m_worker.load());
			m_worker.store(NULL);
			delete pWorker;
		}
		if (m_worker.load() == &worker)
			return;
		if (!worker.IsValid())
		{
			m_worker.store(NULL);
			return;
		}
		CThreadWorker* pWorker = new CThreadWorker(worker);
		TRACE("new pWorker = %08X, m_worker = %08X\r\n", pWorker, m_worker.load());
		m_worker.store(pWorker);
	}

	//返会true代表空闲, false代表已经分配了工作
	bool IsIdle()
	{
		if (m_worker.load() == NULL)
			return true;
		return !m_worker.load()->IsValid();
	}
private:
	virtual void ThreadWorker()
	{
		while (m_bStatus)
		{
			if (m_worker.load() == NULL)
			{
				Sleep(1);
				continue;
			}
			::CThreadWorker worker = *m_worker.load();
			if (worker.IsValid())
			{
				if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT)
				{
					int ret = worker();
					if (ret != 0)
					{
						CString str;
						str.Format(_T("thread found warning code %d\r\n"), ret);
						OutputDebugString(str);
					}
					if (ret < 0)
					{
						CThreadWorker* pWorker = m_worker.load();
						m_worker.store(NULL);
						delete pWorker;
					}
				}
			}
			else
				Sleep(1);
		}
	}
	static void ThreadEntry(void* arg)
	{
		CIQtestmachineThread* thiz = (CIQtestmachineThread*)arg;
		if (thiz)
			thiz->ThreadWorker();
		_endthread();
	}

private:
	HANDLE m_hThread;
	bool m_bStatus;//true: 表示线程正在运行 false: 表示线程即将结束
	std::atomic<::CThreadWorker*> m_worker;
};

class IQtestmachinePool
{
public:
	IQtestmachinePool() { }
	IQtestmachinePool(size_t size)
	{ 
		m_threads.resize(size);
		for (size_t i = 0; i < size; i++)
			m_threads[i] = new CIQtestmachineThread();
	}
	
	~IQtestmachinePool()
	{
		Stop();
		for (size_t i = 0; i < m_threads.size(); i++)
		{
			delete m_threads[i];
			m_threads[i] = NULL;
		}
		
		m_threads.clear();
	}

	bool Invoke()
	{
		bool ret = true;
		for (size_t i = 0; i < m_threads.size(); i++)
		{
			if (m_threads[i]->Start() == false)
			{
				ret = false;
				break;
			}
		}

		if (ret = false)
		{
			for (size_t i = 0; i < m_threads.size(); i++)
			{
				if (m_threads[i]->Start() == false)
				{
					ret = false;
					break;
				}
			}
		}
		return ret;	
	}

	void Stop()
	{
		for (size_t i = 0; i < m_threads.size(); i++)
			m_threads[i]->Stop();
	}

	int DispatchWorker(const CThreadWorker& worker)//返回值-1表示分配失败, 所有线程都正在忙碌  返回值大于等于0代表成功分配线程
	{
		int index = -1;
		m_lock.lock();
		for (size_t i = 0; i < m_threads.size(); i++)
		{
			if (m_threads[i]->IsIdle())
			{
				m_threads[i]->UpdataWorker(worker);
				index = i;
				break;
			}
		}
		m_lock.unlock();
		return index;
	}

	bool CheckThreadValid(size_t index)
	{
		if (index < m_threads.size())
			return m_threads[index]->IsValid();
		return false;
	}
private:
	std::mutex m_lock;
	std::vector<CIQtestmachineThread*> m_threads;
};

