// Se esta usando wxwidgets [con gtk] 
#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/splitter.h>
#include <wx/filedlg.h>
#include <wx/choice.h>
#include <wx/spinctrl.h>
#include <wx/scrolwin.h>
#include <wx/dcbuffer.h>
#include <wx/timer.h>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>

// Estructura de Proceso
struct Proceso {
    std::string pid;
    int burst_time;
    int arrival_time;
    int priority;
    int waiting_time = 0;
    int turnaround_time = 0;
    int remaining_time = 0;
    int start_time = -1;
    bool terminado = false;
    bool en_cola = false;
    wxColour color;
};

// Panel de Diagrama de Gantt
class GanttPanel : public wxScrolledWindow {
private:
    std::vector<std::pair<std::string, int>> timeline;
    std::map<std::string, wxColour> processColors;
    int currentCycle = 0;
    int blockWidth = 40;
    int blockHeight = 50;
    
public:
    GanttPanel(wxWindow* parent) : wxScrolledWindow(parent) {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetVirtualSize(800, 150);
        SetScrollRate(10, 0);
        
        Bind(wxEVT_PAINT, &GanttPanel::OnPaint, this);
    }
    
    void AddTimeSlot(const std::string& pid, int cycle) {
        timeline.push_back({pid, cycle});
        currentCycle = cycle;
        
        int requiredWidth = (timeline.size() + 2) * blockWidth;
        if (requiredWidth > GetVirtualSize().GetWidth()) {
            SetVirtualSize(requiredWidth, 150);
        }
        
        Refresh();
        // Auto-scroll al final
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
            
            // Dibujar el bloque del proceso
            if (slot.first == "IDLE") {
                dc.SetBrush(wxBrush(wxColour(200, 200, 200)));
            } else {
                dc.SetBrush(wxBrush(processColors[slot.first]));
            }
            
            dc.DrawRectangle(x, y, blockWidth, blockHeight);
            
            // Dibujar el nombre del proceso
            dc.DrawText(slot.first, x + 5, y + 20);
            
            // Dibujar el número de ciclo
            dc.DrawText(wxString::Format("%d", slot.second), x + 10, y + blockHeight + 5);
            
            x += blockWidth;
        }
        
        // Dibujar etiqueta de ciclo actual
        dc.SetTextForeground(wxColour(255, 0, 0));
        dc.DrawText(wxString::Format("Ciclo actual: %d", currentCycle), 10, 5);
    }
};
