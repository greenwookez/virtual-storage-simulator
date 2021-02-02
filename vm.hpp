#include <vector>
#include <ctime>
#include "sim.hpp"
using namespace std;




int InitializeInputData();

//TYPES
typedef uint64_t PageNumber;
typedef PageNumber VirtualAddress;
typedef PageNumber RealAddress;
typedef PageNumber DiskAddress;


//HEADER FILE
class AgentVM : public Agent {
    public:
    void Log(string text); // Переопределение логирования для вывода загруженности системы в конце каждого сообщения
};

struct RequestStruct;
class Process;

struct TTStruct {
    VirtualAddress vaddress;
    RealAddress raddress;
    bool is_valid;
};

class TT {
    Process* p_process;

    vector <TTStruct> records;

public:
    TT(Process* _p_process, PageNumber size);

    TTStruct& GetRecord(VirtualAddress vaddress);
    int GetSize();
    Process* GetProcess();
};

class RAM {
    vector <bool> ram; // true means already in use
public:
    RAM();
    bool GetRealAddress(PageNumber raddress);
    void SetRealAddress(PageNumber raddress, bool value);
    int GetSize();
};

class Requester {
    vector <RequestStruct> request_queue;
public:
    void AddRequest(Process* p_process, VirtualAddress vaddress, RealAddress raddress, bool load_flag, Process * p_initialProcess);
    void DeleteRequest(Process* p_process, VirtualAddress vaddress);
    RequestStruct GetRequest();
    bool IsEmpty();
    int GetQueueSize() {
        return request_queue.size();
    };

    void PrintQueue();
};

class Scheduler {
    vector <Process*> process_queue;
public:
    void AddProcess(Process* p_process);
    void DeleteProcess(Process* p_process);
    void PutInTheEnd();
    Process* GetProcess();
    bool IsEmpty();
    void PrintQueue();
};

class OS : public AgentVM {
    vector <TT> translation_tables;
    RAM ram;
    Requester requester;
    Scheduler scheduler;

public:
    OS();
    void HandelInterruption(VirtualAddress vaddress, RealAddress raddress, Process* p_process);
    void LoadProcess(Process* _p_process);
    void Allocate(VirtualAddress vaddress, Process* p_process);
    void Substitute(VirtualAddress vaddress, Process* p_process);
    TT& FindTT(Process* p_process);
    Requester& GetRequester();
    Scheduler& GetScheduler();
    void ProcessQueue();
    void ChangeQueue();

    //For logging
    float ComputeRML();

    void Work();
    void Wait();
    void Start();
};

class CPU : public AgentVM {
public:
    CPU();
    void Convert(VirtualAddress vaddress, Process* p_process);

    void Work();
    void Wait();
    void Start();
};

class DiskSpace {
    vector <bool> disk;  // true means already in use
public:
    DiskSpace();
    bool GetDiskAddress(PageNumber daddress);
    void SetDiskAddress(PageNumber daddress, bool value);
    int GetSize();
};

class AE : public AgentVM {
    DiskSpace disk;

    struct SwapIndexStruct {
        Process* p_process;
        VirtualAddress vaddress;
        DiskAddress daddress;
    };

    vector <SwapIndexStruct> SwapIndex;

    void LoadData();
    void PopData();

    SimulatorTime io_total_time;


public:
    SimulatorTime last_time;
    uint64_t io_count;
    AE();
    void ProcessRequest();
    bool IsLoaded(Process* p_process, VirtualAddress vaddress);
    SimulatorTime GetIOTT();


    //For logging
    float ComputeAEL();
    double ComputePSL();


    void Work();
    void Wait();
    void Start();
};

class Process : public AgentVM {
    PageNumber requested_memory;
    SimulatorTime time_limit;
    bool isWaiting;
public:
    Process();
    void MemoryRequest(VirtualAddress vaddress);
    void SetRequestedMemory(uint64_t value);
    uint64_t GetRequestedMemory();
    void SetTimeLimit(SimulatorTime value);
    SimulatorTime GetTimeLimit();

    void setWaiting() {
      isWaiting = true;
    };

    void setWorking() {
      isWaiting = false;
    };

    bool getWaitingState() {
      return isWaiting;
    };

    void Work();
    void Wait();
    void Start();


};

int randomizer(int max);

extern OS* g_pOS;
extern CPU* g_pCPU;
extern AE* g_pAE;

struct RequestStruct {
    bool load_flag;
    Process* p_process;
    VirtualAddress vaddress;
    RealAddress raddress;

    Process* p_initialProcess;
};
