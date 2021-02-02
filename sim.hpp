//
// Virtual Storage Simulator
//

#include <stdint.h> //bb
#include <assert.h> //bb
#include <string> //bb

typedef uint64_t SimulatorTime;

class Sim;

class Agent
{
public:
    Agent() {}
    virtual ~Agent() {}

    virtual void Start() = 0;
    SimulatorTime GetTime();
    void SetName(std::string str);
    std::string GetName();
    virtual void Log(std::string text, bool empty_strings);

private:
    std::string m_name;
};

class Event
{
public:
    Event() { m_pNext = NULL; }
    virtual ~Event() {}

    SimulatorTime m_time;
    Event *m_pNext;

    virtual void Notify() {}
};

class EventQueue
{
public:
    EventQueue();
    ~EventQueue();

    uint64_t Schedule(Event *pEvent);
    bool CancelEvent(uint64_t handle);
    bool IsEmpty() { return m_pFirstEvent==NULL; }
    Event *GetHead() { return m_pFirstEvent; }
    void RemoveHead()
    {
        if(m_pFirstEvent)
        {
            m_pFirstEvent = m_pFirstEvent->m_pNext;
        }
    }

private:
    Event *m_pFirstEvent;
};

class Sim : public EventQueue
{
public:
    Sim();
    ~Sim();

    void SetLimit(SimulatorTime limit);
    SimulatorTime GetLimit();

    SimulatorTime & GetTime(); // bb -> &
    bool Run();

    void SetBuffer(std::string input) { buffer = input; }; // bb
    std::string GetBuffer() { return buffer; }; // bb

private:
    SimulatorTime m_Time;
    SimulatorTime m_Limit;
    std::string buffer; // bb: буффер для имени последнего агента в логе
};

extern Sim *g_pSim;

// обработчик без параметров
template <typename METHOD, typename OBJECT>
uint64_t Schedule(SimulatorTime time, OBJECT obj, METHOD met)
{
    assert(time >= g_pSim->GetTime());
    class EventImpl : public Event
    {
    public:
        EventImpl(SimulatorTime time, OBJECT obj, METHOD met)
        {
            m_time = time;
            m_obj = obj;
            m_met = met;
        }
        virtual ~EventImpl() {}

        OBJECT m_obj;
        METHOD m_met;

        void Notify()
        {
            (m_obj->*m_met)();
        }
    } *pEvent = new EventImpl(time, obj, met);
    // g_pSim->GetTime() = time;
    return g_pSim->Schedule(pEvent);
}

// обработчик с одним параметром
template <typename METHOD, typename OBJECT, typename PARAM1>
uint64_t Schedule(SimulatorTime time, OBJECT obj, METHOD met, PARAM1 value1)
{
    assert(time >= g_pSim->GetTime());
    class EventImpl : public Event
    {
    public:
        EventImpl(SimulatorTime time, OBJECT obj, METHOD met, PARAM1 value1)
        {
            m_time = time;
            m_obj = obj;
            m_met = met;
            m_value1 = value1;
        }
        virtual ~EventImpl() {}

        OBJECT m_obj;
        METHOD m_met;
        PARAM1 m_value1;

        void Notify()
        {
            (m_obj->*m_met)(m_value1);
        }
    } *pEvent = new EventImpl(time, obj, met, value1);
    // g_pSim->GetTime() = time;
    return g_pSim->Schedule(pEvent);
}

// обработчик с двумя параметрами
template <typename METHOD, typename OBJECT, typename PARAM1, typename PARAM2>
uint64_t Schedule(SimulatorTime time, OBJECT obj, METHOD met, PARAM1 value1, PARAM2 value2)
{
    assert(time >= g_pSim->GetTime());
    class EventImpl : public Event
    {
    public:
        EventImpl(SimulatorTime time, OBJECT obj, METHOD met, PARAM1 value1, PARAM2 value2)
        {
            m_time = time;
            m_obj = obj;
            m_met = met;
            m_value1 = value1;
            m_value2 = value2;
        }
        virtual ~EventImpl() {}

        OBJECT m_obj;
        METHOD m_met;
        PARAM1 m_value1;
        PARAM2 m_value2;

        void Notify()
        {
            (m_obj->*m_met)(m_value1, m_value2);
        }
    } *pEvent = new EventImpl(time, obj, met, value1, value2);
    // g_pSim->GetTime() = time;
    return g_pSim->Schedule(pEvent);
}

// обработчик с тремя параметрами
template <typename METHOD, typename OBJECT, typename PARAM1, typename PARAM2, typename PARAM3>
uint64_t Schedule(SimulatorTime time, OBJECT obj, METHOD met, PARAM1 value1, PARAM2 value2, PARAM3 value3)
{
    assert(time >= g_pSim->GetTime());
    class EventImpl : public Event
    {
    public:
        EventImpl(SimulatorTime time, OBJECT obj, METHOD met, PARAM1 value1, PARAM2 value2, PARAM3 value3)
        {
            m_time = time;
            m_obj = obj;
            m_met = met;
            m_value1 = value1;
            m_value2 = value2;
            m_value3 = value3;
        }
        virtual ~EventImpl() {}

        OBJECT m_obj;
        METHOD m_met;
        PARAM1 m_value1;
        PARAM2 m_value2;
        PARAM3 m_value3;

        void Notify()
        {
            (m_obj->*m_met)(m_value1, m_value2, m_value3);
        }
    } *pEvent = new EventImpl(time, obj, met, value1, value2, value3);
    // g_pSim->GetTime() = time;
    return g_pSim->Schedule(pEvent);
}

// обработчик с четырьмя параметрами
template <typename METHOD, typename OBJECT, typename PARAM1, typename PARAM2, typename PARAM3, typename PARAM4>
uint64_t Schedule(SimulatorTime time, OBJECT obj, METHOD met, PARAM1 value1, PARAM2 value2, PARAM3 value3, PARAM4 value4)
{
    assert(time >= g_pSim->GetTime());
    class EventImpl : public Event
    {
    public:
        EventImpl(SimulatorTime time, OBJECT obj, METHOD met, PARAM1 value1, PARAM2 value2, PARAM3 value3, PARAM4 value4)
        {
            m_time = time;
            m_obj = obj;
            m_met = met;
            m_value1 = value1;
            m_value2 = value2;
            m_value3 = value3;
            m_value4 = value4;
        }
        virtual ~EventImpl() {}

        OBJECT m_obj;
        METHOD m_met;
        PARAM1 m_value1;
        PARAM2 m_value2;
        PARAM3 m_value3;
        PARAM4 m_value4;

        void Notify()
        {
            (m_obj->*m_met)(m_value1, m_value2, m_value3, m_value4);
        }
    } *pEvent = new EventImpl(time, obj, met, value1, value2, value3, value4);
    // g_pSim->GetTime() = time;
    return g_pSim->Schedule(pEvent);
}

bool CancelEvent(uint64_t handle);

void PrintTime(std::ostream *str);
