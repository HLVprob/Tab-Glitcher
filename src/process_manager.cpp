#include "process_manager.h"
#include <tlhelp32.h>
#include <iostream>

ProcessManager::ProcessManager()
    : process_handle_(nullptr)
    , process_id_(0)
    , process_name_(L"RobloxPlayerBeta.exe") {
}

ProcessManager::~ProcessManager() {
    CloseProcess();
}

bool ProcessManager::FindRobloxProcess() {
    // Close any existing process if we have one
    CloseProcess();

    // Take a snapshot of all running processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    // Grab the first one
    if (!Process32FirstW(snapshot, &pe32)) {
        CloseHandle(snapshot);
        return false;
    }

    // Loop through 'em all
    do {
        if (_wcsicmp(pe32.szExeFile, process_name_.c_str()) == 0) {
            process_id_ = pe32.th32ProcessID;
            
            // Open it up with the powers we need
            process_handle_ = OpenProcess(
                PROCESS_SUSPEND_RESUME | PROCESS_QUERY_INFORMATION,
                FALSE,
                process_id_
            );

            if (process_handle_) {
                // Find all the threads so we can freeze 'em later
                if (EnumerateThreads()) {
                    CloseHandle(snapshot);
                    return true;
                } else {
                    CloseHandle(process_handle_);
                    process_handle_ = nullptr;
                }
            }
        }
    } while (Process32NextW(snapshot, &pe32));

    CloseHandle(snapshot);
    return false;
}

bool ProcessManager::EnumerateThreads() {
    thread_handles_.clear();

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(snapshot, &te32)) {
        CloseHandle(snapshot);
        return false;
    }

    do {
        if (te32.th32OwnerProcessID == process_id_) {
            HANDLE thread_handle = OpenThread(
                THREAD_SUSPEND_RESUME,
                FALSE,
                te32.th32ThreadID
            );

            if (thread_handle) {
                thread_handles_.push_back(thread_handle);
            }
        }
    } while (Thread32Next(snapshot, &te32));

    CloseHandle(snapshot);
    return !thread_handles_.empty();
}

bool ProcessManager::SuspendProcess() {
    if (!process_handle_ || thread_handles_.empty()) {
        return false;
    }

    for (HANDLE thread : thread_handles_) {
        SuspendThread(thread);
    }

    return true;
}

bool ProcessManager::ResumeProcess() {
    if (!process_handle_ || thread_handles_.empty()) {
        return false;
    }

    for (HANDLE thread : thread_handles_) {
        ResumeThread(thread);
    }

    return true;
}

void ProcessManager::CloseProcess() {
    CloseThreads();
    
    if (process_handle_) {
        CloseHandle(process_handle_);
        process_handle_ = nullptr;
    }
    
    process_id_ = 0;
}

void ProcessManager::CloseThreads() {
    for (HANDLE thread : thread_handles_) {
        CloseHandle(thread);
    }
    thread_handles_.clear();
}
