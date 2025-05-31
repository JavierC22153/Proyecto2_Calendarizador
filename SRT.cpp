#include "scheduler.h"
#include <climits>
#include <thread>
#include <chrono>

SimulationResult srt(std::vector<Proceso>& procesos, UpdateCallback updateGUI) {
    int tiempo = 0, completados = 0, n = procesos.size();
    SimulationResult result;

    // Inicializar remaining_time
    for (auto& p : procesos) {
        p.remaining_time = p.burst_time;
    }

    while (completados < n) {
        int idx = -1, min_rem = INT_MAX;

        // Buscar el proceso con menor tiempo restante
        for (int i = 0; i < n; ++i) {
            if (!procesos[i].terminado && procesos[i].arrival_time <= tiempo &&
                procesos[i].remaining_time < min_rem && procesos[i].remaining_time > 0) {
                min_rem = procesos[i].remaining_time;
                idx = i;
            }
        }

        if (idx != -1) {
            auto& p = procesos[idx];

            if (p.start_time == -1) {
                p.start_time = tiempo;
            }

            result.timeline.push_back({p.pid, tiempo});
            if (updateGUI) {
                updateGUI(p.pid, tiempo);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }

            p.remaining_time--;
            tiempo++;

            if (p.remaining_time == 0) {
                p.terminado = true;
                p.turnaround_time = tiempo - p.arrival_time;
                p.waiting_time = p.turnaround_time - p.burst_time;
                completados++;
            }
        } else {
            // CPU IDLE
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
