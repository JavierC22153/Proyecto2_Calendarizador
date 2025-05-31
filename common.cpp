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

std::map<std::string, Recurso> leerRecursosDesdeArchivo(const std::string& filename) {
    std::map<std::string, Recurso> recursos;
    std::ifstream file(filename);
    std::string linea;
    
    while (std::getline(file, linea)) {
        std::stringstream ss(linea);
        std::string nombre, contador;
        if (std::getline(ss, nombre, ',') && std::getline(ss, contador, ',')) {
            // Eliminar espacios en blanco
            nombre.erase(0, nombre.find_first_not_of(" \t"));
            nombre.erase(nombre.find_last_not_of(" \t") + 1);
            
            Recurso r;
            r.nombre = nombre;
            r.contador = std::stoi(contador);
            r.contador_inicial = r.contador;
            recursos[nombre] = r;
        }
    }
    return recursos;
}

std::vector<Accion> leerAccionesDesdeArchivo(const std::string& filename) {
    std::vector<Accion> acciones;
    std::ifstream file(filename);
    std::string linea;
    
    while (std::getline(file, linea)) {
        std::stringstream ss(linea);
        std::string pid, tipo, recurso, ciclo;
        if (std::getline(ss, pid, ',') && std::getline(ss, tipo, ',') &&
            std::getline(ss, recurso, ',') && std::getline(ss, ciclo, ',')) {
            // Eliminar espacios en blanco
            pid.erase(0, pid.find_first_not_of(" \t"));
            pid.erase(pid.find_last_not_of(" \t") + 1);
            tipo.erase(0, tipo.find_first_not_of(" \t"));
            tipo.erase(tipo.find_last_not_of(" \t") + 1);
            recurso.erase(0, recurso.find_first_not_of(" \t"));
            recurso.erase(recurso.find_last_not_of(" \t") + 1);
            
            acciones.push_back({pid, tipo, recurso, std::stoi(ciclo)});
        }
    }
    return acciones;
}
