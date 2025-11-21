# TabGlitcher

**TabGlitcher** is a modern, lightweight process suspension tool designed for Roblox. It allows you to perform the "tab glitch" mechanic (temporarily freezing the game process) with a single hotkey press, featuring a sleek ImGui-based interface.

![Platform](https://img.shields.io/badge/platform-Windows-0078D6.svg)
![C++](https://img.shields.io/badge/language-C%2B%2B20-f34b7d.svg)

##  Features

*   **Modern UI:** Clean, dark-themed interface built with ImGui.
*   **Custom Hotkeys:** Bind any keyboard key or mouse button (including Mouse4/Mouse5).
*   **Visual Feedback:** Real-time status indicators (Suspended/Resumed) with color coding.
*   **Safe & Focused:** Specifically targets `RobloxPlayerBeta.exe`, leaving other applications untouched.
*   **Lightweight:** Written in C++ for minimal resource usage.

##  Build & Installation

This project uses **CMake**. Ensure you have Visual Studio (with C++ workload) and CMake installed.

1.  Clone the repository:
    ```bash
    git clone [https://github.com/HLVprob/Tab-Glitcher.git](https://github.com/HLVprob/Tab-Glitcher.git)
    cd Tab-Glitcher
    ```

2.  Generate project files and build:
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build . --config Release
    ```

3.  Run `TabGlitcher.exe` from the `Release` directory.

##  Usage

1.  Run **TabGlitcher** as Administrator (Recommended).
2.  Open **Roblox** and join a game.
3.  Click the **"Set Hotkey"** button.
4.  Press your desired key (e.g., `F`, `X`, or `Mouse4`).
5.  Click **"Start"** to begin monitoring.
6.  **Hold** your hotkey to freeze the game (lag switch), **release** to resume.

## ⚠️ Disclaimer

This software is for educational and testing purposes only. Using this tool to gain an unfair advantage in online games may violate their Terms of Service. The user assumes all responsibility for any consequences, including account bans.

---
*Developed with ❤️ by ifrit*
