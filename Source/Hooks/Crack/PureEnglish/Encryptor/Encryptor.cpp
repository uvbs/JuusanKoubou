#pragma comment(linker, "/ENTRY:main")
#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Asuna,ERW /MERGE:.text=.Asuna")

#include "MyLibrary.cpp"
#include "Cxdec.cpp"
#include "../EncryptKey.h"

ForceInline Void main2(Long_Ptr argc, WChar **argv)
{
    if (argc == 1)
        return;

    Cxdec cxdec;
    CXDEC_OPTION opt;
    PULONG Key;

    opt.SetDefaultOptions();
    opt.EncryptBlock = StaticEncryptKey;

    InitKey(StaticEncryptKey, countof(StaticEncryptKey), KEY_MASK);

    cxdec.SetOption(&opt);
    cxdec.Initialize();

    while (--argc)
    {
        ULONG Index;
        PBYTE Buffer;
        NtFileMemory mem;

        if (NT_FAILED(mem.Open(*++argv)))
            continue;

        EncryptBuffer(cxdec, (PBYTE)mem.GetBuffer(), mem.GetSize32(), StaticEncryptKey);

        NtFileDisk file;

        file.Create(*argv);
        file.Write(mem.GetBuffer(), mem.GetSize32());
    }
}

int __cdecl main(Long_Ptr argc, wchar_t **argv)
{
    getargsW(&argc, &argv);
    main2(argc, argv);
    ReleaseArgv(argv);
    Ps::ExitProcess(0);
}
