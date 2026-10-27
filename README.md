<div align="center">

  <img src="src/assets/icon.png" alt="CircuitGuard Logo" width="140" height="140" style="border-radius: 20px;" />

  # 🛡️ CircuitGuard Pro
  **Industrial PCB Layout Viewer & AI-Powered Diagnostic Suite**

  [![Release](https://img.shields.io/badge/Release-v1.0.0-00e5ff?style=for-the-badge&logo=github)](https://github.com/Draxx143/project_pcb_problem_finder/releases)
  [![Platform](https://img.shields.io/badge/Platform-Linux%20(Ubuntu%2FDebian)-fcc624?style=for-the-badge&logo=linux&logoColor=black)](https://ubuntu.com/)
  [![AI Engine](https://img.shields.io/badge/Local%20AI-Qwen2.5--Coder%207B-7c4dff?style=for-the-badge&logo=ollama)](https://ollama.com/)
  [![Build](https://img.shields.io/badge/Build-CMake%20%2F%20CPack-064F8C?style=for-the-badge&logo=cmake)](https://cmake.org/)
  [![License](https://img.shields.io/badge/License-MIT-2ea44f?style=for-the-badge)](LICENSE)

  <p align="center">
    <b>CircuitGuard</b> is a next-generation industrial EDA inspection tool designed for hardware engineers. Combining an Altium-inspired dark industrial interface with 100% offline local AI diagnostics, it detects design rule violations, trace anomalies, and layout bottlenecks before manufacturing.
  </p>

  [📥 Download .deb Installer](https://github.com/Draxx143/project_pcb_problem_finder/releases/latest) • [🚀 Quick Start](#-installation) • [🧠 AI Diagnostics](#-ai-powered-diagnostics) • [🇮🇷 مستندات فارسی](#-معرفی-پروژه-فارسی)

</div>

---

## 📸 Interface Preview (نمای محیط صنعتی برنامه)

<div align="center">
  <img src="src/assets/screenshot.png" alt="CircuitGuard Industrial Interface" width="85%" />
</div>

---

## ⚡ Key Industrial Features

| Feature | Description |
| :--- | :--- |
| **🎛️ Industrial EDA Interface** | Ergonomic dark-themed workspace inspired by **Altium Designer**, built for long hardware debugging sessions with zero eye strain. |
| **🧠 100% Local AI Engine** | Powered by **Ollama (qwen2.5-coder:7b)** running entirely on your local hardware—ensuring complete IP security for confidential PCB designs. |
| **🔍 Automated Fault Detection** | Identifies short circuits, clearance violations, unrouted nets, thermal bottlenecks, and signal integrity risks. |
| **⚡ High-Performance Core** | Built with CMake and optimized native libraries for lag-free zooming, panning, and multi-layer inspection. |
| **📦 Native Linux Packaging** | Ships as a standard .deb package with full GNOME/Ubuntu desktop integration and quick-launch diagnostic actions. |

---

## 🧠 AI-Powered Diagnostics

Unlike cloud-based tools that upload your proprietary hardware schematics to external servers, **CircuitGuard** processes everything locally:

1. **Netlist & Trace Analysis:** Evaluates copper pour clearances, trace widths, and via placements against industrial IPC standards.
2. **Interactive Hardware Assistant:** Ask the integrated qwen2.5-coder:7b engine to review specific board regions, suggest decoupling capacitor placements, or explain DRC errors in real time.
3. **Zero Latency & Offline Ready:** Works inside isolated laboratory networks without an internet connection.

---

## 🚀 Installation

### Option 1: Official .deb Package (Recommended for Ubuntu/Debian)
Download the latest installer from the [Releases](https://github.com/Draxx143/project_pcb_problem_finder/releases) page and install it via apt:

~~~bash
sudo apt install ./circuit-guard-1.0.0-Linux.deb
~~~

Once installed, launch **CircuitGuard** directly from your Ubuntu Application Menu!

### Option 2: Build from Source

~~~bash
git clone https://github.com/Draxx143/project_pcb_problem_finder.git
cd project_pcb_problem_finder
./setup_linux.sh
./run.sh
~~~

---

<div dir="rtl">

## 🇮🇷 معرفی پروژه (فارسی)

**CircuitGuard Pro** یک نرم‌افزار صنعتی و پیشرفته برای مشاهده، تحلیل و عیب‌یابی بردهای مدار چاپی (**PCB**) است. این نرم‌افزار با الهام از محیط کاربری حرفه‌ای **Altium Designer** طراحی شده و با بهره‌گیری از مدل هوش مصنوعی محلی **qwen2.5-coder:7b**، به مهندسان الکترونیک کمک می‌کند تا خطاهای طراحی، اتصالات کوتاه و مشکلات مسیرکشی (Routing) را پیش از تولید برد به صورت هوشمند شناسایی کنند.

### ✨ ویژگی‌های برجسته:
* **رابط کاربری صنعتی و مهندسی:** طراحی تاریک، مدرن و استاندارد برای کار طولانی‌مدت مهندسان سخت‌افزار.
* **هوش مصنوعی ۱۰۰٪ آفلاین و امن:** اجرای محلی مدل روی سیستم شما بدون نیاز به اینترنت و با حفظ کامل محرمانگی نقشه‌های صنعتی.
* **نصب آسان در اوبونتو:** ارائه شده به صورت پکیج استاندارد .deb و ادغام کامل با منوی برنامه‌های لینوکس.

</div>

---

## 📄 License

Distributed under the **MIT License**. See [LICENSE](LICENSE) for more information.

<div align="center">
  <sub>Built with ⚡ by the <b>CircuitGuard Engineering Team</b></sub>
</div>