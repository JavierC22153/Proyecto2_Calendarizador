# 🗓️ Proyecto2_Calendarizador

**Autores:**

- Gustavo Adolfo Cruz Bardales - 22779  
- Javier Andrés Chen - 22153

---
## Repositorio

[Repositorio](https://github.com/JavierC22153/Proyecto2_Calendarizador.git)
---

## 📋 Descripción

Este proyecto implementa un simulador visual de algoritmos de planificación de procesos. Los algoritmos incluidos son:

- ✅ FIFO (First-In First-Out)  
- ✅ SJF (Shortest Job First)  
- ✅ SRT (Shortest Remaining Time)  
- ✅ Round Robin  
- ✅ Prioridad  
- ✅ Simulador de mutex  
- ✅ Simulador de semáforos

La interfaz gráfica ha sido desarrollada con la biblioteca **wxWidgets** y se necesitan paquetes de **gtk**, permitiendo una interacción intuitiva con el sistema de planificación.

---

## 🧰 Requisitos

Asegúrate de tener instaladas las siguientes herramientas antes de compilar:

### 🔧 Dependencias principales

- `g++` (compatible con C++17)
- `make`
- `wxWidgets` 3.2 o superior

### 💡 Instalación de wxWidgets en Debian/Ubuntu:

```bash
sudo apt update
sudo apt install libwxgtk3.2-dev
```

## Clonar el repositorio
```bash
git clone https://github.com/JavierC22153/Proyecto2_Calendarizador.git 
cd Proyecto2_Calendarizador
```

## Compilar el proyecto
```bash
make
```

## Ejecutar el simulador
```bash
./scheduler_simulator
```
## Limpiar los archivos de compilación
```bash
make clean
```
---

## 📂 Formato de carga de archivos

El simulador permite seleccionar archivos `.txt` desde la interfaz gráfica. Únicamente debes colocarlos dentro de la carpeta y luego solo debes seleccionarlos desde la **GUI del programa** cuando se solicite.

A continuación se detalla el formato que deben seguir los archivos según su tipo:

### 📄 Procesos
**Formato de archivo (una línea por proceso):**

```
<PID>, <BT>, <AT>, <Priority>
```

**Ejemplo:**
```
P1, 8, 0, 1
P2, 4, 2, 3
P3, 5, 1, 2
```

- `PID`: Identificador del proceso  
- `BT`: Burst Time (tiempo de ejecución)  
- `AT`: Arrival Time (tiempo de llegada)  
- `Priority`: Prioridad del proceso (menor número = mayor prioridad)

---

### 📄 Recursos
**Formato de archivo (una línea por recurso):**
```
<PID>, <BT>, <AT>, <Priority>
```

**Ejemplo:**
```
P1, 8, 0, 1
P2, 5, 3, 2
```

*(Este archivo se utiliza para definir procesos asociados a recursos, con el mismo formato que los procesos estándar.)*

---

### ⚙️ Acciones
**Formato de archivo (una línea por acción):**
```
<PID>, <ACCIÓN>, <RECURSO>, <CICLO>
```

**Ejemplo:**
```
P1, READ, R1, 0
P2, WRITE, R2, 3
```

- `PID`: Identificador del proceso que realiza la acción  
- `ACCIÓN`: Tipo de acción (`READ`, `WRITE`, etc.)  
- `RECURSO`: Recurso involucrado (`R1`, `R2`, etc.)  
- `CICLO`: Ciclo del CPU en el que se debe ejecutar la acción

---

📌 **Nota:** Todos los archivos deben tener extensión `.txt` y cargarse exclusivamente desde la **interfaz gráfica del simulador**.

