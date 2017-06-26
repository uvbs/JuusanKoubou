#pragma comment(linker, "/ENTRY:DllMain")
#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Asuna,ERW /MERGE:.text=.Asuna")

//#define _WINVE

#include "MyLibrary.cpp"

#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeW=VERSION.GetFileVersionInfoSizeW")
#pragma comment(linker, "/EXPORT:VerQueryValueW=VERSION.VerQueryValueW")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoW=VERSION.GetFileVersionInfoW")


#define STUDENT_VERSION 1


#if STUDENT_VERSION

    #define RETURN_ADDRESS          (PVOID)0x65FAEA
    #define COMPARE_ACTIVATE_CODE   0x0DBA2C
    #define REG_KEY_PATH            L"Software\\pjjy\\PureEnglish\\Student_Homework"
    #define OPEN_WEBSITE            0x9EEFC

#else

    #define RETURN_ADDRESS          (PVOID)0x59F6E1
    #define COMPARE_ACTIVATE_CODE   0xB49A0
    #define REG_KEY_PATH            L"Software\\pjjy\\PureEnglish\\Teacher_Homework"
    #define OPEN_WEBSITE            0x8701A

#endif


int CDECL CompareActivateCode(PCWSTR s1, PCWSTR s2)
{
    if (_ReturnAddress() != RETURN_ADDRESS)
        return wcscmp(s1, s2);

    static BOOL NotFirst = FALSE;

    if (NotFirst)
        return wcscmp(s1, s2);

    NotFirst = TRUE;
    return 0;
}

HINSTANCE NTAPI ShellExecuteWebSite(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd)
{
    return ShellExecuteW(hwnd, lpOperation, L"http://www.hxkgw.cn/", lpParameters, lpDirectory, nShowCmd);
}

BOOL UnInitialize(PVOID BaseAddress)
{
    return FALSE;
}

BOOL Initialize(PVOID BaseAddress)
{
    BOOL AutoUpdate = FALSE;

    LdrDisableThreadCalloutsForDll(BaseAddress);

    MEMORY_PATCH p[] =
    {
        PATCH_MEMORY(CompareActivateCode, 4, COMPARE_ACTIVATE_CODE),
    };

    MEMORY_FUNCTION_PATCH f[] =
    {
        INLINE_HOOK_CALL_RVA_NULL(OPEN_WEBSITE, ShellExecuteWebSite),
    };

    typedef struct
    {
        HKEY    Root;
        PWSTR   SubKey;
        PWSTR   ValueName;
        ULONG   ValueType;
        PVOID   ValueData;
        ULONG   ValueSize;

    } REG_DATA_ENTRY, *PREG_DATA_ENTRY;

    REG_DATA_ENTRY Values[] =
    {
        {
            HKEY_CURRENT_USER,
            REG_KEY_PATH,
            L"cfg.reg.sActiveCode",
            REG_SZ,
            L"123456789",
            sizeof(L"123456789"),
        },

        {
            HKEY_CURRENT_USER,
            REG_KEY_PATH,
            L"cfg.reg.sSerialNumber",
            REG_SZ,
            L"1111111111111111",
            sizeof(L"1111111111111111"),
        },

        {
            HKEY_CURRENT_USER,
            REG_KEY_PATH,
            L"cfg.bAutoUpdate",
            REG_DWORD,
            &AutoUpdate,
            4
        },
    };

    PREG_DATA_ENTRY Value;

    FOR_EACH(Value, Values, countof(Values))
    {
        Reg::SetKeyValue(
            Value->Root,
            Value->SubKey,
            Value->ValueName,
            Value->ValueType,
            Value->ValueData,
            Value->ValueSize
        );
    }

    Nt_PatchMemory(p, countof(p), f, countof(f), GetExeModuleHandle());

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
