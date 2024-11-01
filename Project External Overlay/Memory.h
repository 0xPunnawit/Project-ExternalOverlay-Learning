#pragma once
#include <vector>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>

class Memory
{
public:
	Memory(const wchar_t* processName, const wchar_t* moduleName);
	~Memory();

	uintptr_t processID;
	uintptr_t moduleBaseAddress;
	HANDLE hProcess;

	DWORD GetProcessID(const wchar_t* processName);

	uintptr_t GetModuleBaseAddress(const wchar_t* moduleName);

	uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int>offsets);

	template<typename T>
	T Read(uintptr_t address)
	{
		T buffer{};
		ReadProcessMemory(this->hProcess, (BYTE*)address, &buffer, sizeof(buffer), 0);
		return buffer;
	}

	template<typename T>
	bool Write(uintptr_t address, T data) 
	{
		try
		{
			T buffer = data;
			WriteProcessMemory(this->hProcess, (BYTE*)address, &buffer, sizeof(buffer), 0);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	template<typename T>
	T ReadDMA(uintptr_t address_ptr, std::vector<unsigned int>offsets)
	{
		uintptr_t address = address_ptr;
		for (unsigned int i = 0; i < offsets.size(); ++i)
		{
			address += offsets[i];
			address = this->Read<uintptr_t>(address);
		}

		return address;
	}

	template<typename T>
	void WriteDMA(uintptr_t address_ptr, std::vector<unsigned int>offsets, T data) 
	{
		uintptr_t address = address_ptr;
		for (unsigned int i = 0; i < offsets.size(); ++i)
		{
			address = this->Read<uintptr_t>(address);
			address += offsets[i];
		}

		this->Write(address, data);
	}

	std::string ReadString(uintptr_t address)
	{
		std::vector<char> buffer(16);

		for (unsigned int i = 0; i < 16; i++)
		{
			buffer[i] = this->Read<char>(address + 1 * i);
		}

		return std::string(buffer.data());
	}

	void PatchEx(BYTE* dst, BYTE* src, unsigned int size);

	void NopEx(BYTE* dst, unsigned int size);


};



