#include "scheduler.h"
#include <fstream>
#include <sstream>

std::vector<Proceso> leerProcesosDesdeArchivo(const std::string& filename) {
    std::vector<Proceso> procesos;
    std::ifstream archivo(filename);
    std::string linea;

    while (std::getline(archivo, linea)) {
        std::stringstream ss(linea);
        std::string pid, bt_str, at_str, prio_str;
        if (std::getline(ss, pid, ',') && std::getline(ss, bt_str, ',') &&
            std::getline(ss, at_str, ',') && std::getline(ss, prio_str, ',')) {
            
            // Eliminar espacios en blanco
            pid.erase(0, pid.find_first_not_of(" \t"));
            pid.erase(pid.find_last_not_of(" \t") + 1);
            
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
