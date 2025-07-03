# 🖧 Raspberry Pi Cluster – High Performance on a Budget

A distributed cluster using Raspberry Pi 4 and MPI to explore parallel computing, load balancing, and other MPI features.

---

## 📝 Description

This project builds a **fully functional cluster using Raspberry Pi 4**, where:

- Each Pi acts as a compute node.
- **MPI (Message Passing Interface)** is used for parallel task execution.
- C/C++ programs can be distributed and executed across nodes.
- The system can easily scale with additional nodes.
- File distribution and remote execution are automated.

---

## 🚀 Technologies Used

- **Language:** C++ with MPI  
- **Hardware:** 4× Raspberry Pi 4  
- **Operating System:** Raspberry Pi OS Lite (headless)  
- **Network:** Ethernet + switch  
- **Automation:** SSH

---

## 🛠️ Installation and Configuration

### 🔧 Prerequisites

- 4 Raspberry Pi 4 devices with Raspberry Pi OS x1 and Raspberry Pi OS Lite x3 installed  
- SSH connectivity enabled between all nodes  
- All nodes connected to the same network
- Master node must have root SSH access to all others

## 🛠️ Installation and Configuration (via `commands/`)

Inside the repository, navigate to the `commands/` folder and run the full setup:

```bash
cd commands
chmod +x 0*.sh 04_all_in_one.sh
./04_all_in_one.sh
```

This will:

1. Install Git (`00_git_setup.sh`)
2. Install C++ build tools (`01_cpp_dev_setup.sh`)
3. Install OpenMPI (`02_mpi_install.sh`)
4. Install CMake (`03_cmake_install.sh`)

> You can also run each script individually if needed.

---

### 🧰 Network and SSH Configuration

### 🖥️ MPI Installation

### 🔗 Useful Links

## 👥 Authors

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/LBrownI">
        <img src="https://github.com/LBrownI.png" width="100px;" alt="LBrownI"/><br />
        <sub><b>@LBrownI</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/DalexQ">
        <img src="https://github.com/DalexQ.png" width="100px;" alt="DalexQ"/><br />
        <sub><b>@DalexQ</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/vicentexte">
        <img src="https://github.com/vicentexte.png" width="100px;" alt="vicentexte"/><br />
        <sub><b>@vicentexte</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/Nach0t">
        <img src="https://github.com/Nach0t.png" width="100px;" alt="Nach0t"/><br />
        <sub><b>@Nach0t</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/AlanSilvaaa">
        <img src="https://github.com/AlanSilvaaa.png" width="100px;" alt="AlanSilvaaa"/><br />
        <sub><b>@AlanSilvaaa</b></sub>
      </a>
    </td>
  </tr>
  <tr>
    <td align="center">
      <a href="https://github.com/Vinbu">
        <img src="https://github.com/Vinbu.png" width="100px;" alt="Vinbu"/><br />
        <sub><b>@Vinbu</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/Claudio153">
        <img src="https://github.com/Claudio153.png" width="100px;" alt="Claudio153"/><br />
        <sub><b>@Claudio153</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/ninaaaa3">
        <img src="https://github.com/ninaaaa3.png" width="100px;" alt="ninaaaa3"/><br />
        <sub><b>@ninaaaa3</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/drijksb">
        <img src="https://github.com/drijksb.png" width="100px;" alt="drijksb"/><br />
        <sub><b>@drijksb</b></sub>
      </a>
    </td>
  </tr>
</table>
