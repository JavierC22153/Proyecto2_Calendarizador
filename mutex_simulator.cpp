#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <thread>
#include <chrono>

#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"

struct Proceso {
    std::string pid;
    int burst_time;
    int arrival_time;
    int priority;
};

struct Recurso {
    std::string nombre;
    int contador;
    bool ocupado = false;
    std::string proceso_actual = "";
};

struct Accion {
    std::string pid;
    std::string tipo; // READ o WRITE
    std::string recurso;
    int ciclo;
};

struct Evento {
    int ciclo;
    std::string contenido;
    std::string color;
};

// Leer Procesos
std::vector<Proceso> leerProcesos(const std::string &filename) {
    std::vector<Proceso> procesos;
    std::ifstream file(filename);
    std::string linea;
    while (std::getline(file, linea)) {
        std::stringstream ss(linea);
        std::string pid, bt, at, prio;
        if (std::getline(ss, pid, ',') && std::getline(ss, bt, ',') &&
            std::getline(ss, at, ',') && std::getline(ss, prio, ',')) {
            procesos.push_back({pid, std::stoi(bt), std::stoi(at), std::stoi(prio)});
        }
    }
    return procesos;
}

// Leer Recursos
std::map<std::string, Recurso> leerRecursos(const std::string &filename) {
    std::map<std::string, Recurso> recursos;
    std::ifstream file(filename);
    std::string linea;
    while (std::getline(file, linea)) {
        std::stringstream ss(linea);
        std::string nombre, contador;
        if (std::getline(ss, nombre, ',') && std::getline(ss, contador, ',')) {
            Recurso r;
            r.nombre = nombre;
            r.contador = std::stoi(contador);
            recursos[nombre] = r;
        }
    }
    return recursos;
}

// Leer Acciones
std::vector<Accion> leerAcciones(const std::string &filename) {
    std::vector<Accion> acciones;
    std::ifstream file(filename);
    std::string linea;
    while (std::getline(file, linea)) {
        std::stringstream ss(linea);
        std::string pid, tipo, recurso, ciclo;
        if (std::getline(ss, pid, ',') && std::getline(ss, tipo, ',') &&
            std::getline(ss, recurso, ',') && std::getline(ss, ciclo, ',')) {
            acciones.push_back({pid, tipo, recurso, std::stoi(ciclo)});
        }
    }
    return acciones;
}

// Simulación Mutex Locks
void simularMutex(std::vector<Proceso> &procesos, std::map<std::string, Recurso> &recursos, std::vector<Accion> &acciones) {
    int ciclo = 0;
    int max_ciclo = 0;
    for (auto &a : acciones) {
        if (a.ciclo > max_ciclo) max_ciclo = a.ciclo;
    }
    max_ciclo += 5;

    std::vector<Evento> linea_tiempo;

    std::cout << "\n--- Simulacion Mutex Locks ---\n";

    for (; ciclo <= max_ciclo; ciclo++) {
        std::cout << "\nCiclo " << ciclo << ":\n";
        bool accion_realizada = false;

        for (auto &a : acciones) {
            if (a.ciclo == ciclo) {
                auto &recurso = recursos[a.recurso];
                if (!recurso.ocupado) {
                    // ACCESSED
                    recurso.ocupado = true;
                    recurso.proceso_actual = a.pid;
                    std::string bloque = "[" + a.pid + "-" + a.tipo + "-" + a.recurso + "-ACCESSED]";
                    std::cout << GREEN << bloque << RESET << "\n";
                    linea_tiempo.push_back({ciclo, bloque, GREEN});
                } else {
                    // WAITING
                    std::string bloque = "[" + a.pid + "-" + a.tipo + "-" + a.recurso + "-WAITING]";
                    std::cout << YELLOW << bloque << RESET << "\n";
                    linea_tiempo.push_back({ciclo, bloque, YELLOW});
                }
                accion_realizada = true;
            }
        }

        // Liberar recursos al siguiente ciclo (1 ciclo por acceso)
        for (auto &r : recursos) {
            if (r.second.ocupado) {
                if (r.second.proceso_actual != "") {
                    r.second.ocupado = false;
                    r.second.proceso_actual = "";
                }
            }
        }

        if (!accion_realizada) {
            std::string bloque = "[CPU IDLE]";
            std::cout << bloque << "\n";
            linea_tiempo.push_back({ciclo, bloque, RESET});
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    // Mostrar Línea de Tiempo Final
    std::cout << "\n--- Linea de Tiempo Final ---\n";
    for (auto &evento : linea_tiempo) {
        std::cout << "Ciclo " << evento.ciclo << ": " << evento.color << evento.contenido << RESET << "\n";
    }

    std::cout << "\nSimulacion finalizada.\n";
}

int main() {
    auto procesos = leerProcesos("procesos.txt");
    auto recursos = leerRecursos("recursos.txt");
    auto acciones = leerAcciones("acciones.txt");

    if (procesos.empty() || recursos.empty() || acciones.empty()) {
        std::cerr << "Error: Verifica los archivos de entrada.\n";
        return 1;
    }

    simularMutex(procesos, recursos, acciones);
    return 0;
}
