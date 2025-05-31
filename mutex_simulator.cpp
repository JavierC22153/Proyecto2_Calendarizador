#include "scheduler.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>


void simularMutex(std::vector<Proceso>& procesos, std::map<std::string, Recurso>& recursos, 
                  std::vector<Accion>& acciones, UpdateCallback updateGUI) {
    int ciclo = 0;
    int max_ciclo = 0;
    
    // Encontrar el ciclo máximo
    for (const auto& a : acciones) {
        if (a.ciclo > max_ciclo) max_ciclo = a.ciclo;
    }
    max_ciclo += 5; // Margen adicional
    
    // Resetear estado de recursos
    for (auto& [nombre, recurso] : recursos) {
        recurso.ocupado = false;
        recurso.proceso_actual = "";
    }
    
    // Simulación ciclo por ciclo
    for (; ciclo <= max_ciclo; ciclo++) {
        bool accion_realizada = false;
        
        // Procesar acciones del ciclo actual
        for (const auto& a : acciones) {
            if (a.ciclo == ciclo) {
                auto& recurso = recursos[a.recurso];
                std::string bloque;
                
                if (!recurso.ocupado) {
                    // Recurso disponible - ACCESSED
                    recurso.ocupado = true;
                    recurso.proceso_actual = a.pid;
                    bloque = "[" + a.pid + "-" + a.tipo + "-" + a.recurso + "-ACCESSED]";
                } else {
                    // Recurso ocupado - WAITING
                    bloque = "[" + a.pid + "-" + a.tipo + "-" + a.recurso + "-WAITING]";
                }
                
                if (updateGUI) {
                    updateGUI(bloque, ciclo);
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                }
                
                accion_realizada = true;
            }
        }
        
        // Liberar recursos al final del ciclo (mutex dura 1 ciclo)
        for (auto& [nombre, recurso] : recursos) {
            if (recurso.ocupado && recurso.proceso_actual != "") {
                recurso.ocupado = false;
                recurso.proceso_actual = "";
            }
        }
        
        if (!accion_realizada) {
            if (updateGUI) {
                updateGUI("CPU IDLE", ciclo);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        }
    }
}
