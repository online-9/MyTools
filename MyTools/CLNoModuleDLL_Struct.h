#ifndef __MYTOOLS_CLNOMODULEDLL_STRUCT_H__
#define __MYTOOLS_CLNOMODULEDLL_STRUCT_H__
#include "Character.h"


typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _CL_LIST_ENTRY {
	_CL_LIST_ENTRY *Flink;
	_CL_LIST_ENTRY *Blink;
} CL_LIST_ENTRY;

typedef struct _LDR_DATA_TABLE_ENTRY
{
	CL_LIST_ENTRY		InLoadOrderModuleList;
	CL_LIST_ENTRY		InMemoryOrderModuleList;
	CL_LIST_ENTRY		InInitializationOrderModuleList;
	PVOID			DllBase;
	PVOID			EntryPoint;
	ULONG			SizeOfImage;	// in bytes
	UNICODE_STRING	FullDllName;
	UNICODE_STRING	BaseDllName;
	ULONG			Flags;			// LDR_*
	USHORT			LoadCount;
	USHORT			TlsIndex;
	CL_LIST_ENTRY		HashLinks;
	PVOID			SectionPointer;
	ULONG			CheckSum;
	ULONG			TimeDateStamp;
	//    PVOID			LoadedImports;					// seems they are exist only on XP !!!
	//    PVOID			EntryPointActivationContext;	// -same-
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB_LDR_DATA
{
	ULONG		Length;
	BOOLEAN		Initialized;
	PVOID		SsHandle;
	CL_LIST_ENTRY	InLoadOrderModuleList; // ref. to PLDR_DATA_TABLE_ENTRY->InLoadOrderModuleList
	CL_LIST_ENTRY	InMemoryOrderModuleList; // ref. to PLDR_DATA_TABLE_ENTRY->InMemoryOrderModuleList
	CL_LIST_ENTRY	InInitializationOrderModuleList; // ref. to PLDR_DATA_TABLE_ENTRY->InInitializationOrderModuleList
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _CURDIR
{
	UNICODE_STRING	DosPath;
	HANDLE			Handle;
} CURDIR, *PCURDIR;

typedef struct _STRING {
	USHORT Length;
	USHORT MaximumLength;
#ifdef MIDL_PASS
	[size_is(MaximumLength), length_is(Length)]
#endif // MIDL_PASS
	PCHAR Buffer;
} STRING, *PSTRING;

typedef struct _RTL_DRIVE_LETTER_CURDIR
{
	WORD	Flags;
	WORD	Length;
	DWORD	TimeStamp;
	STRING	DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct _PROCESS_PARAMETERS
{
	ULONG					MaximumLength;
	ULONG					Length;
	ULONG					Flags;				// PROCESS_PARAMETERS_NORMALIZED
	ULONG					DebugFlags;
	HANDLE					ConsoleHandle;
	ULONG					ConsoleFlags;
	HANDLE					StandardInput;
	HANDLE					StandardOutput;
	HANDLE					StandardError;
	CURDIR					CurrentDirectory;
	UNICODE_STRING			DllPath;
	UNICODE_STRING			ImagePathName;
	UNICODE_STRING			CommandLine;
	PWSTR					Environment;
	ULONG					StartingX;
	ULONG					StartingY;
	ULONG					CountX;
	ULONG					CountY;
	ULONG					CountCharsX;
	ULONG					CountCharsY;
	ULONG					FillAttribute;
	ULONG					WindowFlags;
	ULONG					ShowWindowFlags;
	UNICODE_STRING			WindowTitle;
	UNICODE_STRING			Desktop;
	UNICODE_STRING			ShellInfo;
	UNICODE_STRING			RuntimeInfo;
	RTL_DRIVE_LETTER_CURDIR	CurrentDirectores[32];
} PROCESS_PARAMETERS, *PPROCESS_PARAMETERS;

typedef VOID NTSYSAPI(*PPEBLOCKROUTINE)(PVOID);

typedef struct _PEB_FREE_BLOCK
{
	struct _PEB_FREE_BLOCK	*Next;
	ULONG					Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

typedef struct _RTL_BITMAP
{
	DWORD	SizeOfBitMap;
	PDWORD	Buffer;
} RTL_BITMAP, *PRTL_BITMAP, **PPRTL_BITMAP;

typedef struct _SYSTEM_STRINGS
{
	UNICODE_STRING	SystemRoot;       // C:\WINNT
	UNICODE_STRING	System32Root;     // C:\WINNT\System32
	UNICODE_STRING	BaseNamedObjects; // \BaseNamedObjects
}SYSTEM_STRINGS, *PSYSTEM_STRINGS;

typedef struct _TEXT_INFO
{
	PVOID			Reserved;
	PSYSTEM_STRINGS	SystemStrings;
}TEXT_INFO, *PTEXT_INFO;

typedef struct _PEB
{
	UCHAR				InheritedAddressSpace;				// 0
	UCHAR				ReadImageFileExecOptions;			// 1
	UCHAR				BeingDebugged;						// 2
	BYTE				b003;								// 3
	PVOID				Mutant;								// 4
	PVOID				ImageBaseAddress;					// 8
	PPEB_LDR_DATA		Ldr;								// C
	PPROCESS_PARAMETERS	ProcessParameters;					// 10
	PVOID				SubSystemData;						// 14  
	PVOID				ProcessHeap;						// 18
	KSPIN_LOCK			FastPebLock;						// 1C
	PPEBLOCKROUTINE		FastPebLockRoutine;					// 20
	PPEBLOCKROUTINE		FastPebUnlockRoutine;				// 24
	ULONG				EnvironmentUpdateCount;				// 28
	PVOID				*KernelCallbackTable;				// 2C
	PVOID				EventLogSection;					// 30
	PVOID				EventLog;							// 34
	PPEB_FREE_BLOCK		FreeList;							// 38
	ULONG				TlsExpansionCounter;				// 3C
	PRTL_BITMAP			TlsBitmap;							// 40
	ULONG				TlsBitmapData[0x2];					// 44
	PVOID				ReadOnlySharedMemoryBase;			// 4C
	PVOID				ReadOnlySharedMemoryHeap;			// 50
	PTEXT_INFO			ReadOnlyStaticServerData;			// 54
	PVOID				InitAnsiCodePageData;				// 58
	PVOID				InitOemCodePageData;				// 5C
	PVOID				InitUnicodeCaseTableData;			// 60
	ULONG				KeNumberProcessors;					// 64
	ULONG				NtGlobalFlag;						// 68
	DWORD				d6C;								// 6C
	LARGE_INTEGER		MmCriticalSectionTimeout;			// 70
	ULONG				MmHeapSegmentReserve;				// 78
	ULONG				MmHeapSegmentCommit;				// 7C
	ULONG				MmHeapDeCommitTotalFreeThreshold;	// 80
	ULONG				MmHeapDeCommitFreeBlockThreshold;	// 84
	ULONG				NumberOfHeaps;						// 88
	ULONG				AvailableHeaps;						// 8C
	PHANDLE				ProcessHeapsListBuffer;				// 90
	PVOID				GdiSharedHandleTable;				// 94
	PVOID				ProcessStarterHelper;				// 98
	PVOID				GdiDCAttributeList;					// 9C
	KSPIN_LOCK			LoaderLock;							// A0
	ULONG				NtMajorVersion;						// A4
	ULONG				NtMinorVersion;						// A8
	USHORT				NtBuildNumber;						// AC
	USHORT				NtCSDVersion;						// AE
	ULONG				PlatformId;							// B0
	ULONG				Subsystem;							// B4
	ULONG				MajorSubsystemVersion;				// B8
	ULONG				MinorSubsystemVersion;				// BC
	KAFFINITY			AffinityMask;						// C0
	ULONG				GdiHandleBuffer[0x22];				// C4
	ULONG				PostProcessInitRoutine;				// 14C
	ULONG				TlsExpansionBitmap;					// 150
	UCHAR				TlsExpansionBitmapBits[0x80];		// 154
	ULONG				SessionId;							// 1D4
	ULARGE_INTEGER		AppCompatFlags;						// 1D8
	PWORD				CSDVersion;							// 1E0
	/*	PVOID				AppCompatInfo;						// 1E4
	UNICODE_STRING		usCSDVersion;
	PVOID				ActivationContextData;
	PVOID				ProcessAssemblyStorageMap;
	PVOID				SystemDefaultActivationContextData;
	PVOID				SystemAssemblyStorageMap;
	ULONG				MinimumStackCommit; */
} PEB, *PPEB;

#endif