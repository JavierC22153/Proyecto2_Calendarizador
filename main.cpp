// main.cpp - Interfaz gráfica completa para simulador de scheduling y sincronización
#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/splitter.h>
#include <wx/filedlg.h>
#include <wx/choice.h>
#include <wx/spinctrl.h>
#include <wx/scrolwin.h>
#include <wx/dcbuffer.h>
#include <wx/timer.h>
#include <wx/listctrl.h>
#include <wx/statline.h>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <random>
#include "scheduler.h"

// Panel de Diagrama de Gantt
class GanttPanel : public wxScrolledWindow {
private:
    std::vector<std::pair<std::string, int>> timeline;
    std::map<std::string, wxColour> processColors;
    int currentCycle = 0;
    int blockWidth = 40;
    int blockHeight = 50;
    bool isSync = false;
    
public:
    GanttPanel(wxWindow* parent) : wxScrolledWindow(parent) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetVirtualSize(800, 150);
        SetScrollRate(10, 0);
        Bind(wxEVT_PAINT, &GanttPanel::OnPaint, this);
    }
    
    void SetSyncMode(bool sync) { isSync = sync; }
    
    void AddTimeSlot(const std::string& pid, int cycle) {
        timeline.push_back({pid, cycle});
        currentCycle = cycle;
        
        int requiredWidth = (timeline.size() + 2) * blockWidth;
        if (requiredWidth > GetVirtualSize().GetWidth()) {
            SetVirtualSize(requiredWidth, 150);
        }
        
        Refresh();
        Scroll(timeline.size() * blockWidth / 10, 0);
    }
    
    void SetProcessColor(const std::string& pid, const wxColour& color) {
        processColors[pid] = color;
    }
    
    void Clear() {
        timeline.clear();
        processColors.clear();
        currentCycle = 0;
        SetVirtualSize(800, 150);
        Refresh();
    }
    
    void OnPaint(wxPaintEvent& event) {
        wxAutoBufferedPaintDC dc(this);
        DoPrepareDC(dc);
        
        dc.Clear();
        dc.SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        
        int x = 10;
        int y = 30;
        
        for (size_t i = 0; i < timeline.size(); i++) {
            const auto& slot = timeline[i];
            
            // Dibujar el bloque
            if (slot.first == "IDLE" || slot.first == "CPU IDLE") {
                dc.SetBrush(wxBrush(wxColour(200, 200, 200)));
            } else if (slot.first.find("WAITING") != std::string::npos) {
                dc.SetBrush(wxBrush(wxColour(255, 200, 100)));
            } else if (slot.first.find("ACCESSED") != std::string::npos) {
                dc.SetBrush(wxBrush(wxColour(100, 255, 100)));
            } else {
                dc.SetBrush(wxBrush(processColors[slot.first]));
            }
            
            dc.DrawRectangle(x, y, blockWidth, blockHeight);
            
            // Texto adaptativo para modo sincronización
            if (isSync && (slot.first.find("-") != std::string::npos)) {
                // Mostrar info compacta para sincronización
                dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
                size_t pos = slot.first.find("-");
                dc.DrawText(slot.first.substr(0, pos), x + 2, y + 5);
                dc.DrawText(slot.first.substr(pos + 1, 1), x + 2, y + 20);
            } else {
                dc.DrawText(slot.first, x + 5, y + 20);
            }
            
            // Número de ciclo
            dc.SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            dc.DrawText(wxString::Format("%d", slot.second), x + 10, y + blockHeight + 5);
            
            x += blockWidth;
        }
        
        dc.SetTextForeground(wxColour(255, 0, 0));
        dc.DrawText(wxString::Format("Ciclo actual: %d", currentCycle), 10, 5);
    }
};

// Panel de lista de información
class InfoListPanel : public wxPanel {
private:
    wxListCtrl* listCtrl;
    
public:
    InfoListPanel(wxWindow* parent) : wxPanel(parent) {
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        
        listCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                   wxLC_REPORT | wxLC_SINGLE_SEL);
        sizer->Add(listCtrl, 1, wxEXPAND | wxALL, 5);
        
        SetSizer(sizer);
    }
    
    void ShowProcesses(const std::vector<Proceso>& procesos) {
        listCtrl->ClearAll();
        listCtrl->AppendColumn("PID", wxLIST_FORMAT_LEFT, 100);
        listCtrl->AppendColumn("Burst Time", wxLIST_FORMAT_CENTER, 100);
        listCtrl->AppendColumn("Arrival Time", wxLIST_FORMAT_CENTER, 100);
        listCtrl->AppendColumn("Priority", wxLIST_FORMAT_CENTER, 100);
        
        for (size_t i = 0; i < procesos.size(); i++) {
            long index = listCtrl->InsertItem(i, procesos[i].pid);
            listCtrl->SetItem(index, 1, wxString::Format("%d", procesos[i].burst_time));
            listCtrl->SetItem(index, 2, wxString::Format("%d", procesos[i].arrival_time));
            listCtrl->SetItem(index, 3, wxString::Format("%d", procesos[i].priority));
        }
    }
    
    void ShowResources(const std::map<std::string, Recurso>& recursos) {
        listCtrl->ClearAll();
        listCtrl->AppendColumn("Nombre", wxLIST_FORMAT_LEFT, 150);
        listCtrl->AppendColumn("Contador", wxLIST_FORMAT_CENTER, 100);
        listCtrl->AppendColumn("Estado", wxLIST_FORMAT_CENTER, 100);
        
        int i = 0;
        for (const auto& [nombre, recurso] : recursos) {
            long index = listCtrl->InsertItem(i++, nombre);
            listCtrl->SetItem(index, 1, wxString::Format("%d", recurso.contador));
            listCtrl->SetItem(index, 2, recurso.ocupado ? "Ocupado" : "Disponible");
        }
    }
    
    void ShowActions(const std::vector<Accion>& acciones) {
        listCtrl->ClearAll();
        listCtrl->AppendColumn("PID", wxLIST_FORMAT_LEFT, 100);
        listCtrl->AppendColumn("Tipo", wxLIST_FORMAT_CENTER, 100);
        listCtrl->AppendColumn("Recurso", wxLIST_FORMAT_CENTER, 100);
        listCtrl->AppendColumn("Ciclo", wxLIST_FORMAT_CENTER, 100);
        
        for (size_t i = 0; i < acciones.size(); i++) {
            long index = listCtrl->InsertItem(i, acciones[i].pid);
            listCtrl->SetItem(index, 1, acciones[i].tipo);
            listCtrl->SetItem(index, 2, acciones[i].recurso);
            listCtrl->SetItem(index, 3, wxString::Format("%d", acciones[i].ciclo));
        }
    }
    
    void Clear() {
        listCtrl->ClearAll();
    }
};

// Panel de métricas
class MetricsPanel : public wxPanel {
private:
    wxStaticText* avgWaitingLabel;
    wxStaticText* avgTurnaroundLabel;
    wxListCtrl* processMetrics;
    
public:
    MetricsPanel(wxWindow* parent) : wxPanel(parent) {
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
        
        wxStaticText* title = new wxStaticText(this, wxID_ANY, "Métricas de Eficiencia");
        title->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        mainSizer->Add(title, 0, wxALL, 10);
        
        avgWaitingLabel = new wxStaticText(this, wxID_ANY, "Tiempo de Espera Promedio: -");
        avgTurnaroundLabel = new wxStaticText(this, wxID_ANY, "Tiempo de Retorno Promedio: -");
        
        mainSizer->Add(avgWaitingLabel, 0, wxALL, 5);
        mainSizer->Add(avgTurnaroundLabel, 0, wxALL, 5);
        
        mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 10);
        
        processMetrics = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                        wxLC_REPORT | wxLC_SINGLE_SEL);
        processMetrics->AppendColumn("PID", wxLIST_FORMAT_LEFT, 100);
        processMetrics->AppendColumn("Tiempo de Espera", wxLIST_FORMAT_CENTER, 150);
        processMetrics->AppendColumn("Tiempo de Retorno", wxLIST_FORMAT_CENTER, 150);
        
        mainSizer->Add(processMetrics, 1, wxEXPAND | wxALL, 5);
        
        SetSizer(mainSizer);
    }
    
    void UpdateMetrics(double avgWaiting, double avgTurnaround, const std::vector<Proceso>& procesos) {
        avgWaitingLabel->SetLabel(wxString::Format("Tiempo de Espera Promedio: %.2f", avgWaiting));
        avgTurnaroundLabel->SetLabel(wxString::Format("Tiempo de Retorno Promedio: %.2f", avgTurnaround));
        
        processMetrics->DeleteAllItems();
        for (size_t i = 0; i < procesos.size(); i++) {
            long index = processMetrics->InsertItem(i, procesos[i].pid);
            processMetrics->SetItem(index, 1, wxString::Format("%d", procesos[i].waiting_time));
            processMetrics->SetItem(index, 2, wxString::Format("%d", procesos[i].turnaround_time));
        }
    }
    
    void Clear() {
        avgWaitingLabel->SetLabel("Tiempo de Espera Promedio: -");
        avgTurnaroundLabel->SetLabel("Tiempo de Retorno Promedio: -");
        processMetrics->DeleteAllItems();
    }
};

// Panel de Calendarización
class SchedulingPanel : public wxPanel {
private:
    wxChoice* algorithmChoice;
    wxSpinCtrl* quantumSpinner;
    wxButton* loadButton;
    wxButton* runButton;
    wxButton* clearButton;
    wxCheckListBox* algorithmList;
    GanttPanel* ganttPanel;
    InfoListPanel* infoPanel;
    MetricsPanel* metricsPanel;
    
    std::vector<Proceso> procesos;
    std::thread* simulationThread = nullptr;
    std::atomic<bool> stopSimulation{false};
    
public:
    SchedulingPanel(wxWindow* parent) : wxPanel(parent) {
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
        
        wxPanel* controlPanel = new wxPanel(this);
        wxBoxSizer* controlSizer = new wxBoxSizer(wxHORIZONTAL);
        
        wxStaticText* algoLabel = new wxStaticText(controlPanel, wxID_ANY, "Algoritmos:");
        controlSizer->Add(algoLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        
        wxArrayString algorithms;
        algorithms.Add("FIFO");
        algorithms.Add("SJF");
        algorithms.Add("SRT");
        algorithms.Add("Round Robin");
        algorithms.Add("Priority");
        
        algorithmList = new wxCheckListBox(controlPanel, wxID_ANY, wxDefaultPosition, wxSize(150, 100), algorithms);
        controlSizer->Add(algorithmList, 0, wxALL, 5);
        
        wxStaticText* quantumLabel = new wxStaticText(controlPanel, wxID_ANY, "Quantum:");
        controlSizer->Add(quantumLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        
        quantumSpinner = new wxSpinCtrl(controlPanel, wxID_ANY, "2", wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 1, 10, 2);
        controlSizer->Add(quantumSpinner, 0, wxALL, 5);
        
        loadButton = new wxButton(controlPanel, wxID_ANY, "Cargar Procesos");
        runButton = new wxButton(controlPanel, wxID_ANY, "Ejecutar Simulación");
        clearButton = new wxButton(controlPanel, wxID_ANY, "Limpiar");
        
        controlSizer->Add(loadButton, 0, wxALL, 5);
        controlSizer->Add(runButton, 0, wxALL, 5);
        controlSizer->Add(clearButton, 0, wxALL, 5);
        
        controlPanel->SetSizer(controlSizer);
        mainSizer->Add(controlPanel, 0, wxEXPAND | wxALL, 5);
        
        wxSplitterWindow* splitter = new wxSplitterWindow(this);
        
        wxPanel* leftPanel = new wxPanel(splitter);
        wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
        
        ganttPanel = new GanttPanel(leftPanel);
        leftSizer->Add(ganttPanel, 1, wxEXPAND | wxALL, 5);
        
        infoPanel = new InfoListPanel(leftPanel);
        leftSizer->Add(infoPanel, 1, wxEXPAND | wxALL, 5);
        
        leftPanel->SetSizer(leftSizer);
        
        metricsPanel = new MetricsPanel(splitter);
        
        splitter->SplitVertically(leftPanel, metricsPanel, 600);
        mainSizer->Add(splitter, 1, wxEXPAND);
        
        SetSizer(mainSizer);
        
        loadButton->Bind(wxEVT_BUTTON, &SchedulingPanel::OnLoadProcesses, this);
        runButton->Bind(wxEVT_BUTTON, &SchedulingPanel::OnRunSimulation, this);
        clearButton->Bind(wxEVT_BUTTON, &SchedulingPanel::OnClear, this);
    }
    
    void OnLoadProcesses(wxCommandEvent& event) {
        wxFileDialog openFileDialog(this, "Cargar archivo de procesos", "", "",
                                    "Archivos de texto (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        
        if (openFileDialog.ShowModal() == wxID_OK) {
            procesos = leerProcesosDesdeArchivo(openFileDialog.GetPath().ToStdString());
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(100, 255);
            
            for (auto& p : procesos) {
                wxColour color(dis(gen), dis(gen), dis(gen));
                ganttPanel->SetProcessColor(p.pid, color);
            }
            
            infoPanel->ShowProcesses(procesos);
            wxMessageBox(wxString::Format("Se cargaron %zu procesos", procesos.size()), "Información");
        }
    }
    
    void OnRunSimulation(wxCommandEvent& event) {
        if (procesos.empty()) {
            wxMessageBox("Primero debe cargar los procesos", "Error", wxICON_ERROR);
            return;
        }
        
        wxArrayInt selections;
        algorithmList->GetCheckedItems(selections);
        
        if (selections.IsEmpty()) {
            wxMessageBox("Debe seleccionar al menos un algoritmo", "Error", wxICON_ERROR);
            return;
        }
        
        ganttPanel->Clear();
        metricsPanel->Clear();
        
        stopSimulation = false;
        runButton->Enable(false);
        
        simulationThread = new std::thread([this, selections]() {
            for (size_t i = 0; i < selections.GetCount() && !stopSimulation; i++) {
                int algo = selections[i];
                std::vector<Proceso> procesosTemp = procesos;
                
                // Callback para actualizar GUI
                auto updateGUI = [this](const std::string& pid, int cycle) {
                    wxTheApp->CallAfter([this, pid, cycle]() {
                        ganttPanel->AddTimeSlot(pid, cycle);
                    });
                };
                
                SimulationResult result;
                // Algoritmos de calendarizacion
                switch (algo) {
                    case 0: // FIFO
                        result = fifo(procesosTemp, updateGUI);
                        break;
                    case 1: // SJF
                        result = sjf(procesosTemp, updateGUI);
                        break;
                    case 2: // SRT
                        result = srt(procesosTemp, updateGUI);
                        break;
                    case 3: // Round Robin
                        result = roundRobin(procesosTemp, quantumSpinner->GetValue(), updateGUI);
                        break;
                    case 4: // Priority
                        result = priority(procesosTemp, updateGUI);
                        break;
                }
                
                // Actualizar métricas
                wxTheApp->CallAfter([this, result, procesosTemp]() {
                    metricsPanel->UpdateMetrics(result.avgWaitingTime, result.avgTurnaroundTime, procesosTemp);
                });
                
                // Pausa entre algoritmos si hay más de uno
                if (i < selections.GetCount() - 1) {
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            }
            
            wxTheApp->CallAfter([this]() {
                runButton->Enable(true);
            });
        });
    }
    
    void OnClear(wxCommandEvent& event) {
        stopSimulation = true;
        if (simulationThread && simulationThread->joinable()) {
            simulationThread->join();
            delete simulationThread;
            simulationThread = nullptr;
        }
        
        procesos.clear();
        ganttPanel->Clear();
        infoPanel->Clear();
        metricsPanel->Clear();
        runButton->Enable(true);
    }
    
    ~SchedulingPanel() {
        stopSimulation = true;
        if (simulationThread && simulationThread->joinable()) {
            simulationThread->join();
            delete simulationThread;
        }
    }
};

class SyncPanel : public wxPanel {
private:
    wxRadioBox* syncModeRadio;
    wxButton* loadProcessesButton;
    wxButton* loadResourcesButton;
    wxButton* loadActionsButton;
    wxButton* runButton;
    wxButton* clearButton;
    GanttPanel* ganttPanel;
    InfoListPanel* processInfoPanel;
    InfoListPanel* resourceInfoPanel;
    InfoListPanel* actionInfoPanel;
    
    std::vector<Proceso> procesos;
    std::map<std::string, Recurso> recursos;
    std::vector<Accion> acciones;
    std::thread* simulationThread = nullptr;
    std::atomic<bool> stopSimulation{false};
    
public:
    SyncPanel(wxWindow* parent) : wxPanel(parent) {
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
        
        // Panel de controles
        wxPanel* controlPanel = new wxPanel(this);
        wxBoxSizer* controlSizer = new wxBoxSizer(wxHORIZONTAL);
        
        wxArrayString syncModes;
        syncModes.Add("Mutex");
        syncModes.Add("Semaforo");
        syncModeRadio = new wxRadioBox(controlPanel, wxID_ANY, "Modo de Sincronización",
                                       wxDefaultPosition, wxDefaultSize, syncModes, 1, wxRA_SPECIFY_ROWS);
        controlSizer->Add(syncModeRadio, 0, wxALL, 5);
        
        loadProcessesButton = new wxButton(controlPanel, wxID_ANY, "Cargar Procesos");
        loadResourcesButton = new wxButton(controlPanel, wxID_ANY, "Cargar Recursos");
        loadActionsButton = new wxButton(controlPanel, wxID_ANY, "Cargar Acciones");
        runButton = new wxButton(controlPanel, wxID_ANY, "Ejecutar Simulación");
        clearButton = new wxButton(controlPanel, wxID_ANY, "Limpiar");
        
        controlSizer->Add(loadProcessesButton, 0, wxALL, 5);
        controlSizer->Add(loadResourcesButton, 0, wxALL, 5);
        controlSizer->Add(loadActionsButton, 0, wxALL, 5);
        controlSizer->Add(runButton, 0, wxALL, 5);
        controlSizer->Add(clearButton, 0, wxALL, 5);
        
        controlPanel->SetSizer(controlSizer);
        mainSizer->Add(controlPanel, 0, wxEXPAND | wxALL, 5);
        
        // Splitter para Gantt e información
        wxSplitterWindow* splitter = new wxSplitterWindow(this);
        
        // Panel izquierdo con Gantt
        ganttPanel = new GanttPanel(splitter);
        ganttPanel->SetSyncMode(true);
        
        // Panel derecho con información
        wxNotebook* infoNotebook = new wxNotebook(splitter, wxID_ANY);
        
        processInfoPanel = new InfoListPanel(infoNotebook);
        resourceInfoPanel = new InfoListPanel(infoNotebook);
        actionInfoPanel = new InfoListPanel(infoNotebook);
        
        infoNotebook->AddPage(processInfoPanel, "Procesos");
        infoNotebook->AddPage(resourceInfoPanel, "Recursos");
        infoNotebook->AddPage(actionInfoPanel, "Acciones");
        
        splitter->SplitVertically(ganttPanel, infoNotebook, 600);
        mainSizer->Add(splitter, 1, wxEXPAND);
        
        SetSizer(mainSizer);
        
        // Eventos
        loadProcessesButton->Bind(wxEVT_BUTTON, &SyncPanel::OnLoadProcesses, this);
        loadResourcesButton->Bind(wxEVT_BUTTON, &SyncPanel::OnLoadResources, this);
        loadActionsButton->Bind(wxEVT_BUTTON, &SyncPanel::OnLoadActions, this);
        runButton->Bind(wxEVT_BUTTON, &SyncPanel::OnRunSimulation, this);
        clearButton->Bind(wxEVT_BUTTON, &SyncPanel::OnClear, this);
    }
    
    void OnLoadProcesses(wxCommandEvent& event) {
        wxFileDialog openFileDialog(this, "Cargar archivo de procesos", "", "",
                                    "Archivos de texto (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        
        if (openFileDialog.ShowModal() == wxID_OK) {
            procesos = leerProcesosDesdeArchivo(openFileDialog.GetPath().ToStdString());
            processInfoPanel->ShowProcesses(procesos);
            wxMessageBox(wxString::Format("Se cargaron %zu procesos", procesos.size()), "Información");
        }
    }
    
    void OnLoadResources(wxCommandEvent& event) {
        wxFileDialog openFileDialog(this, "Cargar archivo de recursos", "", "",
                                    "Archivos de texto (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        
        if (openFileDialog.ShowModal() == wxID_OK) {
            recursos = leerRecursosDesdeArchivo(openFileDialog.GetPath().ToStdString());
            resourceInfoPanel->ShowResources(recursos);
            wxMessageBox(wxString::Format("Se cargaron %zu recursos", recursos.size()), "Información");
        }
    }
    
    void OnLoadActions(wxCommandEvent& event) {
        wxFileDialog openFileDialog(this, "Cargar archivo de acciones", "", "",
                                    "Archivos de texto (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        
        if (openFileDialog.ShowModal() == wxID_OK) {
            acciones = leerAccionesDesdeArchivo(openFileDialog.GetPath().ToStdString());
            actionInfoPanel->ShowActions(acciones);
            wxMessageBox(wxString::Format("Se cargaron %zu acciones", acciones.size()), "Información");
        }
    }
    
    void OnRunSimulation(wxCommandEvent& event) {
        if (procesos.empty() || recursos.empty() || acciones.empty()) {
            wxMessageBox("Debe cargar procesos, recursos y acciones antes de ejecutar", "Error", wxICON_ERROR);
            return;
        }
        
        ganttPanel->Clear();
        stopSimulation = false;
        runButton->Enable(false);
        
        bool isMutex = (syncModeRadio->GetSelection() == 0);
        
        simulationThread = new std::thread([this, isMutex]() {
            if (isMutex) {
                SimulateMutex();
            } else {
                SimulateSemaphore();
            }
            
            wxTheApp->CallAfter([this]() {
                runButton->Enable(true);
            });
        });
    }
    
    void SimulateMutex() {
        int ciclo = 0;
        int max_ciclo = 0;
        for (const auto& a : acciones) {
            if (a.ciclo > max_ciclo) max_ciclo = a.ciclo;
        }
        max_ciclo += 5;
        
        for (; ciclo <= max_ciclo && !stopSimulation; ciclo++) {
            bool accion_realizada = false;
            
            for (const auto& a : acciones) {
                if (a.ciclo == ciclo) {
                    auto& recurso = recursos[a.recurso];
                    std::string bloque;
                    
                    if (!recurso.ocupado) {
                        recurso.ocupado = true;
                        recurso.proceso_actual = a.pid;
                        bloque = a.pid + "-" + a.tipo + "-" + a.recurso + "-ACCESSED";
                    } else {
                        bloque = a.pid + "-" + a.tipo + "-" + a.recurso + "-WAITING";
                    }
                    
                    wxTheApp->CallAfter([this, bloque, ciclo]() {
                        ganttPanel->AddTimeSlot(bloque, ciclo);
                    });
                    
                    accion_realizada = true;
                }
            }
            
            // Liberar recursos
            for (auto& [nombre, recurso] : recursos) {
                if (recurso.ocupado) {
                    recurso.ocupado = false;
                    recurso.proceso_actual = "";
                }
            }
            
            if (!accion_realizada) {
                wxTheApp->CallAfter([this, ciclo]() {
                    ganttPanel->AddTimeSlot("CPU IDLE", ciclo);
                });
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }
    
    void SimulateSemaphore() {
        int ciclo = 0;
        int max_ciclo = 0;
        for (const auto& a : acciones) {
            if (a.ciclo > max_ciclo) max_ciclo = a.ciclo;
        }
        max_ciclo += 5;
        
        std::map<std::string, std::map<std::string, int>> procesos_uso;
        
        for (; ciclo <= max_ciclo && !stopSimulation; ciclo++) {
            bool accion_realizada = false;
            
            for (const auto& a : acciones) {
                if (a.ciclo == ciclo) {
                    auto& recurso = recursos[a.recurso];
                    std::string bloque;
                    
                    if (recurso.contador > 0) {
                        recurso.contador--;
                        procesos_uso[a.recurso][a.pid] = 1;
                        bloque = a.pid + "-" + a.tipo + "-" + a.recurso + "-ACCESSED";
                    } else {
                        bloque = a.pid + "-" + a.tipo + "-" + a.recurso + "-WAITING";
                    }
                    
                    wxTheApp->CallAfter([this, bloque, ciclo]() {
                        ganttPanel->AddTimeSlot(bloque, ciclo);
                    });
                    
                    accion_realizada = true;
                }
            }
            
            // Liberar recursos
            for (auto& [nombre, recurso] : recursos) {
                std::vector<std::string> terminados;
                for (auto& [pid, tiempo] : procesos_uso[nombre]) {
                    tiempo--;
                    if (tiempo <= 0) {
                        recurso.contador++;
                        terminados.push_back(pid);
                    }
                }
                for (const auto& pid : terminados) {
                    procesos_uso[nombre].erase(pid);
                }
            }
            
            if (!accion_realizada) {
                wxTheApp->CallAfter([this, ciclo]() {
                    ganttPanel->AddTimeSlot("CPU IDLE", ciclo);
                });
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }
    
    void OnClear(wxCommandEvent& event) {
        stopSimulation = true;
        if (simulationThread && simulationThread->joinable()) {
            simulationThread->join();
            delete simulationThread;
            simulationThread = nullptr;
        }
        
        procesos.clear();
        recursos.clear();
        acciones.clear();
        ganttPanel->Clear();
        processInfoPanel->Clear();
        resourceInfoPanel->Clear();
        actionInfoPanel->Clear();
        runButton->Enable(true);
    }
    
    ~SyncPanel() {
        stopSimulation = true;
        if (simulationThread && simulationThread->joinable()) {
            simulationThread->join();
            delete simulationThread;
        }
    }
};

class MainWindow : public wxFrame {
private:
    wxNotebook* notebook;
    
public:
    MainWindow() : wxFrame(nullptr, wxID_ANY, "Simulador de Scheduling y Sincronización",
                           wxDefaultPosition, wxSize(1200, 800)) {
        wxMenuBar* menuBar = new wxMenuBar();
        
        wxMenu* fileMenu = new wxMenu();
        fileMenu->Append(wxID_EXIT, "Salir\tCtrl+Q");
        menuBar->Append(fileMenu, "Archivo");
        
        wxMenu* helpMenu = new wxMenu();
        helpMenu->Append(wxID_ABOUT, "Acerca de...");
        menuBar->Append(helpMenu, "Ayuda");
        
        SetMenuBar(menuBar);
        
        notebook = new wxNotebook(this, wxID_ANY);
        
        SchedulingPanel* schedulingPanel = new SchedulingPanel(notebook);
        SyncPanel* syncPanel = new SyncPanel(notebook);
        
        notebook->AddPage(schedulingPanel, "Calendarización");
        notebook->AddPage(syncPanel, "Sincronización");
        
        CreateStatusBar();
        SetStatusText("Listo para simular");
        
        // Eventos
        Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);
        Bind(wxEVT_MENU, &MainWindow::OnAbout, this, wxID_ABOUT);
        
        Centre();
    }
    
    void OnExit(wxCommandEvent& event) {
        Close(true);
    }
    
    void OnAbout(wxCommandEvent& event) {
        wxMessageBox("Simulador de Scheduling y Sincronización\n\n"
                     "Permite simular algoritmos de calendarización y mecanismos de sincronización.\n\n"
                     "Versión 1.0", "Acerca de", wxOK | wxICON_INFORMATION);
    }
};

class SchedulerApp : public wxApp {
public:
    bool OnInit() override {
        MainWindow* frame = new MainWindow();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(SchedulerApp);
