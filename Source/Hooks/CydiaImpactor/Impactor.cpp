#pragma comment(linker, "/ENTRY:DllMain")
#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text /MERGE:.text1=.text /SECTION:.idata,ERW")
#pragma comment(linker, "/SECTION:.Asuna,ERW /MERGE:.text=.Asuna")

#pragma comment(linker, "/EXPORT:CoCreateGuid=ole32.CoCreateGuid")
#pragma comment(linker, "/EXPORT:CoTaskMemFree=ole32.CoTaskMemFree")

#include "ml.cpp"
#include "usbmuxd.h"

ML_OVERLOAD_NEW

PIMPACTOR_CONFIG gConfig;

int (CDECL *stub_usbmuxd_subscribe)(USBMUXD_EVENT_CALLBACK Callback, PVOID UserData);

PIMPACTOR_CONFIG LoadConfig()
{
    if (gConfig == nullptr)
        gConfig = new IMPACTOR_CONFIG;

    NtFileMemory f;

    f.Open(L"config.txt");

    const auto& lines = String::Decode(f.GetBuffer(), f.GetSize32(), CP_UTF16_LE).SplitLines();

    if (lines.GetSize() != 3)
    {
        ExceptionBox(L"invalid config.txt");
        return nullptr;
    }

    gConfig->IPA        = lines[0];
    gConfig->UserName   = lines[1];
    gConfig->Password   = lines[2];

    return gConfig;
}

ULONG NTAPI Thread(PVOID Context)
{
    HWND impactor;

    impactor = FindWindowExW(NULL, NULL, L"wxWindowNR", L"Cydia Impactor");

    while (IsWindowVisible(impactor) == FALSE)
        Ps::Sleep(1000);

    Ps::Sleep(1000);
    SendMessageW(impactor, WM_COMMAND, 0x6F, 0);

    return 0;
}

VOID CDECL USBMuxdCallback(PUSBMUXD_EVENT Event, PVOID UserData)
{
    PUSBMUXD_CALLBACK_CONTEXT Context;

    Context = (PUSBMUXD_CALLBACK_CONTEXT)UserData;

    Context->Callbak(Event, Context->Context);

    if (Event->Event == UE_DEVICE_ADD)
    {
        LoadConfig();
        Ps::CreateThread(Thread);
    }
}

int CDECL usbmuxd_subscribe(USBMUXD_EVENT_CALLBACK Callback, PVOID UserData)
{
    PUSBMUXD_CALLBACK_CONTEXT Context;

    Context = new USBMUXD_CALLBACK_CONTEXT;
    Context->Callbak = Callback;
    Context->Context = UserData;

    return stub_usbmuxd_subscribe(USBMuxdCallback, Context);
}

BOOL NTAPI ImpactorGetOpenFileNameW(LPOPENFILENAMEW name)
{
    PWSTR filename;

    RtlGetFullPathName_U(gConfig->IPA, name->nMaxFile * sizeof(name->lpstrFile[0]), name->lpstrFile, &filename);

    StrCopyW(name->lpstrFileTitle, filename);

    name->nFileOffset = filename - name->lpstrFile;
    name->nFileExtension = findextw(filename) - name->lpstrFile;
    name->nFileExtension += name->lpstrFile[name->nFileExtension] == '.';

    return TRUE;
}

BOOL UnInitialize(PVOID BaseAddress)
{
    return FALSE;
}

BOOL Initialize(PVOID BaseAddress)
{
    using namespace Mp;

    LdrDisableThreadCalloutsForDll(BaseAddress);
    ml::MlInitialize();
    Rtl::SetExeDirectoryAsCurrent();

    PVOID ImpactorExe, ImpactorDll;

    ImpactorExe = GetModuleBase(nullptr);
    ImpactorDll = Ldr::LoadDll(L"Impactor.dll");

    PATCH_MEMORY_DATA p[] =
    {
        MemoryPatchVa((ULONG64)ImpactorGetOpenFileNameW, sizeof(PVOID), IATLookupRoutineByHash(ImpactorExe, COMDLG32_GetOpenFileNameW)),
        FunctionJumpVa(Ldr::GetRoutineAddress(ImpactorDll, "usbmuxd_subscribe"), usbmuxd_subscribe, &stub_usbmuxd_subscribe),
    };

    PatchMemory(p, countof(p), nullptr);

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
