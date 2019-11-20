#pragma once

#include <limits>
#include <vector>
#include <tuple>
#include <cmath>
#include <iostream>
#include <string>
#include <memory>
#include <array>
#include <bitset>
#include <set>

#include <winsock2.h>
#include <Windows.h>
#include <iphlpapi.h>

#pragma comment(lib, "IPHLPAPI.lib")

using size = unsigned long long int;
using real = double;
using amount = unsigned int;

#define str std::wstring
#define win_nl(T, y) std::unique_ptr<T> y{ new T() };
#define win_ptr(T, y) win_nl(T, y) y->dwLength = sizeof(T);

class WindowsInfoCollector
{
public:

	static size get_physical_memory()
	{
		win_ptr(MEMORYSTATUSEX, ret);
		GlobalMemoryStatusEx(ret.get());
		return static_cast<size>(ret->ullTotalPhys);
	}

	static real get_physical_memory_usage()
	{
		win_ptr(MEMORYSTATUSEX, ret);
		GlobalMemoryStatusEx(ret.get());
		return static_cast<size>(ret->dwMemoryLoad) / 100.0;
	}

	static amount get_amount_of_cores()
	{
		win_nl(SYSTEM_INFO, ret);
		GetSystemInfo(ret.get());
		return static_cast<amount>(ret->dwNumberOfProcessors);
	}

	static str get_cpu_info()
	{
		std::wstring ret{ L"" };

		std::array<int, 4> cpui;
		__cpuid(cpui.data(), 0x80000000);
		size_t nExIds_ = cpui[0];

		char brand[0x40];
		memset(brand, 0, sizeof(brand));
		std::vector<std::array<int, 4>> extdata_;

		for (int i = 0x80000000; i <= nExIds_; ++i)
		{
			__cpuidex(cpui.data(), i, 0);
			extdata_.push_back(cpui);
		}

		if (nExIds_ >= 0x80000004)
		{
			memcpy(brand, extdata_[2].data(), sizeof(cpui));
			memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
			memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
			for (int i = 0; i < 0x40; i++)
				ret += static_cast<wchar_t>(brand[i]);
		}

		return ret.substr(0, ret.find(L'\0'));
	}

	static size get_hz_per_core()
	{
		win_nl(LARGE_INTEGER, ret);
		QueryPerformanceFrequency(ret.get());
		return static_cast<size>(ret->QuadPart);
	}

	static real cpu_usage()
	{
		real ret{ 0.0 };
		FILETIME prevSysIdle, prevSysKernel, prevSysUser;
		if (GetSystemTimes(&prevSysIdle, &prevSysKernel, &prevSysUser) == 0)
			return 0;
		Sleep(15
		);
		FILETIME sysIdle, sysKernel, sysUser;
		if (GetSystemTimes(&sysIdle, &sysKernel, &sysUser) == 0) 
			return 0;

		if (prevSysIdle.dwLowDateTime != 0 && prevSysIdle.dwHighDateTime != 0)
		{
			ULONGLONG sysIdleDiff, sysKernelDiff, sysUserDiff;
			sysIdleDiff = __substract_time(sysIdle, prevSysIdle);
			sysKernelDiff = __substract_time(sysKernel, prevSysKernel);
			sysUserDiff = __substract_time(sysUser, prevSysUser);

			ULONGLONG sysTotal = sysKernelDiff + sysUserDiff;
			ULONGLONG kernelTotal = sysKernelDiff - sysIdleDiff;

			ret = (double)(((kernelTotal + sysUserDiff) * 100.0) / sysTotal);
		}

		return ret;
	}

	static str os_info()
	{
		DWORD os_maj{ 0 }, os_min{ 0 }, os_sp_maj{ 0 }, os_sp_min{ 0 };
		win_nl(DWORD, ret_code);
		GetProductInfo(os_maj, os_min, os_sp_maj, os_sp_min, ret_code.get());
		std::ifstream list{ "windows_name_translation.dat" };
		std::string line;
		while (std::getline(list, line))
		{
			std::string num{ "" };
			for (int i = 0; i < line.size(); i++)
				if (line[i] != ' ') num += line[i]; else break;
			if (*ret_code == std::stoi(num))
			{
				list.close();
				str ret{ L"[" + std::to_wstring(*ret_code) + L"] " };
				line = line.substr(num.size() + 1);
				for (const char var : line) 
					ret += static_cast<wchar_t>(var);
				return ret;
			}
		}

		return L"Not Found";
	}

	static std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> version()
	{
		#define st(x) static_cast<size_t>(x)
		DWORD os_maj{ 0 }, os_min{ 0 }, os_sp_maj{ 0 }, os_sp_min{ 0 };
		win_nl(DWORD, ret_code);
		GetProductInfo(os_maj, os_min, os_sp_maj, os_sp_min, ret_code.get());
		return {{st(os_maj), st(os_min)}, { st(os_sp_maj), st(os_sp_maj)}};
	}
	
	static size total_disks_space()
	{
		const std::vector<wchar_t> disks = avaiable_disk(true);
		unsigned long long int tot{ 0ull };
		win_nl(ULARGE_INTEGER, _);
		win_nl(ULARGE_INTEGER, tat);
		win_nl(ULARGE_INTEGER, __);
		for (const wchar_t var : disks)
		{
			std::string tmp{ "" };
			tmp += static_cast<unsigned char>(var);
			tmp += ':';
			tmp += '\\';
			GetDiskFreeSpaceExA(tmp.c_str(), _.get(), tat.get(), __.get());
			tot += tat->QuadPart;
		}
		return tot;
	}

	static real total_free_space()
	{
		const std::vector<wchar_t> disks = avaiable_disk(true);
		unsigned long long int tot{ 0ull };
		unsigned long long int tot_free{ 0ull };
		win_nl(ULARGE_INTEGER, _);
		win_nl(ULARGE_INTEGER, tat);
		win_nl(ULARGE_INTEGER, tat_free);
		for (const wchar_t var : disks)
		{
			std::string tmp{ "" };
			tmp += static_cast<unsigned char>(var);
			tmp += ':';
			tmp += '\\';
			GetDiskFreeSpaceExA(tmp.c_str(), _.get(), tat.get(), tat_free.get());
			tot += tat->QuadPart;
			tot_free += tat_free->QuadPart;
		}
		return real(tot_free/1000000.0)/real(tot/1000000.0);
	}

	static size disk_space(const char i_disk_letter)
	{
		unsigned long long int tot{ 0ull };
		win_nl(ULARGE_INTEGER, _);
		win_nl(ULARGE_INTEGER, tat);
		win_nl(ULARGE_INTEGER, __);
		std::string tmp{ "" };
		tmp += i_disk_letter;
		tmp += ':';
		tmp += '\\';
		GetDiskFreeSpaceExA(tmp.c_str(), _.get(), tat.get(), __.get());
		tot += tat->QuadPart;
		return tot;
	}

	static real free_space(const char i_disk_letter)
	{
		unsigned long long int tot{ 0ull };
		unsigned long long int tot_free{ 0ull };
		win_nl(ULARGE_INTEGER, _);
		win_nl(ULARGE_INTEGER, tat);
		win_nl(ULARGE_INTEGER, tat_free);
		std::string tmp{ "" };
		tmp += i_disk_letter;
		tmp += ':';
		tmp += '\\';
		GetDiskFreeSpaceExA(tmp.c_str(), _.get(), tat.get(), tat_free.get());
		tot += tat->QuadPart;
		tot_free += tat_free->QuadPart;
		return real(tot_free / 1000000.0) / real(tot / 1000000.0);
	}

	static std::vector<int> avaiable_disk()
	{
		std::bitset<sizeof(DWORD) * 8> disks{ GetLogicalDrives() };
		std::vector<int> ret;
		for (int i = 0; i < disks.size(); i++)
			if (disks[i]) ret.push_back(i + 65);
		return ret;
	}

	static std::vector<wchar_t> avaiable_disk(bool)
	{
		std::bitset<sizeof(DWORD) * 8> disks{ GetLogicalDrives() };
		std::vector<wchar_t> ret;
		for (int i = 0; i < disks.size(); i++)
			if (disks[i]) ret.push_back(static_cast<wchar_t>(i + 65));
		return ret;
	}

	static std::set<str> avaiable_ips()
	{
		std::set<str> ret;

		PIP_ADAPTER_INFO pAdapterInfo;
		PIP_ADAPTER_INFO pAdapter = NULL;

		ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
		pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
		
		//Call to get buffor size (if failed writes proper values)
		if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
		{
			free(pAdapterInfo);
			pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
		}

		if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) 
		{
			pAdapter = pAdapterInfo;
			while (pAdapter) 
			{
				const auto ip = pAdapter->IpAddressList.IpAddress;
				str ip_concat{ L"" };
				for (int i = 0; i < 16; i++)
					if(ip.String[i] != '\0') ip_concat += static_cast<wchar_t>(ip.String[i]);
				ret.emplace(ip_concat);
				pAdapter = pAdapter->Next;
			}
			if (pAdapterInfo) free(pAdapterInfo);
		}
		return ret;
	}

	using SI = std::pair<real, str>;
	static SI SImplify(const size to_conv) noexcept
	{
		#define d(x) static_cast<real>(x)
		const size multipler = 1024;
		const real multi = 1024.0;
		if(d(to_conv / multipler) < multi)
			return SI{ d(to_conv / multipler), L"k" };
		else if( d(to_conv / std::pow(multipler, 2)) < multi )
			return SI{ d(to_conv / std::pow(multipler, 2)), L"m" };
		else if( d(to_conv / std::pow(multipler, 3)) < multi )
			return SI{ d(to_conv / std::pow(multipler, 3)), L"g" };
		else if( d(to_conv / std::pow(multipler, 4)) < multi )
			return SI{ d(to_conv / std::pow(multipler, 4)), L"t" };
		else if( d(to_conv / std::pow(multipler, 5)) < multi )
			return SI{ d(to_conv / std::pow(multipler, 5)), L"p" };
		else if( d(to_conv / std::pow(multipler, 6)) < multi )
			return SI{ d(to_conv / std::pow(multipler, 6)), L"e" };
		else 
			return SI{ to_conv, L"1" };
	}

private:

	static ULONGLONG __substract_time(const FILETIME one, const FILETIME two)
	{
		LARGE_INTEGER a, b;
		a.LowPart = one.dwLowDateTime;
		a.HighPart = one.dwHighDateTime;

		b.LowPart = two.dwLowDateTime;
		b.HighPart = two.dwHighDateTime;

		return a.QuadPart - b.QuadPart;
	}

};