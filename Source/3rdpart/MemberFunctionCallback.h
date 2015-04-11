#ifndef IU_MEMBER_FUNCTION_CALLBACK_H
#define IU_MEMBER_FUNCTION_CALLBACK_H

#pragma once
#include <windows.h>
class CWebViewWindow;
typedef CWebViewWindow ClassWithCallback;
typedef LRESULT (CALLBACK *LPFN_CBTHookCallback)(int nCode, WPARAM wParam, LPARAM lParam);
typedef LRESULT (ClassWithCallback::*LPFN_CBTHookMemberFunctionCallback)(int nCode, WPARAM wParam, LPARAM lParam);

class CBTHookCallbackBase;
enum { kMaxCallbacks = 5};
extern  CBTHookCallbackBase* AvailableCallbackSlots[kMaxCallbacks];

// this object holds the state for a C++ member function callback in memory
class CBTHookCallbackBase
{
public:
    // input: pointer to a unique C callback. 
    CBTHookCallbackBase(LPFN_CBTHookCallback pCCallback)
        :	m_pClass( NULL ),
        m_pMethod( NULL ),
        m_pCCallback( pCCallback )
    {
    }

    // when done, remove allocation of the callback
    void Free()
    {
        m_pClass = NULL;
        // not clearing m_pMethod: it won't be used, since m_pClass is NULL and so this entry is marked as free
    }

    // when free, allocate this callback
    LPFN_CBTHookCallback Reserve(ClassWithCallback* instance, LPFN_CBTHookMemberFunctionCallback method)
    {
        if( m_pClass )
            return NULL;

        m_pClass = instance;
        m_pMethod = method;
        return m_pCCallback;
    }

protected:
    static LRESULT StaticInvoke(int context, int a, WPARAM b, LPARAM c) {
        return ((AvailableCallbackSlots[context]->m_pClass)->*(AvailableCallbackSlots[context]->m_pMethod))(a, b,c);
    }

private:
    LPFN_CBTHookCallback m_pCCallback;
    ClassWithCallback* m_pClass;
    LPFN_CBTHookMemberFunctionCallback m_pMethod;
};


template <int context> class DynamicCBTHookCallback : public CBTHookCallbackBase
{
public:
    DynamicCBTHookCallback()
        :	CBTHookCallbackBase(&DynamicCBTHookCallback<context>::GeneratedStaticFunction)
    {
    }

private:
    static LRESULT CALLBACK GeneratedStaticFunction(int a, WPARAM b, LPARAM c)
    {
        int ab = context;
        return StaticInvoke(context, a, b,c);
    }
};

class CBTHookMemberFunctionCallback
{
public:
    CBTHookMemberFunctionCallback(ClassWithCallback* instance, LPFN_CBTHookMemberFunctionCallback method);
    ~CBTHookMemberFunctionCallback();

public:
    operator LPFN_CBTHookCallback() const
    {
        return m_cbCallback;
    }

    bool IsValid() const
    {
        return m_cbCallback != NULL;
    }

private:
    LPFN_CBTHookCallback m_cbCallback;
    int m_nAllocIndex;

private:
    CBTHookMemberFunctionCallback( const CBTHookMemberFunctionCallback& os );
    CBTHookMemberFunctionCallback& operator=( const CBTHookMemberFunctionCallback& os );
};



#endif