#include "Memory.h"

Memory::Memory(const wchar_t* processName, const wchar_t* moduleName)
{
    this->processID = this->GetProcessID(processName);
    this->moduleBaseAddress = this->GetModuleBaseAddress(moduleName);
    this->hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, this->processID);
}

Memory::~Memory()
{
}

// =========================== �Ҥ�� Process ID ===========================
DWORD Memory::GetProcessID(const wchar_t* processName)
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

// =========================== ���ҷ������ (Base Address) ===========================
uintptr_t Memory::GetModuleBaseAddress(const wchar_t* moduleName)
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
uintptr_t Memory::FindDMAAddy(uintptr_t ptr, std::vector<unsigned int>offsets)
{
    uintptr_t address = ptr;
    for (unsigned int i = 0; i < offsets.size(); ++i)
    {
        ReadProcessMemory(this->hProcess, (BYTE*)address, &address, sizeof(address), 0);
        address += offsets[i];
    }
    return address;
}

void Memory::PatchEx(BYTE* dst, BYTE* src, unsigned int size)
{
    DWORD oldProtect;
    VirtualProtectEx(this->hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    WriteProcessMemory(this->hProcess, dst, src, size, nullptr);
    VirtualProtectEx(this->hProcess, dst, size, oldProtect, &oldProtect);
}

void Memory::NopEx(BYTE* dst, unsigned int size)
{
    BYTE* nopArray = new BYTE[size];
    memset(nopArray, 0x90, size);

    PatchEx(dst, nopArray, size);
    delete[] nopArray;
}