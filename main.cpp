#include <iostream>
#include <stdint.h>
#include <assert.h>
#include "vm.hpp"


using namespace std;

Sim *g_pSim;
OS *g_pOS;
CPU *g_pCPU;
AE *g_pAE;

extern const SimulatorTime CONFIG_SIM_TIME_LIMIT;
extern const int PROCESS_AMOUNT;

int main()
{
    cout << endl << endl << endl << endl;
    cout << "          Welcome to the Virtual Storage Simulator" << endl << endl;

    int initialize_result = InitializeInputData();
    if (initialize_result != 0) {
        return initialize_result;
    };

    cout << endl << endl << "Simulation will go on until the time limit expires." << endl << "To interrupt simulation process immediately press Ctrl+C. " << endl;
    cout << "Press any key to start simulation." << endl;
    system("pause >nul");
    cout << "Starting simulation. It could take few minutes until first messages" << endl << "appear in case you set CONFIG_LOG_DETAIL_LEVEL = 1." << endl;

    try {
        g_pSim = new Sim;
        g_pOS = new OS;
        g_pCPU = new CPU;
        g_pAE = new AE;

        Process * all_processes[PROCESS_AMOUNT];
        for (int i = 0; i < PROCESS_AMOUNT; i++) {
            all_processes[i] = new Process;
            string name = "Process" + string(2-to_string(i).length(), '0') + to_string(i);
            all_processes[i]->SetName(name);
            Schedule(g_pSim->GetTime(), all_processes[i], &Process::Start);
        };

        g_pSim->SetLimit(CONFIG_SIM_TIME_LIMIT);
        while(!g_pSim->Run())
        {
            PrintTime(&std::cout);
            std::cout << "Do you want to proceed? [Y/n]" << std::endl;

            char c[3];
            std::cin.getline(c,sizeof(c));

            if(c[0] == 'n')
            {
                break;
            };

            g_pSim->SetLimit(g_pSim->GetTime()+CONFIG_SIM_TIME_LIMIT);
        }

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
    return 0;
}
