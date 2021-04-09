#include <iostream>
#include <stdint.h>
#include <assert.h>
#include <fstream>
#include <chrono>
#include "vm.hpp"


using namespace std;

Sim *g_pSim;
OS *g_pOS;
CPU *g_pCPU;
AE *g_pAE;
Clock *g_pClock;

extern SimulatorTime CONFIG_SIM_TIME_LIMIT;
extern int PROCESS_AMOUNT;
extern int OS_SUBSTITUTE_STRATEGY;

ofstream fileout;

int main()
{
    fileout.open("psl.data");
    Process * all_processes[PROCESS_AMOUNT];

    cout << endl << endl << endl << endl;
    cout << "          Welcome to the Virtual Storage Simulator" << endl << endl;

    int initialize_result = InitializeInputData();
    if (initialize_result != 0) {
        return initialize_result;
    };

    cout << endl << endl << "Simulation will go on until the time limit expires." << endl << "To interrupt simulation process immediately press Ctrl+C. " << endl;
    
    #ifdef _WIN32
        cout << "Press any key to start simulation" << endl;
        system("pause >nul");
    #else
        system("read -n 1 -s -r -p 'Press any key to start simulation'");
    #endif
    
    
    cout << "Starting simulation. It could take few minutes until first messages" << endl << "appear in case you set CONFIG_LOG_DETAIL_LEVEL = 1." << endl;

    try {
        g_pSim = new Sim;
        g_pOS = new OS;
        g_pCPU = new CPU;
        g_pAE = new AE;

        for (int i = 0; i < PROCESS_AMOUNT; i++) {
            all_processes[i] = new Process;
            string name = "Process" + string(2-to_string(i).length(), '0') + to_string(i);
            all_processes[i]->SetName(name);
            Schedule(g_pSim->GetTime(), all_processes[i], &Process::Start);
        };

        g_pSim->SetLimit(CONFIG_SIM_TIME_LIMIT);
        
        auto startt = chrono::high_resolution_clock::now();
        chrono::duration<float> total_duration = startt - startt;

        while(!g_pSim->Run())
        {
            PrintTime(&cout);
            auto endt = chrono::high_resolution_clock::now();
            chrono::duration<float> duration = endt - startt;
            cout << "Duration " << duration.count() << "s" << endl;
            total_duration += duration;
            
            PrintTime(&cout);
            cout << "Do you want to proceed? [Y/n]: ";

            char c[3];
            cin.getline(c,sizeof(c));

            if(c[0] == 'n')
            {
                break;
            };

            cout << endl;

            g_pSim->SetLimit(g_pSim->GetTime()+CONFIG_SIM_TIME_LIMIT);
        }

        cout << "Total duration " << total_duration.count() << "s" << endl;
        
        for (int i = 0; i < PROCESS_AMOUNT; i++) {
            delete all_processes[i];
        }

        delete g_pSim;
        delete g_pOS;
        delete g_pCPU;
        delete g_pAE;
    }
    catch(exception& ex) {
        cout << "CAUGHT AN EXCEPTION: " << ex.what() << endl;
    }
    catch(Requester& err) {
        cout << "CAUGHT AN EXCEPTION: NO INFO IN TTS FOUND FOR CANDIDATE" << endl;
        err.PrintQueue();
    }
    fileout.close();
    return 0;
}
