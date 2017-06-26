#pragma comment(linker, "/ENTRY:main2")
#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Asuna,ERW /MERGE:.text=.Asuna")

#include "MyLibrary.cpp"
#include "../Common.h"

ForceInline Void main2(Long_Ptr argc, WChar **argv)
{
    struct
    {
        ULARGE_INTEGER  Prime;
        ULONG32         Crc32;

    } SerialNumber;

    LOOP_FOREVER
    {
        LOOP_FOREVER
        {
            SerialNumber.Prime.LowPart = GetRandom32Range(100000000, 999999999);
            if (IsPrime(SerialNumber.Prime.LowPart))
                break;

            Ps::Sleep(0);
        }

        SerialNumber.Prime.LowPart *= 2;
        SerialNumber.Prime.HighPart = SerialNumber.Prime.LowPart ^ KEY_MASK;
        SerialNumber.Crc32 = RtlComputeCrc32(INITIAL_CRC32, &SerialNumber.Prime, 8);

        WCHAR buf[0x30];

        for (int i = 0; i != 12; ++i)
            swprintf(&buf[i * 2], L"%02X", ((PBYTE)&SerialNumber)[i]);

        PrintConsoleA("%S\n", buf);
        Ps::Sleep(0);
    }
}

int __cdecl main(Long_Ptr argc, wchar_t **argv)
{
    getargsW(&argc, &argv);
    main2(argc, argv);
    ReleaseArgv(argv);
    Ps::ExitProcess(0);
}