///////////////////////////////////////////////////////////
//  EventManager.cpp
//  Implementation of the Class EventManager
//  Created on:      16-九月-2014 15:27:25
//  Original author: Administrator
///////////////////////////////////////////////////////////

//#include "OSGEditor/OsgEngine.h"
#include <OSGEditor/EventManager.h>

EventManager * EventManager::m_pInstance = NULL;

EventManager::EventManager(){
    
}


EventManager::~EventManager(){
    
}

void EventManager::deleteInstance()
{
    if (m_pInstance != NULL)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

void EventManager::Release()
{
    map_event.erase(map_event.begin(), map_event.end());
    map_event.clear();
}

void EventManager::registerEvent(CALLBACK_EVENT_TYPE eventType, EventBaseServer*obs,void *pOsgEngine)
{
    if (obs != nullptr && pOsgEngine != nullptr && obs->pOsgEngine == nullptr)
        obs->pOsgEngine = pOsgEngine;
    map_event.insert(std::make_pair(eventType, obs));
}

// note:!though unused now,need to modify it when using it later.
void EventManager::removeEvent(CALLBACK_EVENT_TYPE eventType)
{
    if (map_event.find(eventType) != map_event.end())
    {
        map_event.erase(eventType);       
    }
}

void EventManager::notifyEvent(const EventInfo &info,void* pOsgEngine)
{
    if (map_event.find(info.getEventType()) != map_event.end())
    {
        ///map_event.find(info.getEventType())->second->callBackEvent(info.getEventType(), info);
        for (auto& t : map_event)
        {
            if (t.first == info.getEventType())
            {
                if (pOsgEngine != nullptr && pOsgEngine == t.second->pOsgEngine)
                {
                    t.second->callBackEvent(info.getEventType(), info);
                    break;
                }
                else if (pOsgEngine == nullptr)
                {
                    t.second->callBackEvent(info.getEventType(), info);
                    break;
                }
            }
        }
    }
}

 EventManager * EventManager::GetInstance()
 {
    if (m_pInstance == NULL)
    {
        m_pInstance = new EventManager();
    }

    return  m_pInstance;
}

