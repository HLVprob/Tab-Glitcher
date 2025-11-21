#pragma once
#include <windows.h>
#include <vector>
#include <string>

class ProcessManager {
public:
    ProcessManager();
    ~ProcessManager();

    bool FindRobloxProcess();
    bool SuspendProcess();
    bool ResumeProcess();
    void CloseProcess();
    
    bool IsProcessOpen() const { return process_handle_ != nullptr; }
    DWORD GetProcessId() const { return process_id_; }
    std::wstring GetProcessName() const { return process_name_; }

private:
    HANDLE process_handle_;
    DWORD process_id_;
    std::wstring process_name_;
    std::vector<HANDLE> thread_handles_;
    
    bool EnumerateThreads();
    void CloseThreads();
};
