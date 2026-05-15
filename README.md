# Asynchronous Motor Analyzer ⚡

![C++](https://img.shields.io/badge/Language-C%2B%2B%2FCLI-blue)
![Framework](https://img.shields.io/badge/Framework-.NET_Windows_Forms-purple)
![Status](https://img.shields.io/badge/Status-Completed-success)

**Asynchronous Motor Analyzer** is a comprehensive, interactive desktop application designed for the steady-state performance analysis of three-phase induction motors. Developed as an Electric Machines design project, this software bridges the gap between theoretical electromagnetic models and practical engineering analysis.

By automating the complex, iterative calculations required for the **exact per-phase T-equivalent circuit**, the application eliminates the time-consuming nature of manual analysis and provides a dynamic exploration of machine behavior.

---

## 🔬 Mathematical Model & Core Logic

The simulation engine is built upon the **Exact Per-Phase T-Equivalent Circuit**. It performs a full sweep from standstill ($n = 0$) to synchronous speed ($n = n_s$) with a 1-RPM resolution. 

Key computational steps include:
* **Thevenin Equivalent Simplification:** The stator and magnetizing branches are reduced to a Thevenin equivalent circuit ($V_{th}$, $Z_{th}$) to efficiently calculate electromagnetic torque.
* **Power Flow Chain:** Accurately tracks the active power flow: Input Power ($P_{in}$) ➔ Stator Copper Loss ($P_{cu1}$) ➔ Core Loss ($P_{fe}$) ➔ Air-Gap Power ($P_{gap}$) ➔ Rotor Copper Loss ($P_{cu2}$) ➔ Converted Mechanical Power ($P_{conv}$) ➔ Net Output Power ($P_{out}$).
* **Complex Arithmetic:** Utilizes the standard C++ `<complex>` library to handle AC circuit impedances, phase angles, and power factors losslessly.

---

## ✨ Key Features

### 📊 1. Multi-Panel Interactive Data Visualization
The software features a 5-panel dark-themed chart grid powered by .NET Charting:
* **Torque vs. Speed:** Visualizes starting torque, breakdown (pull-out) torque, and the operational curve.
* **Line Current vs. Speed:** Tracks the inrush current drop-off.
* **Efficiency & Power Factor vs. Speed:** Dual-axis chart for performance monitoring.
* **Torque vs. Slip:** Reversed x-axis representation of motor torque.
* **Efficiency vs. Output Power ($\eta$ vs $P_{out}$):** To identify the most efficient load range.
* *Interaction:* All charts support **mouse-wheel zooming**, **click-and-drag panning**, and **crosshair tooltips** for precise data point reading.

### 🧮 2. Integrated Parameter Calculator (Lab Test Estimator)
Users don't need to know the exact internal resistances to use this tool. The built-in calculator derives the equivalent circuit parameters ($R_1$, $X_1$, $R_2'$, $X_2'$, $X_m$, $R_c$) directly from standard laboratory tests:
* **No-Load Test Inputs:** $V_{NL}$, $I_{NL}$, $P_{NL}$
* **Blocked-Rotor Test Inputs:** $V_{BR}$, $I_{BR}$, $P_{BR}$, and Stator DC Resistance ($R_{1,dc}$)

### ❄️ 3. Freeze & Compare Mode
An essential tool for motor design optimization. Users can "FREEZE" the current simulation curves on the screen as dashed lines, modify a motor parameter (e.g., increase rotor resistance $R_2'$), and run the analysis again. This allows for an instant visual comparison of how parameter changes affect the motor's starting torque and efficiency.

### ⚙️ 4. Mechanical Fan/Pump Load Overlay
Simulates real-world applications by allowing users to input a rated load torque. The software plots a quadratic fan/pump load line ($T_L \propto n^2$) over the torque-speed curve, making it easy to visually identify the exact steady-state operating point of the motor-load system.

### 📄 5. Automated Reporting & Exporting
* **Analysis Report:** Generates an on-screen textual report highlighting critical points (Starting Torque, Breakdown Torque ratio, Speed Regulation, Rated Efficiency, IEC Efficiency Class, Total Losses).
* **CSV Export:** Dumps the entire 1500+ row simulation array (Speed, Slip, Torque, Current, Eta, PF, Pout) into a CSV file for MATLAB or Excel post-processing.
* **Graphical Export:** Save the entire 5-chart dashboard as a high-resolution `PNG` or print directly to `PDF`.
* **Profile Management:** Save (`.mprof`) and load motor configurations for later use.

---

## 🚀 Getting Started

### Prerequisites
* **Visual Studio** (2019 or newer recommended)
* **.NET Framework** (typically v4.7.2 or higher)
* **C++/CLI build tools** installed via Visual Studio Installer.

### Installation & Build
1. Clone the repository:
   ```bash
   git clone [https://github.com/agueler06/Asynchronous-Motor-Analyzer.git](https://github.com/agueler06/Asynchronous-Motor-Analyzer.git)
