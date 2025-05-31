#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <vector>
#include <string>
#include <functional>
#include <map>

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

struct Recurso {
    std::string nombre;
    int contador;
    int contador_inicial;
    bool ocupado = false;
    std::string proceso_actual = "";
    std::map<std::string, int> procesos_uso;
};

struct Accion {
    std::string pid;
    std::string tipo; 
    std::string recurso;
    int ciclo;
};

using UpdateCallback = std::function<void(const std::string& pid, int cycle)>;

struct SimulationResult {
    std::vector<std::pair<std::string, int>> timeline;
    double avgWaitingTime;
    double avgTurnaroundTime;
};

std::vector<Proceso> leerProcesosDesdeArchivo(const std::string& filename);
std::map<std::string, Recurso> leerRecursosDesdeArchivo(const std::string& filename);
std::vector<Accion> leerAccionesDesdeArchivo(const std::string& filename);

// Funciones de algoritmos de calendarización
SimulationResult fifo(std::vector<Proceso>& procesos, UpdateCallback updateGUI);
SimulationResult sjf(std::vector<Proceso>& procesos, UpdateCallback updateGUI);
SimulationResult srt(std::vector<Proceso>& procesos, UpdateCallback updateGUI);
SimulationResult roundRobin(std::vector<Proceso>& procesos, int quantum, UpdateCallback updateGUI);
SimulationResult priority(std::vector<Proceso>& procesos, UpdateCallback updateGUI);

// Funciones de sincronización
void simularMutex(std::vector<Proceso>& procesos, std::map<std::string, Recurso>& recursos, 
                  std::vector<Accion>& acciones, UpdateCallback updateGUI);
void simularSemaforo(std::vector<Proceso>& procesos, std::map<std::string, Recurso>& recursos, 
                     std::vector<Accion>& acciones, UpdateCallback updateGUI);

#endif
