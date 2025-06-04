#include "scheduler.h"
#include <climits>
#include <thread>
#include <chrono>

SimulationResult priority(std::vector<Proceso>& procesos, UpdateCallback updateGUI) {
    int tiempo = 0, completados = 0, n = procesos.size();
    SimulationResult result;

    while (completados < n) {
        int idx = -1, min_prio = INT_MAX;

        // Buscar el proceso de mayor prioridad sin importar el arrival_time
        for (int i = 0; i < n; ++i) {
            if (!procesos[i].terminado && procesos[i].priority < min_prio) {
                min_prio = procesos[i].priority;
                idx = i;
            }
        }

        if (idx != -1) {
            auto& p = procesos[idx];
            p.waiting_time = tiempo; 
			// No entendi muy bien lo que pusieron en discord, pero si se toma en cuenta el arrival time
			// para el tiempo entonces:
			// p.waiting_time = tiempo - p.arrival_time;

            for (int i = 0; i < p.burst_time; ++i) {
                result.timeline.push_back({p.pid, tiempo});
                if (updateGUI) {
                    updateGUI(p.pid, tiempo);
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                }
                tiempo++;
            }

            p.turnaround_time = p.waiting_time + p.burst_time;
            p.terminado = true;
            completados++;
        }
    }

    double total_wait = 0, total_tat = 0;
    for (const auto& p : procesos) {
        total_wait += p.waiting_time;
        total_tat += p.turnaround_time;
    }

    result.avgWaitingTime = total_wait / procesos.size();
    result.avgTurnaroundTime = total_tat / procesos.size();

    return result;
}

