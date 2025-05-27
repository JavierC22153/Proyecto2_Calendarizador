#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include <thread>
#include <chrono>

// 🎨 Colores ANSI
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
    int waiting_time = 0;
    int turnaround_time = 0;
    int remaining_time = 0;
    bool terminado = false;
};

// Leer procesos desde archivo
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

void sjf(std::vector<Proceso> &procesos) {
    std::map<std::string, std::string> mapa_colores;
    for (size_t i = 0; i < procesos.size(); ++i) {
        mapa_colores[procesos[i].pid] = colores[i % colores.size()];
    }

    int tiempo = 0, completados = 0, n = procesos.size();
    std::vector<std::string> timeline;

    std::cout << "\nSimulación SJF - Diagrama de Gantt:\n";
    while (completados < n) {
        int idx = -1, min_bt = 1e9;

        for (int i = 0; i < n; ++i) {
            if (!procesos[i].terminado && procesos[i].arrival_time <= tiempo &&
                procesos[i].burst_time < min_bt) {
                min_bt = procesos[i].burst_time;
                idx = i;
            }
        }

        if (idx != -1) {
            auto &p = procesos[idx];
            p.waiting_time = tiempo - p.arrival_time;
            for (int i = 0; i < p.burst_time; ++i) {
                timeline.push_back(p.pid);
                std::cout << "Ciclo " << tiempo << ": "
                          << mapa_colores[p.pid] << "[" << p.pid << "]" << RESET << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                tiempo++;
            }
            p.turnaround_time = p.waiting_time + p.burst_time;
            p.terminado = true;
            completados++;
        } else {
            std::cout << "Ciclo " << tiempo << ": CPU IDLE\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            tiempo++;
        }
    }

    // Resumen de métricas
    double total_wait = 0, total_tat = 0;
    std::cout << "\nResumen de Métricas:\n";
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
        std::cout << mapa_colores[p] << "[" << p << "]" << RESET;
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

    sjf(procesos);
    return 0;
}
