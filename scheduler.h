#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <string>
#include <functional>

struct Proceso {
    std::string pid;
    int burst_time;
    int arrival_time;
    int priority;
    int waiting_time = 0;
    int turnaround_time = 0;
    int remaining_time = 0;
    int start_time = -1;
    bool terminado = false;
    bool en_cola = false;
};

using UpdateCallback = std::function<void(const std::string& pid, int cycle)>;

struct SimulationResult {
    std::vector<std::pair<std::string, int>> timeline;
    double avgWaitingTime;
    double avgTurnaroundTime;
};

std::vector<Proceso> leerProcesosDesdeArchivo(const std::string& filename);
SimulationResult fifo(std::vector<Proceso>& procesos, UpdateCallback updateGUI);
SimulationResult sjf(std::vector<Proceso>& procesos, UpdateCallback updateGUI);
SimulationResult srt(std::vector<Proceso>& procesos, UpdateCallback updateGUI);
SimulationResult roundRobin(std::vector<Proceso>& procesos, int quantum, UpdateCallback updateGUI);
SimulationResult priority(std::vector<Proceso>& procesos, UpdateCallback updateGUI);

#endif 
