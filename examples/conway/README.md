# Conway MPI - Juego de la Vida Paralelo

Este ejemplo implementa el **Juego de la Vida** usando **MPI**, dividiendo el tablero por filas entre procesos y sincronizando las fronteras.


## ⚙️ Compilación

### Con `mpic++` directamente:

```bash
mpic++ mpi_life.cpp -o mpi_life
```

### Con `CMake`:

```bash
mkdir build
cd build
cmake ..
make
```

---

## ▶️ Ejecución

### En un solo nodo (4 procesos):

```bash
mpirun -np 4 ./mpi_life -c 40 -f 40 -g 10
```

## ⚙️ Argumentos

- `-c` → columnas del tablero (default: 40)
- `-f` → filas del tablero (default: 40) → debe ser múltiplo de procesos
- `-g` → generaciones a simular (default: 10)

---

## 💡 Ejemplo completo de sincronización:

```bash
./distribute_mpi_life.sh
mpirun -np 4 -hostfile ../../hostfile ./mpi_life -c 40 -f 40 -g 10
```

---

## 📁 Archivos incluidos

- `mpi_life.cpp` → código principal MPI
- `script_conway.sh` → compila y ejecuta localmente
- `distribute_mpi_life.sh` → distribuye y compila en el clúster
- `CMakeLists.txt` → soporte para CMake
