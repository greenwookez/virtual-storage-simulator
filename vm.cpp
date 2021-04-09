#include "vm.hpp"
#include <iostream>
#include <iomanip>
#include <random>
#include <fstream>

using namespace std;

struct RequestStruct;

//GVARs
extern Sim *g_pSim;
extern OS *g_pOS;
extern CPU *g_pCPU;
extern AE *g_pAE;

//CONSTs
const SimulatorTime nanoSec = 1;
const SimulatorTime microSec = 1000000 * nanoSec;
const SimulatorTime Sec = 1000000000;
const SimulatorTime Minute = Sec * 60;
const SimulatorTime Hour = Minute * 60;
const uint64_t msg_space = 77;

//CONFIG
int CONFIG_LOG_ENABLE_EMPTY_STRINGS = 1; // включает пустые строки в логе 1 - выключает, 0 включает
int CONFIG_LOG_DETAIL_LEVEL = 2; // степень подробности логирования
// 3 - максимально подробное логирования
// 2 - отображаются только сообщения о прерываниях и все действия ОС по обработке этих прерываний
// 1 - отображаются только действия АС
SimulatorTime CONFIG_SIM_TIME_LIMIT = Hour; // лимит времени работы симулятора

SimulatorTime AE_DEFAULT_TIME_FOR_DATA_IO = 10*microSec; // время работы устройства ввода/ввывода
uint64_t AE_DEFAULT_DISKSPACE_SIZE = 2000; // размер файла подкачки в страницах

SimulatorTime CPU_DEFAULT_TIME_FOR_CONVERSION = 1; // время на преобразование адреса процессом

SimulatorTime OS_DEFAULT_PROCESS_QUEUE_TIME_LIMIT = 10000; // время, на которое процессу дается ЦП (потом ЦП передается другуму претенденту в очереди)
uint64_t OS_DEFAULT_RAM_SIZE = 700; // размер ОП в страницах
uint64_t OS_DEFAULT_TIME_FOR_ALLOCATION = 10*microSec; // время на размещение
int OS_SUBSTITUTE_STRATEGY = 1; // стратегия выбора кандидата на перераспределение

SimulatorTime PROCESS_DEFAULT_WORK_TIME = 1; // время, за которое процесс совершает единицу работы
uint64_t PROCESS_DEFAULT_REQUESTED_MEMORY = 40; // память, необходимая процессу в количестве страниц
int PROCESS_AMOUNT = 15; // количество процессов
int PROCESS_MEMORY_ACCESS_PERCENTAGE = 40; // процент инструкций процесса, которые требуют обращения в память

bool DEBUG_MODE = false;
extern ofstream fileout;

// Substitue strategies
RealAddress RandomSelectionStrategy() {
    return static_cast<RealAddress>(randomizer(OS_DEFAULT_RAM_SIZE));
}

RealAddress ClockSelectionStrategy() {
    return g_pOS->GetClock()->Tick();
}

int InitializeInputData() {
    ifstream input_data;
    input_data.open("input_data.txt");
    string read_buffer;
    if (input_data.is_open()) {
        while (getline (input_data,read_buffer))
        {
            int eq_pos = 0;
            for (int i = 0; i < read_buffer.size(); i++) {
                if (read_buffer[i] == '=') {
                    eq_pos = i;
                    break;
                }
            }

            if (read_buffer.substr(0,eq_pos) == "CONFIG_LOG_ENABLE_EMPTY_STRINGS") {
                CONFIG_LOG_ENABLE_EMPTY_STRINGS = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "CONFIG_LOG_DETAIL_LEVEL") {
                CONFIG_LOG_DETAIL_LEVEL = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "CONFIG_SIM_TIME_LIMIT") {
                CONFIG_SIM_TIME_LIMIT = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "AE_DEFAULT_TIME_FOR_DATA_IO") {
                AE_DEFAULT_TIME_FOR_DATA_IO = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "AE_DEFAULT_DISKSPACE_SIZE") {
                AE_DEFAULT_DISKSPACE_SIZE = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "CPU_DEFAULT_TIME_FOR_CONVERSION") {
                CPU_DEFAULT_TIME_FOR_CONVERSION = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "OS_DEFAULT_PROCESS_QUEUE_TIME_LIMIT") {
                OS_DEFAULT_PROCESS_QUEUE_TIME_LIMIT = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "OS_DEFAULT_RAM_SIZE") {
                OS_DEFAULT_RAM_SIZE = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "OS_DEFAULT_TIME_FOR_ALLOCATION") {
                OS_DEFAULT_TIME_FOR_ALLOCATION = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "OS_SUBSTITUTE_STRATEGY") {
                OS_SUBSTITUTE_STRATEGY = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "PROCESS_DEFAULT_WORK_TIME") {
                PROCESS_DEFAULT_WORK_TIME = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "PROCESS_DEFAULT_REQUESTED_MEMORY") {
                PROCESS_DEFAULT_REQUESTED_MEMORY = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "PROCESS_AMOUNT") {
                PROCESS_AMOUNT = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            } else if (read_buffer.substr(0,eq_pos) == "PROCESS_MEMORY_ACCESS_PERCENTAGE") {
                PROCESS_MEMORY_ACCESS_PERCENTAGE = stoll(read_buffer.substr(eq_pos+1, read_buffer.size()));
            };
        }
        input_data.close();
        cout << "                   Current configuration:" << endl;
        cout << setw(40) << left << "CONFIG_LOG_ENABLE_EMPTY_STRINGS = " << setw(20) << right << CONFIG_LOG_ENABLE_EMPTY_STRINGS << endl;
        cout << setw(40) << left << "CONFIG_LOG_DETAIL_LEVEL = " << setw(20) << right  << CONFIG_LOG_DETAIL_LEVEL << endl;
        cout << setw(40) << left << "CONFIG_SIM_TIME_LIMIT = " << setw(20) << right  << CONFIG_SIM_TIME_LIMIT << endl;
        cout << setw(40) << left << "AE_DEFAULT_TIME_FOR_DATA_IO = " << setw(20) << right  << AE_DEFAULT_TIME_FOR_DATA_IO << endl;
        cout << setw(40) << left << "AE_DEFAULT_DISKSPACE_SIZE = " << setw(20) << right  << AE_DEFAULT_DISKSPACE_SIZE << endl;
        cout << setw(40) << left << "CPU_DEFAULT_TIME_FOR_CONVERSION = " << setw(20) << right  << CPU_DEFAULT_TIME_FOR_CONVERSION << endl;
        cout << setw(40) << left << "OS_DEFAULT_PROCESS_QUEUE_TIME_LIMIT = " << setw(20) << right  << OS_DEFAULT_PROCESS_QUEUE_TIME_LIMIT << endl;
        cout << setw(40) << left << "OS_DEFAULT_RAM_SIZE = " << setw(20) << right  << OS_DEFAULT_RAM_SIZE << endl;
        cout << setw(40) << left << "OS_DEFAULT_TIME_FOR_ALLOCATION = " << setw(20) << right  << OS_DEFAULT_TIME_FOR_ALLOCATION << endl;
        cout << setw(40) << left << "OS_SUBSTITUTE_STRATEGY = " << setw(20) << right  << OS_SUBSTITUTE_STRATEGY << endl;
        cout << setw(40) << left << "PROCESS_DEFAULT_WORK_TIME = " << setw(20) << right  << PROCESS_DEFAULT_WORK_TIME << endl;
        cout << setw(40) << left << "PROCESS_DEFAULT_REQUESTED_MEMORY = " << setw(20) << right  << PROCESS_DEFAULT_REQUESTED_MEMORY << endl;
        cout << setw(40) << left << "PROCESS_AMOUNT = " << setw(20) << right  << PROCESS_AMOUNT << endl;
        cout << setw(40) << left << "PROCESS_MEMORY_ACCESS_PERCENTAGE = " << setw(20) << right  << PROCESS_MEMORY_ACCESS_PERCENTAGE << endl;
    } else {
        cout << "ERROR: COULD NOT OPEN INPUT DATA FILE" << endl;
        return 1;
    }
    return 0;
}

void AgentVM :: Log(string text) {
    string tail =
    " || RML=" + to_string(g_pOS->ComputeRML()).substr(0,5) +
    " AEL=" + to_string(g_pAE->ComputeAEL()).substr(0,5) +
    " PSL="  + to_string(g_pAE->ComputePSL()).substr(0,5);
    Agent::Log(text + string(msg_space-text.length(), ' ') + tail, CONFIG_LOG_ENABLE_EMPTY_STRINGS);
}

TT::TT(Process* _p_process, PageNumber size) {
    for (int i = 0; i < static_cast<int>(size); i++) {
        TTStruct tmp_struct = { static_cast<VirtualAddress>(i), 0, false, false, false};
        records.push_back(tmp_struct);
    }
    p_process = _p_process;
}

TTStruct& TT::GetRecord(VirtualAddress vaddress) {
    for (int i = 0; i < static_cast<int>(records.size()); i++) {
        if (records[i].vaddress == vaddress) {
            return records[i];
        }
    }

    __throw_logic_error("RECORD NOT FOUND");
}

TTStruct& TT::GetRecordByIndex(uint32_t i) {
    if (i < records.size()) {
        return records[i];
    }

    __throw_logic_error("RECORD NOT FOUND BY INDEX");
}

int TT::GetSize() {
    return static_cast<int>(records.size());
}

Process* TT::GetProcess() {
    return p_process;
}

RAM::RAM() {
    for (int i = 0; i < static_cast<int>(OS_DEFAULT_RAM_SIZE); i++) {
        ram.push_back(false);
    }
}

bool RAM::GetRealAddress(PageNumber raddress) {
    return ram[raddress];
}

void RAM::SetRealAddress(PageNumber raddress, bool value) {
    ram[raddress] = value;
}

int RAM::GetSize() {
    return ram.size();
}

void Requester :: AddRequest(Process* p_process, VirtualAddress vaddress, RealAddress raddress, bool load_flag, Process * p_initialProcess) {
    if (IsEmpty()) {
        if (g_pSim->GetTime() - g_pAE->last_time >= AE_DEFAULT_TIME_FOR_DATA_IO) {
            Schedule(g_pSim->GetTime(), g_pAE, &AE::ProcessRequest);
        };
    }
    if (CONFIG_LOG_DETAIL_LEVEL >= 2) {
        string tmp =
        " || RML=" + to_string(g_pOS->ComputeRML()).substr(0,5) +
        " AEL=" + to_string(g_pAE->ComputeAEL()).substr(0,5) +
        " PSL="  + to_string(g_pAE->ComputePSL()).substr(0,5);

        string text = "      New request (" + p_process->GetName() + " VA="
        + string(4 - to_string(vaddress).length(), '0') + to_string(vaddress)
        + " RA=" + string(4 - to_string(raddress).length(), '0') + to_string(raddress)
        + " LF=" + to_string(load_flag) + " IP=" + p_initialProcess->GetName() + ")";

        string tail = string(msg_space-text.length(), ' ') + tmp;

        PrintTime(&std::cout);
        std::cout << " " << std::setw(10) << std::setfill(' ') << std::right << "Requester"
            << "   " << text << tail << std::endl;
    }
    RequestStruct tmp_struct = { load_flag, p_process, vaddress, raddress, p_initialProcess };
    request_queue.push_back(tmp_struct);
}

void Requester::DeleteRequest(Process* p_process, VirtualAddress vaddress) {
    for (int i = 0; i < static_cast<int>(request_queue.size()); i++) {
        if (request_queue[i].p_process == p_process && request_queue[i].vaddress == vaddress) {
            if (CONFIG_LOG_DETAIL_LEVEL >= 2) {
                string tmp =
                " || RML=" + to_string(g_pOS->ComputeRML()).substr(0,5) +
                " AEL=" + to_string(g_pAE->ComputeAEL()).substr(0,5) +
                " PSL="  + to_string(g_pAE->ComputePSL()).substr(0,5);

                string text = "      Delete request (" + request_queue[0].p_process->GetName() + " VA="
                + string(4 - to_string(request_queue[0].vaddress).length(), '0') + to_string(request_queue[0].vaddress)
                + " RA=" + string(4 - to_string(request_queue[0].raddress).length(), '0') + to_string(request_queue[0].raddress)
                + " LF=" + to_string(request_queue[0].load_flag) + ")";

                string tail = string(msg_space-text.length(), ' ') + tmp;

                PrintTime(&std::cout);
                std::cout << " " << std::setw(10) << std::setfill(' ') << std::right << "Requester"
                    << "   " << text << tail << std::endl;
            }

            request_queue.erase(request_queue.begin() + i);
            return;
        }
    }

    __throw_logic_error("REQUEST NOT FOUND");
}

RequestStruct Requester::GetRequest() {
    return request_queue[0];
}

bool Requester::IsEmpty() {
    return request_queue.empty();
}

void Requester::PrintQueue() {
    if (request_queue.size() == 0) {
        std::cout << "Request Queue is Empty." << endl;
    } else {
        for (int i = 0; i < static_cast<int>(request_queue.size()); i++) {
            std::cout << "{" << request_queue[i].load_flag << "} {" <<
            request_queue[i].p_process << "} {" << request_queue[i].vaddress <<
            "} {" << request_queue[i].raddress << "}" << endl;
        }
    }
}

void Scheduler::AddProcess(Process* p_process) {
    if (DEBUG_MODE) cout << "Adding " << p_process->GetName() << endl;
    
    if (IsEmpty()) {
        // Если очередь пуста, планируем событие обработки этой очереди
        if (DEBUG_MODE) cout << "from AddProcess ";
        Schedule(g_pSim->GetTime(), g_pOS, &OS::ProcessQueue);
    }

    // Добавляем процесс в очередь кандидатов
    process_queue.push_back(p_process);
}

void Scheduler::DeleteProcess(Process* p_process) {
    if (DEBUG_MODE) cout << "Deleting " << p_process->GetName() << endl;
    process_queue.erase(process_queue.begin());
}

void Scheduler::PutInTheEnd() {
    // Ставим первый процесс в очереди в конец этой очереди
    // (Здесь речь идет об очереди кандидатов на ЦП)
    Process* tmp = process_queue[0];
    process_queue.erase(process_queue.begin());
    process_queue.push_back(tmp);
}

Process* Scheduler::GetProcess() {
    // Возвращаем указатель на первый в очереди процесс
    return process_queue[0];
}

bool Scheduler::IsEmpty() {
    return process_queue.empty();
}

void Scheduler::PrintQueue() {
    if (process_queue.size() == 0) {
        std::cout << "Request Queue is Empty." << endl;
    } else {
        for (int i = 0; i < static_cast<int>(process_queue.size()); i++) {
            string name = process_queue[i]->GetName();
            std::cout << "{" << name.substr(name.length() - 2, name.length()) << "}";
        }
    }
    std::cout << endl;
}

void OS::ProcessQueue() {
    Process *p_process = scheduler.GetProcess();
    if (DEBUG_MODE) {
        cout << "Processing queue (" << p_process->GetName() << "):    ";
        scheduler.PrintQueue();
    }
    // Событие обработки очереди кандидатов на ЦП
    // Устанавливаем лимит по времени на работу одного процесса с ЦП
    p_process->SetTimeLimit(OS_DEFAULT_PROCESS_QUEUE_TIME_LIMIT);
    // Планируем саму работу процесса
    Schedule(GetTime(), p_process, &Process::Work);
}

void OS::ChangeQueue() {
    if (DEBUG_MODE) {
        cout << "Changing queue (from "
        << scheduler.GetProcess()->GetName() << "("
        << scheduler.GetProcess()->GetTimeLimit() << ")" << " to ";
    }
    scheduler.GetProcess()->SetTimeLimit(0);
    scheduler.PutInTheEnd();
    if (DEBUG_MODE) {
        cout << scheduler.GetProcess()->GetName() << "("
        << scheduler.GetProcess()->GetTimeLimit() << ")" << ")" << endl;
    }
    if (DEBUG_MODE) cout << "from ChangeQueue ";
    Schedule(GetTime(), g_pOS, &OS::ProcessQueue);
}

OS::OS() {
    SetName("OS");
    if (OS_SUBSTITUTE_STRATEGY == 2) {
        clock = new Clock;
    }
}

OS::~OS() {
    delete clock;
}

void OS::HandelInterruption(VirtualAddress vaddress, RealAddress raddress,  Process* p_process) {
    // Если по виртуальному адресу vaddress есть данные в АС, необходимо их
    // выгрузить

    if (g_pAE->IsLoaded(p_process, vaddress)) {
        requester.AddRequest(p_process, vaddress, raddress, false, p_process);

    }

    Allocate(vaddress, p_process);
}

void OS::LoadProcess(Process* p_process) {
    // Создаём новую ТП для процесса
    translation_tables.push_back(TT(p_process, p_process->GetRequestedMemory()));

    // Добавляем этот процесс в очередь претендентов на ЦП
    scheduler.AddProcess(p_process);
}

void OS::Allocate(VirtualAddress vaddress, Process* p_process) {
    for (int i = 0; i < static_cast<int>(ram.GetSize()); i++) {
        if (ram.GetRealAddress(i) == false) {
            // Устанавливаем флаг распределенности для найденного
            ram.SetRealAddress(i, true);

            TTStruct & tt = FindTT(p_process).GetRecord(vaddress);
            // Вносим изменение в ТП процесса о новом соответствии виртуального адреса
            // реальному. А также о том, что реальный адрес действителен
            tt.raddress = i;
            tt.is_valid = true;

            Process* scheduler_process = g_pOS->GetScheduler().GetProcess();

            g_pSim->GetTime() = g_pSim->GetTime() + OS_DEFAULT_TIME_FOR_ALLOCATION;
            if (scheduler_process->GetTimeLimit() > 0) {
                if (scheduler_process->getWaitingState() == false) {
                  Schedule(GetTime(), scheduler_process, &Process::Work);
                }
            } else {
                if (DEBUG_MODE) cout << "from Allocate ";
                //Schedule(GetTime(), g_pOS, &OS::ChangeQueue);
                ChangeQueue();
            }

            if (CONFIG_LOG_DETAIL_LEVEL >= 2) {
                Log("  Allocate RA=" + string(4 - to_string(i).length(), '0') + to_string(i) + " as VA=" + string(4 - to_string(vaddress).length(), '0') + to_string(vaddress) + " " + p_process->GetName() + ", resume");
            }
            return;
        }
    }
    // Если нераспределенного реального адреса найдено не было, необходимо перераспределение
    // существующих реальных адресов. Вызываем соответсвующую подпрограмму
    Substitute(vaddress, p_process);
}

void OS::Substitute(VirtualAddress vaddress, Process* p_process) {
    RealAddress candidate_raddress;
    Process* candidate_process;
    VirtualAddress candidate_vaddress;

    // Ввести процесс в состояние ожидания и вытащить его из очереди

    p_process->setWaiting(); // Вводим в состояние ожидания
    scheduler.DeleteProcess(p_process);  // Удаляем из очереди претендентов
    if (DEBUG_MODE) cout << "Setting limit for " << scheduler.GetProcess()->GetName() << endl;
    scheduler.GetProcess()->SetTimeLimit(OS_DEFAULT_PROCESS_QUEUE_TIME_LIMIT);

    bool found_flag = false;
    while (!found_flag) {

        switch (OS_SUBSTITUTE_STRATEGY) {
          case 1:
            candidate_raddress = RandomSelectionStrategy();
            break;
          case 2:
            candidate_raddress = ClockSelectionStrategy();
            break;
        }


        for (int j =0; j < static_cast<int>(translation_tables.size()); j++) { // цикл по всем ТП
            for (int k = 0; k < static_cast<int>(translation_tables[j].GetSize()); k++) { // цикл по записям j-й ТП
                if (translation_tables[j].GetRecord(k).raddress == candidate_raddress && translation_tables[j].GetRecord(k).is_valid == true) {
                    translation_tables[j].GetRecord(k).is_valid = false;
                    candidate_process = translation_tables[j].GetProcess();
                    candidate_vaddress = translation_tables[j].GetRecord(k).vaddress;

                    found_flag = true;
                    break;
                }
            }
            if (found_flag) {
                break;
            }
        }
    }
    if (CONFIG_LOG_DETAIL_LEVEL >= 2) {
        Log("  Deallocate RA=" + string(4 - to_string(candidate_raddress).length(), '0') + to_string(candidate_raddress) + " " + candidate_process->GetName());
    }
    // В очередь запросов добавляем новый запрос на загрузку данных в АС из виртуального
    // адреса кандидата
    requester.AddRequest(candidate_process, candidate_vaddress, candidate_raddress, true, p_process);

    TTStruct & tt = FindTT(p_process).GetRecord(vaddress);
    // Вносим изменение в ТП процесса о новом соответствии виртуального адреса реальному
    // А также о том, что реальный адрес действителеy
    tt.raddress = candidate_raddress;
    tt.is_valid = true;

    Process* scheduler_process = g_pOS->GetScheduler().GetProcess();
    g_pSim->GetTime() = g_pSim->GetTime() + OS_DEFAULT_TIME_FOR_ALLOCATION;
    if (scheduler_process->GetTimeLimit() > 0) {
        if (scheduler_process->getWaitingState() == false) {
          Schedule(GetTime(), scheduler_process, &Process::Work);
        }
    } else {
        ChangeQueue();
    }
}

TT& OS::FindTT(Process* p_process) {
    for (int i = 0; i < static_cast<int>(translation_tables.size()); i++) {
        if (translation_tables[i].GetProcess() == p_process) {
            return translation_tables[i];
        }
    }

    __throw_logic_error("TT NOT FOUND");
}

vector <TT> OS::GetTTs() {
    return translation_tables;
}

Requester& OS::GetRequester() {
    return requester;
}

Scheduler& OS::GetScheduler() {
    return scheduler;
}

RAM& OS::GetRAM() {
    return ram;
}

Clock* OS::GetClock() {
    return clock;
};

float OS::ComputeRML() {
    float result = 0.0;
    for (int i = 0; i < ram.GetSize(); i++) {
        if (ram.GetRealAddress(static_cast<PageNumber>(i)) == true) {
            result++;
        }
    }
    return result/ram.GetSize() * 100.;
}

void OS::Work() {
    //
}

void OS::Wait() {
    //
}

void OS::Start() {
    //
}



CPU::CPU() {
    SetName("CPU");
}

void CPU::Convert(VirtualAddress vaddress, Process *p_process) {
    TTStruct &tmp = g_pOS->FindTT(p_process).GetRecord(vaddress);
    Process *scheduler_process = g_pOS->GetScheduler().GetProcess();

    if (OS_SUBSTITUTE_STRATEGY == 2) tmp.bitR = true;

    if (tmp.is_valid == false) {
        // Прерывание по отсутствию страницы
        if (CONFIG_LOG_DETAIL_LEVEL >= 2) {
            Log("Translate VA=" + string(4 - to_string(vaddress).length(), '0') + to_string(vaddress) + " " + p_process->GetName() + " -> Interrupt");
        }

        g_pOS->HandelInterruption(vaddress, tmp.raddress, p_process);
        return;
    } else {
        // Успешное преобразование
        if (CONFIG_LOG_DETAIL_LEVEL >= 3) {
            Log("Translate VA=" + string(4 - to_string(vaddress).length(), '0') + to_string(vaddress) + " " + p_process->GetName() + " -> RA=" + string(4 - to_string(tmp.raddress).length(), '0') + to_string(tmp.raddress) + " TL=" + to_string(p_process->GetTimeLimit()));
        };

        if (scheduler_process->GetTimeLimit() > 0) {
          if (scheduler_process->getWaitingState() == false) {
            scheduler_process->SetTimeLimit(scheduler_process->GetTimeLimit() - CPU_DEFAULT_TIME_FOR_CONVERSION);
            Schedule(GetTime() + CPU_DEFAULT_TIME_FOR_CONVERSION, scheduler_process, &Process::Work);
          }
        } else {
            if (DEBUG_MODE) cout << "from Convert ";
            g_pSim->GetTime() = g_pSim->GetTime() + CPU_DEFAULT_TIME_FOR_CONVERSION;
            g_pOS->ChangeQueue();
        }
    }
}

void CPU::Work() {
    //
}

void CPU::Wait() {
    //
}

void CPU::Start() {
    //
}



DiskSpace::DiskSpace() {
    for (int i = 0; i < static_cast<int>(AE_DEFAULT_DISKSPACE_SIZE); i++) {
        disk.push_back(false);
    }
}

bool DiskSpace::GetDiskAddress(PageNumber daddress) {
    return disk[daddress];
}

void DiskSpace::SetDiskAddress(PageNumber daddress, bool value) {
    disk[daddress] = value;
}

int DiskSpace::GetSize() {
    return disk.size();
}

AE::AE() {
    last_time = 0;
    io_total_time = 0;
    io_count = 0;
    SetName("AE");
}

void AE::LoadData() {
    RequestStruct tmp = g_pOS->GetRequester().GetRequest();
    for (int i = 0; i < disk.GetSize(); i++) {
        if (disk.GetDiskAddress(i) == false) {
            disk.SetDiskAddress(i, true);
            SwapIndexStruct tmp_struct = {tmp.p_process, tmp.vaddress, static_cast<DiskAddress>(i)};
            SwapIndex.push_back(tmp_struct);
            io_total_time += AE_DEFAULT_TIME_FOR_DATA_IO;
            io_count += 1;

            if (CONFIG_LOG_DETAIL_LEVEL >= 1) {
                Log("    Save RA=" + string(4 - to_string(tmp.raddress).length(), '0') + to_string(tmp.raddress) + " (" + tmp.p_process->GetName() +" VA=" + string(4 - to_string(tmp.vaddress).length(), '0') + to_string(tmp.vaddress) + ") -> AA=" + string(4 - to_string(i).length(), '0') + to_string(i));
            }

            g_pOS->GetRequester().DeleteRequest(tmp.p_process, tmp.vaddress);

            // Вывести процесс из состояния ожидания и вернуть его в очередь
            if (tmp.p_initialProcess) {
              tmp.p_initialProcess->setWorking();
              g_pOS->GetScheduler().AddProcess(tmp.p_initialProcess);
            }

            last_time = g_pSim->GetTime();
            Schedule(GetTime() + AE_DEFAULT_TIME_FOR_DATA_IO, this, &AE::ProcessRequest);

            return;
        }
    }

    __throw_logic_error("NO FREE DISK SPACE");
}

void AE::PopData() {
    RequestStruct tmp = g_pOS->GetRequester().GetRequest();
    for (int i = 0; i < static_cast<int>(SwapIndex.size()); i++) {
        if (SwapIndex[i].p_process == tmp.p_process && SwapIndex[i].vaddress == tmp.vaddress) {
            SwapIndex.erase(SwapIndex.begin() + i);
            disk.SetDiskAddress(SwapIndex[i].daddress, false);
            io_total_time += AE_DEFAULT_TIME_FOR_DATA_IO;
            io_count += 1;
            if (CONFIG_LOG_DETAIL_LEVEL >= 1) {
                Log("    Pop AA=" + string(4 - to_string(i).length(), '0') + to_string(i) + " (" + tmp.p_process->GetName() + " VA=" + string(4 - to_string(tmp.vaddress).length(), '0') + to_string(tmp.vaddress) + ")");
            }

            g_pOS->GetRequester().DeleteRequest(tmp.p_process, tmp.vaddress);

            last_time = g_pSim->GetTime();
            Schedule(GetTime() + AE_DEFAULT_TIME_FOR_DATA_IO, this, &AE::ProcessRequest);

            return;
        }
    }

    __throw_logic_error("SWAP INDEX RECORD NOT FOUND");
}

void AE::ProcessRequest() {
    if (!g_pOS->GetRequester().IsEmpty()) {
        RequestStruct tmp = g_pOS->GetRequester().GetRequest();

        if (CONFIG_LOG_DETAIL_LEVEL >= 2) {
            string tmp2 =
            " || RML=" + to_string(g_pOS->ComputeRML()).substr(0,5) +
            " AEL=" + to_string(g_pAE->ComputeAEL()).substr(0,5) +
            " PSL="  + to_string(g_pAE->ComputePSL()).substr(0,5);

            string text = "      Process request (" + tmp.p_process->GetName() + " VA=" + string(4 - to_string(tmp.vaddress).length(), '0') + to_string(tmp.vaddress) + " RA=" + string(4 - to_string(tmp.raddress).length(), '0') + to_string(tmp.raddress) + " LF=" + to_string(tmp.load_flag) + " IP=" + tmp.p_initialProcess->GetName() + ")" + " RQL=" + to_string(g_pOS->GetRequester().GetQueueSize());
            PrintTime(&std::cout);
            string tail = string(msg_space-text.length(), ' ') + tmp2;
            std::cout << " " << std::setw(10) << std::setfill(' ') << std::right << "Requester"
                << "   " << text << tail << std::endl;
        }

        if (tmp.load_flag) {
            LoadData();
        } else {
            PopData();
        }

        g_pSim->GetTime() = g_pSim->GetTime() + AE_DEFAULT_TIME_FOR_DATA_IO;
    }


    return;
}

bool AE::IsLoaded(Process* p_process, VirtualAddress vaddress) {
    for (int i = 0; i < static_cast<int>(SwapIndex.size()); i++) {
        if (SwapIndex[i].p_process == p_process && SwapIndex[i].vaddress == vaddress) {
            return true;
        }
    }

    return false;
}

SimulatorTime AE::GetIOTT() {
    return io_total_time;
}

float AE::ComputeAEL() {
    // float result =0.0;
    // for (int i = 0; i < disk.GetSize(); i++) {
    //     if (disk.GetDiskAddress(static_cast<PageNumber>(i)) == true) {
    //         result++;
    //     }
    // }

    return (float)SwapIndex.size()/(float)AE_DEFAULT_DISKSPACE_SIZE * 100.;
}

double AE::ComputePSL() {
    float res = (float)io_total_time / (float)g_pSim->GetTime() * 100.;
    fileout << g_pSim->GetTime() << " " << res << endl;
    return res;
}


void AE::Work() {
    //
}

void AE::Wait() {
    //
}

void AE::Start() {
    //
}



Process::Process() {
    requested_memory = PROCESS_DEFAULT_REQUESTED_MEMORY;
    time_limit = 0;
    isWaiting = false;
}

void Process::Work() {
    if (randomizer(100) + 1 >= PROCESS_MEMORY_ACCESS_PERCENTAGE) {
        g_pSim->GetTime() = g_pSim->GetTime() + PROCESS_DEFAULT_WORK_TIME;
        time_limit -= PROCESS_DEFAULT_WORK_TIME;

        if (CONFIG_LOG_DETAIL_LEVEL >= 3) {
            Log("Working. No need of CPU.");
        }
        if (time_limit > 0) {
            Schedule(GetTime(), this, &Process::Work);
        } else {
            if (DEBUG_MODE) cout << "from Work ";
            g_pOS->ChangeQueue();
        }
        
    } else {
        VirtualAddress vaddress = static_cast<VirtualAddress>(randomizer(requested_memory));
        Schedule(GetTime(), g_pCPU, &CPU::Convert, vaddress, this);
    }
}

void Process::Wait() {
    //
}

void Process::Start() {
    Schedule(GetTime(), g_pOS, &OS::LoadProcess, this);
    if (CONFIG_LOG_DETAIL_LEVEL >= 3) {
        Log("Start!");
    }
}

void Process::SetRequestedMemory(uint64_t value) {
    requested_memory = value;
}

uint64_t Process::GetRequestedMemory() {
    return requested_memory;
}

void Process::SetTimeLimit(SimulatorTime value) {
    time_limit = value;
}

SimulatorTime Process::GetTimeLimit() {
    return time_limit;
}



Clock::Clock() {
    tindex = 0;
    lindex = 0;
}

PageNumber Clock::Tick() {
    vector <TT> tables = g_pOS->GetTTs();
    uint32_t tables_size = tables.size();

    if (tindex == tables_size) {
        tindex = 0;
    }

    if (lindex == tables[tindex].GetSize()) {
        lindex = 0;
    }

    uint32_t tindex_old = tindex;
    uint32_t lindex_old = lindex;
    if (tables[tindex].GetRecordByIndex(lindex).bitR == true) {
        do {
            if (tindex == tindex_old && lindex == lindex_old) {
                break;
            }

            tables[tindex].GetRecordByIndex(lindex).bitR = false;

            lindex++;
            if (lindex == tables[tindex].GetSize()) {
                tindex++;
                lindex = 0;
            }
        } while (tables[tindex].GetRecordByIndex(lindex).bitR == true);
    }

    RealAddress result = tables[tindex].GetRecordByIndex(lindex).raddress;
    return result;
}



int randomizer(int max) {
    // Функция, которая возвращает случайное число, работает на алгоритме Вихре Мерсенна
    std::random_device rd;
    std::mt19937 mersenne(rd());
    return mersenne() % max;
}
