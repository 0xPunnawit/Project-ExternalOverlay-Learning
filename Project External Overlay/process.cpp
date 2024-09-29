#include "process.h"

// =========================== หาค่า Process ID ===========================
DWORD GetProcessID(const wchar_t* processName)
{
    DWORD processID = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(processEntry);

        if (Process32First(hSnap, &processEntry))
        {
            do
            {
                if (!_wcsicmp(processEntry.szExeFile, processName))
                {
                    processID = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &processEntry));
        }
    }

    CloseHandle(hSnap);
    return processID;
}

// =========================== ค้นหาที่อยู่ (Base Address) ===========================
uintptr_t GetModuleBaseAddress(DWORD processID, const wchar_t* moduleName)
{
    DWORD moduleBaseAddress = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 moduleEntry;
        moduleEntry.dwSize = sizeof(moduleEntry);
        if (Module32First(hSnap, &moduleEntry))
        {
            do
            {
                if (!_wcsicmp(moduleEntry.szModule, moduleName))
                {
                    moduleBaseAddress = (uintptr_t)moduleEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &moduleEntry));
        }
    }

    CloseHandle(hSnap);
    return moduleBaseAddress;
}

// =========================== Find Dynamic Memory Address ===========================
uintptr_t FindDMAAddy(HANDLE hProcess, uintptr_t ptr, std::vector<unsigned int>offsets)
{
    uintptr_t address = ptr;
    for (unsigned int i = 0; i < offsets.size(); ++i)
    {
        ReadProcessMemory(hProcess, (BYTE*)address, &address, sizeof(address), 0);
        address += offsets[i];
    }
    return address;
}