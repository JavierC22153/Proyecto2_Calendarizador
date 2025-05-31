// FIFO.cpp
#include "scheduler.h"
#include <algorithm>
#include <thread>
#include <chrono>

SimulationResult fifo(std::vector<Proceso>& procesos, UpdateCallback updateGUI) {
    // Ordenar por tiempo de llegada
    std::sort(procesos.begin(), procesos.end(), [](const Proceso& a, const Proceso& b) {
        return a.arrival_time < b.arrival_time;
    });

    int tiempo = 0;
    SimulationResult result;

    for (auto& p : procesos) {
        // Si hay tiempo de espera antes de que llegue el proceso
        if (tiempo < p.arrival_time) {
            // Agregar ciclos IDLE
            while (tiempo < p.arrival_time) {
                result.timeline.push_back({"IDLE", tiempo});
                if (updateGUI) {
                    updateGUI("IDLE", tiempo);
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                }
                tiempo++;
            }
        }
        
        p.waiting_time = tiempo - p.arrival_time;

        // Ejecutar el proceso
        for (int i = 0; i < p.burst_time; ++i) {
            result.timeline.push_back({p.pid, tiempo});
            if (updateGUI) {
                updateGUI(p.pid, tiempo);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
            tiempo++;
        }

        p.turnaround_time = p.waiting_time + p.burst_time;
    }

    // Calcular métricas
    double total_wait = 0, total_tat = 0;
    for (const auto& p : procesos) {
        total_wait += p.waiting_time;
        total_tat += p.turnaround_time;
    }

    result.avgWaitingTime = total_wait / procesos.size();
    result.avgTurnaroundTime = total_tat / procesos.size();

    return result;
}
