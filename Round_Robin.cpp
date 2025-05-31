#include "scheduler.h"
#include <queue>
#include <thread>
#include <chrono>
#include <algorithm>

SimulationResult roundRobin(std::vector<Proceso>& procesos, int quantum, UpdateCallback updateGUI) {
    int tiempo = 0, completados = 0, n = procesos.size();
    SimulationResult result;
    std::queue<int> cola;

    // Inicializar remaining_time
    for (auto& p : procesos) {
        p.remaining_time = p.burst_time;
    }

    while (completados < n) {
        for (int i = 0; i < n; ++i) {
            if (!procesos[i].en_cola && !procesos[i].terminado && 
                procesos[i].arrival_time <= tiempo) {
                cola.push(i);
                procesos[i].en_cola = true;
            }
        }

        if (!cola.empty()) {
            int idx = cola.front();
            cola.pop();
            auto& p = procesos[idx];

            if (p.start_time == -1) {
                p.start_time = tiempo;
            }

            int ejecucion = std::min(quantum, p.remaining_time);
            
            for (int i = 0; i < ejecucion; ++i) {
                result.timeline.push_back({p.pid, tiempo});
                if (updateGUI) {
                    updateGUI(p.pid, tiempo);
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                }
                tiempo++;

                // Revisar si llegaron nuevos procesos mientras se ejecutaba
                for (int j = 0; j < n; ++j) {
                    if (!procesos[j].en_cola && !procesos[j].terminado && 
                        procesos[j].arrival_time <= tiempo) {
                        cola.push(j);
                        procesos[j].en_cola = true;
                    }
                }
            }

            p.remaining_time -= ejecucion;

            if (p.remaining_time == 0) {
                p.terminado = true;
                p.turnaround_time = tiempo - p.arrival_time;
                p.waiting_time = p.turnaround_time - p.burst_time;
                completados++;
            } else {
                cola.push(idx); // Volver a la cola
            }

        } else {
            result.timeline.push_back({"IDLE", tiempo});
            if (updateGUI) {
                updateGUI("IDLE", tiempo);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
            tiempo++;
        }
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
