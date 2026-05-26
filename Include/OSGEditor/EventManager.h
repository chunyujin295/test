#pragma once
#include "OSGEditor/Base.h"
#include <map>
using namespace std;

//class OsgEngine;

class DLL_API EventInfo
{

public:
    EventInfo::EventInfo();
    EventInfo(const CALLBACK_EVENT_TYPE& type, void* arg):m_type(type), m_para(arg) {};
    virtual ~EventInfo() {};

    void setEventInfo(const CALLBACK_EVENT_TYPE& type, void* arg) { m_type = type; m_para = arg; };
    void* getEventInfo() const { return m_para; };
    const CALLBACK_EVENT_TYPE getEventType() const { return  m_type; };

private:
    CALLBACK_EVENT_TYPE m_type;
    void* m_para;

};

//回调事件注册基类，子类继承并实现接口
class DLL_API EventBaseServer
{
public:
    EventBaseServer() { pOsgEngine = nullptr; }
    virtual ~EventBaseServer() {};

    //virtual void callBackEvent(CALLBACK_EVENT_TYPE, const EventInfo&) = 0;
    virtual void callBackEvent(CALLBACK_EVENT_TYPE, const EventInfo&) {}

public:
    void* pOsgEngine;
};

class DLL_API EventManager
{

public:
    EventBaseServer*m_observer;
    static void deleteInstance();

    EventManager();
    ~EventManager();
    virtual void registerEvent(CALLBACK_EVENT_TYPE , EventBaseServer*,void *pOsgEngine = nullptr);
    virtual void removeEvent(CALLBACK_EVENT_TYPE);
    virtual void notifyEvent(const EventInfo &,void* pOsgEngine = nullptr);
    static EventManager * GetInstance();
    void Release();
private:
    static EventManager*m_pInstance;
    multimap<CALLBACK_EVENT_TYPE, EventBaseServer*>  map_event;
};

