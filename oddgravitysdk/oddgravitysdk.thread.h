/*----------------------------------------------------------------------------
| File:    oddgravitysdk.thread.h
| Project: oddgravitysdk
|
| Description:
|   Thread functions
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 363 $
| $Id: oddgravitysdk.thread.h 363 2008-07-07 19:28:22Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#ifndef __ODDGRAVITYSDK_THREAD_H__
#define __ODDGRAVITYSDK_THREAD_H__


namespace oddgravity
{

template<class DescendantType>
class CThread
{
protected:
    BOOL    m_bStop;
    BOOL    m_bRunning;

    HANDLE  m_hThread;
    HANDLE  ShutdownEvent;
    HANDLE  PauseEvent;

    enum
    {
        ThreadError,
        ThreadRunning,
        ThreadShutdown,
        ThreadTimeout,
    };

    typedef DWORD (WINAPI *THREADPROC)(LPVOID);

    static DWORD WINAPI StaticThreadProc(DescendantType* self)
    {
        self->m_bRunning = true;
        self->ThreadProc();
        self->m_bRunning = false;
        self->m_hThread = 0;
        ExitThread(0);
    }   

public:
    CThread()
    {
        m_hThread = 0;
        m_bRunning = false;
        m_bStop = false;

        ShutdownEvent = ::CreateEvent(
            NULL,   // security
            TRUE,   // manual-reset
            FALSE,  // not signaled
            NULL);  // anonymous

        PauseEvent = ::CreateEvent(
            NULL,   // security
            TRUE,   // manual-reset
            TRUE,   // signaled, means running
            NULL);  // anonymous
    }

    ~CThread()
    {
        if(ShutdownEvent)
        {
            ::CloseHandle(ShutdownEvent);
        }

        if(PauseEvent)
        {
            ::CloseHandle(PauseEvent);
        }

        if(m_hThread)
            Stop();
    }

    operator HANDLE()
    {
        return m_hThread;
    }

    BOOL StopRequested()
    {
        return m_bStop;
    }

    BOOL IsRunning()
    {
        return m_bRunning;
    }

    DWORD Wait()
    {
        HANDLE objects[2];
        objects[0] = ShutdownEvent;
        objects[1] = PauseEvent;
        DWORD result = ::WaitForMultipleObjects(2, objects, FALSE, INFINITE);
        switch(result)
        {
            case WAIT_TIMEOUT:
                return ThreadTimeout;
            case WAIT_OBJECT_0:
                return ThreadShutdown;
            case WAIT_OBJECT_0 + 1:
                return ThreadRunning;
            default:
                ASSERT(0);
                return ThreadError;
        }
    }

    //BOOL Wait(DWORD dwTimeOut=100)
    //{
    //  BOOL result = true;
    //  if(::WaitForSingleObject(m_hThread,dwTimeOut) == WAIT_TIMEOUT)
    //      result = false;
    //  return result;
    //}

    // override
    virtual void ThreadProc() = 0;

    void Start()
    {
        m_bStop = false;
        DWORD dwThreadID = 0;
        m_hThread = ::CreateThread(0, 0, (THREADPROC)StaticThreadProc, (DescendantType*)this, CREATE_SUSPENDED, &dwThreadID);
        ASSERT(m_hThread);
        if (m_hThread)
        {
            m_bRunning = true;
            DWORD dwResult = ::ResumeThread(m_hThread);
            ASSERT(dwResult != ((DWORD)-1));
        }
        //::SetThreadPriority(m_hThread,THREAD_PRIORITY_ABOVE_NORMAL);
    }   

    BOOL Stop(DWORD dwTimeOut=3000, BOOL bForce=FALSE)
    {
        m_bRunning = false;
        m_bStop = true;
        BOOL result = true;
        if(::WaitForSingleObject(m_hThread, dwTimeOut) == WAIT_TIMEOUT)
            result = false;
        if (bForce && !result)
        {
            ::TerminateThread(m_hThread, 0);
        }
        ::CloseHandle(m_hThread);
        m_hThread = 0;
        return result;
    }

    void Pause()
    {
        ::ResetEvent(PauseEvent);
    }

    void Resume()
    {
        ::SetEvent(PauseEvent);
    }

    void Shutdown()
    {
        ::SetEvent(ShutdownEvent);
    }
};

template<typename TObject, typename TFunc, typename TData>
class CCallMethodThread
    : public CThread<CCallMethodThread<TObject, TFunc, TData> >
{
protected:
    TObject*    m_pObject;
    TFunc       m_func;
    TData       m_data;

public:
    CCallMethodThread(TObject* pObject, TFunc func, const TData& data)
    {
        m_pObject = pObject;
        m_func = func;
        m_data = data;
    }

    void ThreadProc()
    {
        (m_pObject->*m_func)(m_data);
    }
};





}

#endif // __ODDGRAVITYSDK_THREAD_H__
