#pragma comment(linker, "/ENTRY:DllMain")
#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Asuna,ERW /MERGE:.text=.Asuna")
#pragma comment(linker, "/EXPORT:PlaySoundW=WINMM.PlaySoundW")
#pragma comment(lib, "comctl32.lib")

#include "MyLibrary.cpp"
#include "WindowBase.h"
#include "../Common.h"
#include "Cxdec.cpp"
#include "../EncryptKey.h"
#include "resource.h"

#define STUDENT_VERSION 1

ML_OVERLOAD_NEW

using namespace MlWindowClass;

ULONG_PTR DecryptKey;

class InputBoxDialog : public CDialogImpl<InputBoxDialog>
{
public:
    struct
    {
        ULARGE_INTEGER  Prime;
        ULONG32         Crc32;
        BOOL            Valid;

    } SerialNumber;

public:
    InputBoxDialog(INT ID) : CDialogImpl<InputBoxDialog>(ID)
    {
        this->SerialNumber.Valid = FALSE;
    }

    BOOL VerifySerialNumber(PWSTR SerialNumber, ULONG_PTR Length)
    {
        ULONG Crc32;

        if (Length != 24)
            return FALSE;

        this->SerialNumber.Crc32 = Bswap(StringToInt32HexW(&SerialNumber[Length - 8]));
        SerialNumber[Length - 8] = 0;
        this->SerialNumber.Prime.HighPart = Bswap(StringToInt32HexW(&SerialNumber[Length - 16]));
        SerialNumber[Length - 16] = 0;
        this->SerialNumber.Prime.LowPart = Bswap(StringToInt32HexW(&SerialNumber[Length - 24]));

        if (RtlComputeCrc32(INITIAL_CRC32, &this->SerialNumber.Prime, sizeof(this->SerialNumber.Prime)) != this->SerialNumber.Crc32)
            return FALSE;

        if (!IsPrime(this->SerialNumber.Prime.LowPart / 2))
            return FALSE;

        return TRUE;
    }

    VOID OnCommand(HWND hWnd, INT ID, HWND hWndCtl, UINT CodeNotify)
    {
        WCHAR       SerialNumber[0x30];
        ULONG_PTR   Length;

        switch (ID)
        {
            case IDOK:
                SerialNumber[0] = 0;
                hWnd = GetDlgItem(IDC_EDIT1);
                Length = CallWindowProcW((WNDPROC)::GetWindowLongPtrW(hWnd, GWLP_WNDPROC), hWnd, WM_GETTEXT, countof(SerialNumber), (LPARAM)SerialNumber);

                if (!VerifySerialNumber(SerialNumber, Length))
                {
                    MessageBoxTimeoutW(L"序列号不正确");
                    ::SetFocus(hWnd);
                    break;
                }

                SetMsgHandled(FALSE);
                break;

            case IDCANCEL:
                EndDialog(0);
                return;
        }
    }

    BEGIN_MSG_MAP(InputBoxDialog)
        MSG_HANDLER(WM_COMMAND, OnCommand)
    END_MSG_MAP()
};

PVOID ExeResource, DllResource;

BOOL (FASTCALL *StubOnInstallComplete)(PVOID Window);

HGLOBAL NTAPI HookLoadResource(HMODULE hModule, HRSRC hResInfo)
{
    HGLOBAL     res;
    PVOID       Buffer;
    ULONG_PTR   Size;
    PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry;

    ResourceDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)hResInfo;

    res = Ldr::LoadResource(hModule, ResourceDataEntry);

    LOOP_ONCE
    {
        if (ResourceDataEntry != ExeResource && ResourceDataEntry != DllResource)
            break;

        Size = ResourceDataEntry->Size;
        Buffer = AllocateMemoryP(Size);
        if (Buffer == NULL)
            break;

        Mm::ReadMemory(CurrentProcess, res, Buffer, Size);

        Cxdec cxdec;
        CXDEC_OPTION opt;
        PULONG Key;

        opt.SetDefaultOptions();
        opt.EncryptBlock = StaticEncryptKey;

        cxdec.SetOption(&opt);
        cxdec.Initialize();

        DecryptBuffer(cxdec, (PBYTE)Buffer, Size, StaticEncryptKey);

        WriteProtectMemory(CurrentProcess, res, Buffer, Size);

        FreeMemoryP(Buffer);
    }

    return res;
}

BOOL FASTCALL OnInstallComplete(PVOID Window)
{
    BOOL        Success;
    PWSTR       SelfPath;
    PLDR_MODULE Module;

    Module = FindLdrModuleByHandle(nullptr);
    SelfPath = (PWSTR)AllocStack(Module->FullDllName.Length);

    CopyMemory(SelfPath, Module->FullDllName.Buffer, Module->FullDllName.Length);
    *findnamew(SelfPath) = 0;

    EnumDirectoryFiles(nullptr, L"*.*", 0, SelfPath, nullptr,
        EnumFilesM(Buffer, FindData, Context)
        {
#if 1
            if (FLAG_ON(FindData->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
            {
                Io::DeleteDirectory(FindData->cFileName);
            }
            else
            {
                Io::DeleteFile(FindData->cFileName);
            }
#else
            AllocConsole();
            PrintConsoleW(L"delete %s\n", FindData->cFileName);
#endif
            return 0;
        },
        0,
        EDF_SUBDIR | EDF_AFTER
    );

    return StubOnInstallComplete(Window);
}

BOOL UnInitialize(PVOID BaseAddress)
{
    return FALSE;
}

BOOL Initialize(PVOID BaseAddress)
{
    ml::MlInitialize();

    SetExeDirectoryAsCurrent();

    Ps::QueueApcRoutine(
        [] ()
        {
            InputBoxDialog input(IDD_DIALOG_INPUT);

            if (input.DoModal(NULL, (HINSTANCE)&__ImageBase) != IDOK)
                Ps::ExitProcess(0);

#if STUDENT_VERSION

            MEMORY_PATCH p[] =
            {
                PATCH_MEMORY(0x0F0C8, 2, 0x183B9),
                PATCH_MEMORY(0x1C781, 4, 0x183BC),
                PATCH_MEMORY(0x81,    1, 0x1755E),  // bypass del self
            };

            MEMORY_FUNCTION_PATCH f[] =
            {
                INLINE_HOOK_CALL_RVA_NULL(0x1AFFF, HookLoadResource),
                INLINE_HOOK_CALL_RVA(0x1CFF7, OnInstallComplete, StubOnInstallComplete),
            };

#else // teacher ver

            MEMORY_PATCH p[] =
            {
                PATCH_MEMORY(0x0F0C8, 2, 0x17249),  // copy to %temp%
                PATCH_MEMORY(0x18F81, 4, 0x1724C),  // copy to %temp%
                PATCH_MEMORY(0x81,    1, 0x1688E),  // bypass del self
            };

            MEMORY_FUNCTION_PATCH f[] =
            {
                INLINE_HOOK_CALL_RVA_NULL(0x1998A, HookLoadResource),
                INLINE_HOOK_CALL_RVA(0x1B8A8, OnInstallComplete, StubOnInstallComplete),
            };

#endif

            Nt_PatchMemory(p, countof(p), f, countof(f), GetExeModuleHandle());

            InitKey(StaticEncryptKey, countof(StaticEncryptKey), input.SerialNumber.Prime.LowPart ^ input.SerialNumber.Prime.HighPart);

#if STUDENT_VERSION

            ExeResource = Ldr::FindResource(NULL, L"PUREENGLISH.EXE>:{SOFTNAME}", L"FILE");
            DllResource = Ldr::FindResource(NULL, L"PUREENGLISHX.DLL", L"FILE");

#else

            ExeResource = Ldr::FindResource(NULL, L"PUREENGLISH.EXE>:纯正英语教师教学系统", L"FILE");
            DllResource = Ldr::FindResource(NULL, L"PUREENGLISHX.DLL", L"FILE");

#endif

        }
    );

    return TRUE;
}

BOOL WINAPI DllMain(PVOID BaseAddress, ULONG Reason, PVOID Reserved)
{
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
            return Initialize(BaseAddress) || UnInitialize(BaseAddress);

        case DLL_PROCESS_DETACH:
            UnInitialize(BaseAddress);
            break;
    }

    return TRUE;
}
