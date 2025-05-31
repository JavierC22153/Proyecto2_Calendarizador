#include "scheduler.h"
#include <thread>
#include <chrono>
#include <vector>

void simularSemaforo(std::vector<Proceso>& procesos, std::map<std::string, Recurso>& recursos, 
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
        recurso.contador = recurso.contador_inicial;
        recurso.procesos_uso.clear();
    }
    
    // Simulación ciclo por ciclo
    for (; ciclo <= max_ciclo; ciclo++) {
        bool accion_realizada = false;
        
        // Procesar nuevas acciones del ciclo actual
        for (const auto& a : acciones) {
            if (a.ciclo == ciclo) {
                auto& recurso = recursos[a.recurso];
                std::string bloque;
                
                if (recurso.contador > 0) {
                    // Hay recursos disponibles - ACCESSED
                    recurso.contador--;
                    recurso.procesos_uso[a.pid] = 1; 
                    bloque = "[" + a.pid + "-" + a.tipo + "-" + a.recurso + "-ACCESSED]";
                } else {
                    // No hay recursos disponibles - WAITING
                    bloque = "[" + a.pid + "-" + a.tipo + "-" + a.recurso + "-WAITING]";
                }
                
                if (updateGUI) {
                    updateGUI(bloque, ciclo);
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                }
                
                accion_realizada = true;
            }
        }
        
        // Decrementar el tiempo de uso y liberar recursos que ya terminaron
        for (auto& [nombre, recurso] : recursos) {
            std::vector<std::string> procesos_terminados;
            
            for (auto& [pid, tiempo_restante] : recurso.procesos_uso) {
                tiempo_restante--;
                if (tiempo_restante <= 0) {
                    recurso.contador++; // Liberar el recurso
                    procesos_terminados.push_back(pid);
                }
            }
            
            // Eliminar procesos que ya terminaron de usar el recurso
            for (const auto& pid : procesos_terminados) {
                recurso.procesos_uso.erase(pid);
            }
        }
        
        // Si no hubo acción, CPU IDLE
        if (!accion_realizada) {
            if (updateGUI) {
                updateGUI("CPU IDLE", ciclo);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        }
    }
}
