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

// Panel de Diagrama de Gantt MEJORADO
class GanttPanel : public wxScrolledWindow {
private:
    struct TimeSlot {
        std::string content;
        int cycle;
        bool isWaiting;
        bool isIdle;
        std::string processId;
        std::string resource;
        std::string action;
        std::string status;
    };
    
    std::vector<TimeSlot> timeline;
    std::map<std::string, wxColour> processColors;
    int currentCycle = 0;
    int blockWidth = 120;  // Más ancho para más información
    int blockHeight = 60;  // Más alto para múltiples líneas
    bool isSync = false;
    
public:
    GanttPanel(wxWindow* parent) : wxScrolledWindow(parent) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetVirtualSize(800, 200);
        SetScrollRate(10, 0);
        Bind(wxEVT_PAINT, &GanttPanel::OnPaint, this);
    }
    
    void SetSyncMode(bool sync) { isSync = sync; }
    
    void AddTimeSlot(const std::string& content, int cycle) {
        TimeSlot slot;
        slot.content = content;
        slot.cycle = cycle;
        slot.isIdle = (content == "CPU IDLE" || content == "IDLE");
        slot.isWaiting = (content.find("WAITING") != std::string::npos);
        
        // Parsear contenido para modo sincronización
        if (isSync && !slot.isIdle) {
            std::vector<std::string> parts;
            std::stringstream ss(content);
            std::string part;
            
            while (std::getline(ss, part, '-')) {
                parts.push_back(part);
            }
            
            if (parts.size() >= 4) {
                slot.processId = parts[0];
                slot.action = parts[1];
                slot.resource = parts[2];
                slot.status = parts[3];
            }
        } else if (!isSync) {
            slot.processId = content;
        }
        
        timeline.push_back(slot);
        currentCycle = cycle;
        
        int requiredWidth = (timeline.size() + 2) * blockWidth;
        if (requiredWidth > GetVirtualSize().GetWidth()) {
            SetVirtualSize(requiredWidth, 200);
        }
        
        Refresh();
        Scroll(std::max(0, (int)(timeline.size() * blockWidth / 10) - 50), 0);
    }
     
    void Clear() {
        timeline.clear();
        currentCycle = 0;
        SetVirtualSize(800, 200);
        Refresh();
    }
    
    void OnPaint(wxPaintEvent& event) {
        wxAutoBufferedPaintDC dc(this);
        DoPrepareDC(dc);
        
        dc.Clear();
        dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        
        int x = 10;
        int y = 40;  // Más espacio arriba para el ciclo
        
        for (size_t i = 0; i < timeline.size(); i++) {
            const auto& slot = timeline[i];
            
            // Dibujar número de ciclo arriba
            dc.SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
            dc.SetTextForeground(wxColour(0, 0, 0));
            wxString cycleText = wxString::Format("C%d", slot.cycle);
            wxSize cycleSize = dc.GetTextExtent(cycleText);
            dc.DrawText(cycleText, x + (blockWidth - cycleSize.x) / 2, 5);
            
            // Configurar colores y bordes según el estado
            if (slot.isIdle) {
                dc.SetBrush(wxBrush(wxColour(220, 220, 220)));
                dc.SetPen(wxPen(wxColour(100, 100, 100), 1, wxPENSTYLE_SOLID));
            } else if (slot.isWaiting) {
                dc.SetBrush(wxBrush(wxColour(255, 200, 200)));  // Rojo claro para waiting
                dc.SetPen(wxPen(wxColour(200, 0, 0), 2, wxPENSTYLE_DOT));  // Borde punteado
            } else {
                // Estado ACCESSED - usar color del proceso
                if (processColors.find(slot.processId) != processColors.end()) {
                    dc.SetBrush(wxBrush(processColors[slot.processId]));
                } else {
                    dc.SetBrush(wxBrush(wxColour(200, 255, 200)));  // Verde claro por defecto
                }
                dc.SetPen(wxPen(wxColour(0, 150, 0), 2, wxPENSTYLE_SOLID));  // Borde verde
            }
            
            dc.DrawRectangle(x, y, blockWidth - 5, blockHeight);
            
            // Dibujar contenido según el modo
            dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            
            if (slot.isIdle) {
                dc.SetTextForeground(wxColour(100, 100, 100));
                wxSize textSize = dc.GetTextExtent("CPU IDLE");
                dc.DrawText("CPU IDLE", x + (blockWidth - textSize.x) / 2, y + (blockHeight - textSize.y) / 2);
            } else if (isSync) {
                // Modo sincronización - mostrar información detallada
                dc.SetTextForeground(wxColour(0, 0, 0));
                int lineHeight = 12;
                int textY = y + 5;
                
                dc.DrawText(slot.processId, x + 5, textY);
                dc.DrawText(slot.action, x + 5, textY + lineHeight);
                dc.DrawText(slot.resource, x + 5, textY + lineHeight * 2);
                
                // Estado con color específico
                if (slot.isWaiting) {
                    dc.SetTextForeground(wxColour(180, 0, 0));
                    dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
                } else {
                    dc.SetTextForeground(wxColour(0, 120, 0));
                    dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
                }
                dc.DrawText(slot.status, x + 5, textY + lineHeight * 3);
            } else {
                // Modo calendarización - mostrar solo PID
                dc.SetTextForeground(wxColour(0, 0, 0));
                wxSize textSize = dc.GetTextExtent(slot.processId);
                dc.DrawText(slot.processId, x + (blockWidth - textSize.x) / 2, y + (blockHeight - textSize.y) / 2);
            }
            
            x += blockWidth;
        }
        
        // Mostrar ciclo actual
        dc.SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        dc.SetTextForeground(wxColour(200, 0, 0));
        dc.DrawText(wxString::Format("Ciclo actual: %d", currentCycle), 10, y + blockHeight + 15);
        
        // Leyenda para modo sincronización
        if (isSync) {
            int legendY = y + blockHeight + 40;
            dc.SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
            
            // ACCESSED
            dc.SetBrush(wxBrush(wxColour(200, 255, 200)));
            dc.SetPen(wxPen(wxColour(0, 150, 0), 2));
            dc.DrawRectangle(10, legendY, 20, 15);
            dc.SetTextForeground(wxColour(0, 0, 0));
            dc.DrawText("ACCESSED", 35, legendY + 2);
            
            // WAITING
            dc.SetBrush(wxBrush(wxColour(255, 200, 200)));
            dc.SetPen(wxPen(wxColour(200, 0, 0), 2, wxPENSTYLE_DOT));
            dc.DrawRectangle(120, legendY, 20, 15);
            dc.DrawText("WAITING", 145, legendY + 2);
            
            // CPU IDLE
            dc.SetBrush(wxBrush(wxColour(220, 220, 220)));
            dc.SetPen(wxPen(wxColour(100, 100, 100), 1));
            dc.DrawRectangle(220, legendY, 20, 15);
            dc.DrawText("CPU IDLE", 245, legendY + 2);
        }
    }
    
    void SetProcessColor(const std::string& processId, const wxColour& color) {
        processColors[processId] = color;
    }
};

// ... InfoListPanel y MetricsPanel se mantienen igual ...

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
        listCtrl->AppendColumn("Contador Inicial", wxLIST_FORMAT_CENTER, 120);
        listCtrl->AppendColumn("Contador Actual", wxLIST_FORMAT_CENTER, 120);
        listCtrl->AppendColumn("Estado", wxLIST_FORMAT_CENTER, 100);
        
        int i = 0;
        for (const auto& [nombre, recurso] : recursos) {
            long index = listCtrl->InsertItem(i++, nombre);
            listCtrl->SetItem(index, 1, wxString::Format("%d", recurso.contador_inicial));
            listCtrl->SetItem(index, 2, wxString::Format("%d", recurso.contador));
            
            std::string estado;
            if (recurso.ocupado) {
                estado = "Ocupado por " + recurso.proceso_actual;
            } else if (recurso.contador < recurso.contador_inicial) {
                estado = wxString::Format("En uso (%d/%d)", 
                    recurso.contador_inicial - recurso.contador, recurso.contador_inicial).ToStdString();
            } else {
                estado = "Disponible";
            }
            listCtrl->SetItem(index, 3, estado);
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

// SchedulingPanel se mantiene igual...
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
        runButton = new wxButton(controlPanel, wxID_ANY, "Ejecutar Simulacion");
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
            
            ganttPanel->Clear();
            
            std::vector<wxColour> predefinedColors = {
                wxColour(255, 100, 100),  // Rojo claro
                wxColour(100, 255, 100),  // Verde claro
                wxColour(100, 100, 255),  // Azul claro
                wxColour(255, 255, 100),  // Amarillo
                wxColour(255, 100, 255),  // Magenta
                wxColour(100, 255, 255),  // Cyan
                wxColour(255, 180, 100),  // Naranja
                wxColour(180, 100, 255),  // Púrpura
                wxColour(255, 180, 180),  // Rosa claro
                wxColour(180, 255, 180)   // Verde menta
            };
            
            for (size_t i = 0; i < procesos.size(); i++) {
                wxColour color;
                if (i < predefinedColors.size()) {
                    color = predefinedColors[i];
                } else {
                    int r = 100 + ((i * 67) % 156);
                    int g = 100 + ((i * 89) % 156);
                    int b = 100 + ((i * 113) % 156);
                    color = wxColour(r, g, b);
                }
                
                ganttPanel->SetProcessColor(procesos[i].pid, color);
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

// SyncPanel CORREGIDO
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
    
    // Estructura para trackear operaciones activas en mutex
    struct OperacionActiva {
        std::string pid;
        std::string recurso;
        std::string tipo;
        int ciclo_inicio;
        int duracion;
    };
    
    std::vector<OperacionActiva> operaciones_activas;
    
public:
    SyncPanel(wxWindow* parent) : wxPanel(parent) {
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
        
        // Panel de controles
        wxPanel* controlPanel = new wxPanel(this);
        wxBoxSizer* controlSizer = new wxBoxSizer(wxHORIZONTAL);
        
        wxArrayString syncModes;
        syncModes.Add("Mutex");
        syncModes.Add("Semaforo");
        syncModeRadio = new wxRadioBox(controlPanel, wxID_ANY, "Modo de Sincronizacion",
                                       wxDefaultPosition, wxDefaultSize, syncModes, 1, wxRA_SPECIFY_ROWS);
        controlSizer->Add(syncModeRadio, 0, wxALL, 5);
        
        loadProcessesButton = new wxButton(controlPanel, wxID_ANY, "Cargar Procesos");
        loadResourcesButton = new wxButton(controlPanel, wxID_ANY, "Cargar Recursos");
        loadActionsButton = new wxButton(controlPanel, wxID_ANY, "Cargar Acciones");
        runButton = new wxButton(controlPanel, wxID_ANY, "Ejecutar Simulacion");
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
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(100, 255);
            
            for (const auto& p : procesos) {
                wxColour color(dis(gen), dis(gen), dis(gen));
                ganttPanel->SetProcessColor(p.pid, color);
            }
            
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
        max_ciclo += 10; // Margen adicional
        
        // Resetear estado de recursos
        for (auto& [nombre, recurso] : recursos) {
            recurso.ocupado = false;
            recurso.proceso_actual = "";
        }
        
        operaciones_activas.clear();
        
        for (; ciclo <= max_ciclo && !stopSimulation; ciclo++) {
            bool accion_realizada = false;
            
            // Procesar nuevas acciones del ciclo actual
            for (const auto& a : acciones) {
                if (a.ciclo == ciclo) {
                    auto& recurso = recursos[a.recurso];
                    std::string bloque;
                    
                    if (!recurso.ocupado) {
                        // Recurso disponible - ACCESSED
                        recurso.ocupado = true;
                        recurso.proceso_actual = a.pid;
                        bloque = a.pid + "-" + a.tipo + "-" + a.recurso + "-ACCESSED";
                        
                        // Agregar operación activa (duración 2 ciclos para mutex)
                        operaciones_activas.push_back({a.pid, a.recurso, a.tipo, ciclo, 2});
                        
                    } else {
                        // Recurso ocupado - WAITING
                        bloque = a.pid + "-" + a.tipo + "-" + a.recurso + "-WAITING";
                    }
                    
                    wxTheApp->CallAfter([this, bloque, ciclo]() {
                        ganttPanel->AddTimeSlot(bloque, ciclo);
                    });
                    
                    accion_realizada = true;
                    
                    // Actualizar panel de recursos
                    wxTheApp->CallAfter([this]() {
                        resourceInfoPanel->ShowResources(recursos);
                    });
                }
            }
            
            // Liberar recursos de operaciones completadas
            std::vector<size_t> operaciones_terminadas;
            for (size_t i = 0; i < operaciones_activas.size(); i++) {
                auto& op = operaciones_activas[i];
                if (ciclo >= op.ciclo_inicio + op.duracion) {
                    // Liberar el recurso
                    recursos[op.recurso].ocupado = false;
                    recursos[op.recurso].proceso_actual = "";
                    operaciones_terminadas.push_back(i);
                }
            }
            
            // Eliminar operaciones terminadas (en orden inverso)
            for (int i = operaciones_terminadas.size() - 1; i >= 0; i--) {
                operaciones_activas.erase(operaciones_activas.begin() + operaciones_terminadas[i]);
            }
            
            if (!accion_realizada) {
                wxTheApp->CallAfter([this, ciclo]() {
                    ganttPanel->AddTimeSlot("CPU IDLE", ciclo);
                });
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    
    void SimulateSemaphore() {
        int ciclo = 0;
        int max_ciclo = 0;
        
        for (const auto& a : acciones) {
            if (a.ciclo > max_ciclo) max_ciclo = a.ciclo;
        }
        max_ciclo += 10;
        
        // Resetear estado de recursos
        for (auto& [nombre, recurso] : recursos) {
            recurso.contador = recurso.contador_inicial;
            recurso.procesos_uso.clear();
        }
        
        for (; ciclo <= max_ciclo && !stopSimulation; ciclo++) {
            bool accion_realizada = false;
            
            // Procesar nuevas acciones del ciclo actual
            for (const auto& a : acciones) {
                if (a.ciclo == ciclo) {
                    auto& recurso = recursos[a.recurso];
                    std::string bloque;
                    
                    if (recurso.contador > 0) {
                        // Hay recursos disponibles - ACCESSED
                        recurso.contador--;
                        recurso.procesos_uso[a.pid] = 3; // Duración de 3 ciclos para semáforos
                        bloque = a.pid + "-" + a.tipo + "-" + a.recurso + "-ACCESSED";
                    } else {
                        // No hay recursos disponibles - WAITING
                        bloque = a.pid + "-" + a.tipo + "-" + a.recurso + "-WAITING";
                    }
                    
                    wxTheApp->CallAfter([this, bloque, ciclo]() {
                        ganttPanel->AddTimeSlot(bloque, ciclo);
                    });
                    
                    accion_realizada = true;
                    
                    // Actualizar panel de recursos
                    wxTheApp->CallAfter([this]() {
                        resourceInfoPanel->ShowResources(recursos);
                    });
                }
            }
            
            // Decrementar tiempo de uso y liberar recursos
            for (auto& [nombre, recurso] : recursos) {
                std::vector<std::string> procesos_terminados;
                
                for (auto& [pid, tiempo_restante] : recurso.procesos_uso) {
                    tiempo_restante--;
                    if (tiempo_restante <= 0) {
                        recurso.contador++; // Liberar el recurso
                        procesos_terminados.push_back(pid);
                    }
                }
                
                // Eliminar procesos que terminaron
                for (const auto& pid : procesos_terminados) {
                    recurso.procesos_uso.erase(pid);
                }
            }
            
            if (!accion_realizada) {
                wxTheApp->CallAfter([this, ciclo]() {
                    ganttPanel->AddTimeSlot("CPU IDLE", ciclo);
                });
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
        operaciones_activas.clear();
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
                           wxDefaultPosition, wxSize(1400, 900)) {
        wxMenuBar* menuBar = new wxMenuBar();
        
        wxMenu* fileMenu = new wxMenu();
        fileMenu->Append(wxID_EXIT, "Salir\tCtrl+Q");
        menuBar->Append(fileMenu, "Archivo");
        
        wxMenu* helpMenu = new wxMenu();
        helpMenu->Append(wxID_ABOUT, "Acerca de\tF1");
        menuBar->Append(helpMenu, "Ayuda");
        
        SetMenuBar(menuBar);
        
        notebook = new wxNotebook(this, wxID_ANY);
        
        SchedulingPanel* schedulingPanel = new SchedulingPanel(notebook);
        SyncPanel* syncPanel = new SyncPanel(notebook);
        
        notebook->AddPage(schedulingPanel, "Calendarizacion");
        notebook->AddPage(syncPanel, "Sincronizacion");
        
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
                     "Características:\n"
                     "• Algoritmos: FIFO, SJF, SRT, Round Robin, Priority\n"
                     "• Sincronización: Mutex y Semáforos\n"
                     "• Visualización en tiempo real\n"
                     "• Métricas de rendimiento\n\n"
                     "Versión 2.0", "Acerca de", wxOK | wxICON_INFORMATION);
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
