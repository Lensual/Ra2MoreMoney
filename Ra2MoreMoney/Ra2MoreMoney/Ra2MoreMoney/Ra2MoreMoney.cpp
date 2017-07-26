// Ra2MoreMoney.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdlib.h>
#include <windows.h>
#include <string>
#include <sstream>
#include <iostream>
#include "tlhelp32.h"
#include <string.h>
#include "Psapi.h"
#pragma comment(lib, "Psapi.lib ")
#include "tchar.h"



using  std::string;
using namespace std;






//const char* long2char(long l)
//{
//	stringstream s;
//	string r;
//	s << l;
//	s >> r;
//	return r.c_str();
//}

DWORD GetPidByName(LPCWSTR n)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 p;
	p.dwSize = sizeof(PROCESSENTRY32);
	bool b = Process32First(hSnapshot, &p);
	while (b)
	{
		if (lstrcmp(n, p.szExeFile) == 0)
		{
			CloseHandle(hSnapshot);
			return p.th32ProcessID;
		}
		else
		{
			b = Process32Next(hSnapshot, &p);
		}
	}
	CloseHandle(hSnapshot);
	return 0;
}
HMODULE GetProcessAddress(HANDLE hProcess)
{
	HMODULE hModules[1];
	DWORD dwCbNeeded;	//返回需要的数组的大小（字节为单位）
	EnumProcessModules(hProcess, hModules, sizeof(hModules), &dwCbNeeded);
	return hModules[0];
}
int ReadMem(HANDLE hProcess, LPCVOID IpBaseAddress, LPVOID rbuffer )
{
	SIZE_T bytesRead;
	ReadProcessMemory(hProcess, IpBaseAddress, (LPVOID)rbuffer, sizeof(rbuffer), &bytesRead);
	return 0;
}
int WriteMem(HANDLE hProcess, LPVOID IpBaseAddress, LPVOID rbuffer)
{
	SIZE_T bytesWritten;
	WriteProcessMemory(hProcess, IpBaseAddress, (LPVOID)rbuffer, sizeof(rbuffer), &bytesWritten);
	return 0;
}
//int转byte
void  intToByte(int i, byte *bytes, int size = 4)
{
	//byte[] bytes = new byte[4];
	memset(bytes, 0, sizeof(byte) *  size);
	bytes[0] = (byte)(0xff & i);
	bytes[1] = (byte)((0xff00 & i) >> 8);
	bytes[2] = (byte)((0xff0000 & i) >> 16);
	bytes[3] = (byte)((0xff000000 & i) >> 24);
	return;
}

int main()
{
	//格式化字符
	char* name = "Game.exe";
	WCHAR wname[50];
	memset(wname, 0, sizeof(wname));
	MultiByteToWideChar(CP_ACP, 0, name, strlen(name) + 1, wname,
		sizeof(wname) / sizeof(wname[0]));
	printf("Process:%S\n", wname);

r:

	//获取PID
	DWORD pid = GetPidByName(wname);
	printf("PID:%d\n", pid);
	if (!pid)
	{
		system("pause");
		return 0;
	}

	//打开进程
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, false, pid);
	printf("hProcess:%d\n", hProcess);
	if (!hProcess)
	{
		system("pause");
		return 0;
	}

	//获取进程内存地址
	HMODULE hModules = GetProcessAddress(hProcess);
	printf("ProcessAddress:%08X\n", hModules);

	//读4B内存 金钱基址 game.exe+0x61E0C4
	printf("Read4ByteFromAddress:%08X\n", (int)hModules + 0x61E0C4);
	BYTE rbuffer[4];
	ReadMem(hProcess, (LPCVOID)((int)hModules + 0x61E0C4), (LPVOID)rbuffer);
	printf("4ByteFromBuffer:%02X %02X %02X %02X\n", rbuffer[0], rbuffer[1], rbuffer[2], rbuffer[3]);
	printf("Count:%d\n", rbuffer[3] * 0x1000000 + rbuffer[2] * 0x10000 + rbuffer[1] * 0x100 + rbuffer[0]);

	//读4B内存 金钱 金钱基址+0x24C
	int MoneyAddr = rbuffer[3] * 0x1000000 + rbuffer[2] * 0x10000 + rbuffer[1] * 0x100 + rbuffer[0] + 0x24C;
	printf("Read4ByteFromAddress:%08X\n", MoneyAddr);
	BYTE rbuffer2[4];
	ReadMem(hProcess, (LPCVOID)(MoneyAddr), (LPVOID)rbuffer2);
	printf("4ByteFromBuffer:%02X %02X %02X %02X\n", rbuffer2[0], rbuffer2[1], rbuffer2[2], rbuffer2[3]);
	printf("Count:%d\n", rbuffer2[3] * 0x1000000 + rbuffer2[2] * 0x10000 + rbuffer2[1] * 0x100 + rbuffer2[0]);


	//写4B内存
	printf("Write4ByteToAddress:%08X\n", MoneyAddr);
	BYTE wbuffer[4];
	intToByte(10000, wbuffer, 4);
	WriteMem(hProcess, (LPVOID)(MoneyAddr), (LPVOID)wbuffer);

	CloseHandle(hProcess);
	Sleep(1000);
	goto r;

    return 0;
}






