//
// Virtual Storage Simulator
//

#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <assert.h>
#include "sim.hpp"

////// Agent //////////////////////////////////////////////////////////////////////////

SimulatorTime Agent::GetTime()
{
    return g_pSim->GetTime();
}

void Agent::SetName(std::string name)
{
    m_name = name;
}

std::string Agent::GetName()
{
    return m_name;
}

void Agent::Log(std::string text, bool empty_strings)
{
    /* bb */
    std::string name_buffer = g_pSim->GetBuffer();
    if (name_buffer != m_name && empty_strings)
        std::cout << std::endl;
    /* bb end */

    PrintTime(&std::cout);
    std::cout << " " << std::setw(10) << std::setfill(' ') << std::right << GetName()
        << "   " << text << std::endl;

    g_pSim->SetBuffer(m_name); // bb
}

////// EventQueue //////////////////////////////////////////////////////////////////////////

EventQueue::EventQueue()
{
    m_pFirstEvent = NULL;
}

EventQueue::~EventQueue()
{
    while(m_pFirstEvent)
    {
        Event *pEvent = m_pFirstEvent->m_pNext;
        delete m_pFirstEvent;
        m_pFirstEvent = pEvent;
    }
}

uint64_t EventQueue::Schedule(Event *pEvent)
{
    if(m_pFirstEvent == NULL || m_pFirstEvent->m_time > pEvent->m_time)
    {
        pEvent->m_pNext = m_pFirstEvent;
        m_pFirstEvent = pEvent;
    }
    else
    {
        Event *pCurrentEvent = m_pFirstEvent;
        Event *pPrevEvent = NULL;
        while(pCurrentEvent != NULL && pCurrentEvent->m_time <= pEvent->m_time)
        {
            pPrevEvent = pCurrentEvent;
            pCurrentEvent = pCurrentEvent->m_pNext;
        }
        if(pCurrentEvent != NULL)
        {
            assert(pPrevEvent);
            pPrevEvent->m_pNext = pEvent;
            pEvent->m_pNext = pCurrentEvent;
        }
        else
        {
            pPrevEvent->m_pNext = pEvent;
        }
    }

    return reinterpret_cast<uint64_t>(pEvent);
}

bool EventQueue::CancelEvent(uint64_t handle)
{
    Event *pEvent = reinterpret_cast<Event *>(handle);

    if(m_pFirstEvent == pEvent)
    {
        m_pFirstEvent = m_pFirstEvent->m_pNext;
        delete pEvent;
        return true;
    }
    else
    {
        for(Event *pCurrentEvent = m_pFirstEvent; pCurrentEvent->m_pNext != NULL; pCurrentEvent = pCurrentEvent->m_pNext)
        {
            if(pCurrentEvent->m_pNext == pEvent)
            {
                pCurrentEvent->m_pNext = pCurrentEvent->m_pNext->m_pNext;
                delete pEvent;
                return true;
            }
        }
    }

    return false;
}

////// Sim //////////////////////////////////////////////////////////////////////////

Sim::Sim()
{
    m_Time = 0;
    m_Limit = 86400ULL*1000*1000*1000;         // одни сутки
}

Sim::~Sim()
{
}

void Sim::SetLimit(SimulatorTime limit)
{
    m_Limit = limit;
}

SimulatorTime Sim::GetLimit()
{
    return m_Limit;
}

SimulatorTime& Sim::GetTime()
{
    return m_Time;
}

bool Sim::Run()
{
    Event *pEvent;
    while((pEvent = GetHead()))
    {
        if(pEvent->m_time >= GetLimit())
        {
            m_Time = GetLimit();
            break;
        }
        RemoveHead();
        m_Time = pEvent->m_time;
        pEvent->Notify();
        delete pEvent;
    }
    return IsEmpty();
}

////// Global //////////////////////////////////////////////////////////////////////////

bool CancelEvent(uint64_t handle)
{
    return g_pSim->CancelEvent(handle);
}

void PrintTime(std::ostream *str)
{
    SimulatorTime now = g_pSim->GetTime();
    uint32_t seconds = now/(1000*1000*1000);
    uint16_t ms = (now%(1000*1000*1000))/(1000*1000);
    uint16_t us = (now%(1000*1000))/1000;
    uint16_t ns = now%1000;
    (*str) << seconds
        << ":" << std::setw(3) << std::setfill('0') << std::right << ms
        << "." << std::setw(3) << std::setfill('0') << std::right << us
        << "." << std::setw(3) << std::setfill('0') << std::right << ns
        << " "
    ;
}
