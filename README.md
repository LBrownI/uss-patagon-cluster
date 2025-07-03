# 🖧 Raspberry Pi Cluster – High Performance on a Budget

Un clúster distribuido utilizando Raspberry Pi 4 y MPI para explorar computación paralela, balanceo de carga y otras funcionalidades de MPI.

## 📝 Descripción

Este proyecto crea un **clúster funcional con Raspberry Pi 4**, donde:

- Cada Pi actúa como nodo de cómputo.
- Se usa **MPI (Message Passing Interface)** para tareas paralelas.
- Se pueden ejecutar programas en C/C++ distribuidos entre nodos.
- El sistema permite escalar a más nodos fácilmente.
- Se automatiza la distribución de archivos y la ejecución remota.

---

## 🚀 Tecnologías Usadas

- **Lenguaje:** C++ con MPI
- **Hardware:** Raspberry Pi 4 x4  
- **Sistema Operativo:** Raspberry Pi OS Lite (headless)  
- **Red:** Ethernet + switch  
- **Automatización:** SSH, `rsync`, `tmux`, `bash scripts`  


---

## 🛠️ Instalación y Configuración

### 🔧 Requisitos Previos

- 4 Raspberry Pi 4 con Raspberry Pi OS x1 Raspberry Pi OS Lite x4.
- Conectividad por SSH habilitada entre los nodos.
- Todos los nodos conectados a una misma red.
- El nodo maestro con acceso root por SSH al resto.

### 🧰 Configuración de Red y SSH

### 🖥️Instalation of MPI

### 🔗 Enlaces Útiles

### 👥 Autores
