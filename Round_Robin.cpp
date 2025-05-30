#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <map>
#include <thread>
#include <chrono>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

std::vector<std::string> colores = {RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};

struct Proceso {
    std::string pid;
    int burst_time;
    int arrival_time;
    int priority;
    int remaining_time = 0;
    int waiting_time = 0;
    int turnaround_time = 0;
    int start_time = -1;
    bool en_cola = false;
    bool terminado = false;
};

std::vector<Proceso> leerProcesosDesdeArchivo(const std::string &filename) {
    std::vector<Proceso> procesos;
    std::ifstream archivo(filename);
    std::string linea;

    while (std::getline(archivo, linea)) {
        std::stringstream ss(linea);
        std::string pid, bt_str, at_str, prio_str;
        if (std::getline(ss, pid, ',') && std::getline(ss, bt_str, ',') &&
            std::getline(ss, at_str, ',') && std::getline(ss, prio_str, ',')) {
            Proceso p;
            p.pid = pid;
            p.burst_time = std::stoi(bt_str);
            p.arrival_time = std::stoi(at_str);
            p.priority = std::stoi(prio_str);
            p.remaining_time = p.burst_time;
            procesos.push_back(p);
        }
    }

    return procesos;
}

void round_robin(std::vector<Proceso> &procesos, int quantum) {
    std::map<std::string, std::string> mapa_colores;
    for (size_t i = 0; i < procesos.size(); ++i) {
        mapa_colores[procesos[i].pid] = colores[i % colores.size()];
    }

    int tiempo = 0, completados = 0, n = procesos.size();
    std::vector<std::string> timeline;
    std::queue<int> cola;

    std::cout << "\nSimulación Round Robin - Diagrama de Gantt:\n";

    while (completados < n) {
        // Agregar procesos que llegan en este ciclo
        for (int i = 0; i < n; ++i) {
            if (!procesos[i].en_cola && !procesos[i].terminado && procesos[i].arrival_time <= tiempo) {
                cola.push(i);
                procesos[i].en_cola = true;
            }
        }

        if (!cola.empty()) {
            int idx = cola.front();
            cola.pop();
            auto &p = procesos[idx];

            if (p.start_time == -1) p.start_time = tiempo;

            int ejecucion = std::min(quantum, p.remaining_time);
            for (int i = 0; i < ejecucion; ++i) {
                timeline.push_back(p.pid);
                std::cout << "Ciclo " << tiempo << ": "
                          << mapa_colores[p.pid] << "[" << p.pid << "]" << RESET << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                tiempo++;

                // Revisar si llegaron nuevos procesos
                for (int j = 0; j < n; ++j) {
                    if (!procesos[j].en_cola && !procesos[j].terminado && procesos[j].arrival_time <= tiempo) {
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
            } else {
                cola.push(idx); // Volver a la cola
            }

        } else {
            std::cout << "Ciclo " << tiempo << ": CPU IDLE\n";
            timeline.push_back("-");
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            tiempo++;
        }

        // Revisar si todos terminaron
        completados = 0;
        for (auto &p : procesos) {
            if (p.terminado) completados++;
        }
    }

    // Resumen de métricas
    double total_wait = 0, total_tat = 0;
    std::cout << "\nResumen de Metricas:\n";
    std::cout << "PID\tWT\tTAT\n";
    for (auto &p : procesos) {
        std::cout << p.pid << "\t" << p.waiting_time << "\t" << p.turnaround_time << "\n";
        total_wait += p.waiting_time;
        total_tat += p.turnaround_time;
    }
    std::cout << "\nAverage Waiting Time: " << total_wait / procesos.size() << "\n";
    std::cout << "Average Turnaround Time: " << total_tat / procesos.size() << "\n";

    // Mostrar línea de tiempo final
    std::cout << "\nDiagrama de Gantt Final:\n";
    for (const auto &p : timeline) {
        if (p == "-") {
            std::cout << "[IDLE]";
        } else {
            std::cout << mapa_colores[p] << "[" << p << "]" << RESET;
        }
    }
    std::cout << "\n";
}

int main() {
    std::string archivo = "procesos.txt";
    auto procesos = leerProcesosDesdeArchivo(archivo);

    if (procesos.empty()) {
        std::cerr << "No se pudieron cargar procesos desde el archivo.\n";
        return 1;
    }

    int quantum;
    std::cout << "Ingrese el quantum para Round Robin: ";
    std::cin >> quantum;

    round_robin(procesos, quantum);
    return 0;
}
