#ifndef _IOSAFC_H_14493e0b_32a8_4ec7_9984_f29312865b14_
#define _IOSAFC_H_14493e0b_32a8_4ec7_9984_f29312865b14_

#include "iOSService.h"

using namespace iTunesApi::AFC;
using ml::String;

class iOSAFC
{
public:
    iOSAFC()
    {
        this->Connection = nullptr;
    }

    ~iOSAFC()
    {
        CloseConnection();
    }

    operator AFCConnection()
    {
        return this->Connection;
    }

    NTSTATUS CreateConnection(iOSService &Service, ULONG Timeout = 0)
    {
        this->Connection = AFCConnectionCreate(nullptr, Service, 0, 0, Timeout);
        return this->Connection == nullptr ? kAFCUndefinedError : kAFCSuccess;
    }

    NTSTATUS CloseConnection()
    {
        if (this->Connection != nullptr)
        {
            AFCConnectionClose(*this);
            this->Connection = nullptr;
        }

        return kAFCSuccess;
    }

    NTSTATUS OpenDirectory(const String& Path, AFCDirectory* Directory)
    {
        return AFCDirectoryOpen(*this, Path.Encode(CP_UTF8), Directory);
    }

    NTSTATUS CreateDirectory(const String& Path)
    {
        DebugLog(L"create dir: %s", Path);

        String      dir;
        NTSTATUS    status;

        for (auto& s : Path.Split(L"/"))
        {
            dir += s;
            dir += L"/";

            status = AFCDirectoryCreate(*this, dir.Encode(CP_UTF8));
            if (status != kAFCSuccess)
                return status;
        }

        return kAFCSuccess;
    }

    NTSTATUS ReadDirectory(AFCDirectory Directory, String& Path)
    {
        PCSTR PathName;
        NTSTATUS Status;

        Status = AFCDirectoryRead(*this, Directory, &PathName);

        if (Status == kAFCSuccess && PathName != nullptr)
        {
            Path = String::Decode(PathName, StrLengthA(PathName), CP_UTF8);
        }
        else
        {
            Path = L"";
        }

        return Status;
    }

    NTSTATUS CloseDirectory(AFCDirectory Directory)
    {
        return AFCDirectoryClose(*this, Directory);
    }

    NTSTATUS RemovePath(const String& Path)
    {
        NTSTATUS        Status;
        AFCDirectory    Directory;

        DebugLog(L"remove %s", Path);

        Status = OpenDirectory(Path, &Directory);
        if (Status != kAFCSuccess)
        {
            AFCRemovePath(*this, Path.Encode(CP_UTF8));
            return Status;
        }

        String SubFile;

        while (ReadDirectory(Directory, SubFile) == kAFCSuccess)
        {
            if (!SubFile)
                break;

            if (SubFile == L"." || SubFile == L"..")
                continue;

            RemovePath(Path + L"/" + SubFile);
        }

        CloseDirectory(Directory);
        AFCRemovePath(*this, Path.Encode(CP_UTF8));

        return kAFCSuccess;
    }

    NTSTATUS RenamePath(const String& OldPath, const String& NewPath)
    {
        return AFCRenamePath(*this, OldPath.Encode(CP_UTF8), NewPath.Encode(CP_UTF8));
    }

    NTSTATUS LinkPath(LONG64 LinkType, const String& Target, const String& Link)
    {
        NTSTATUS Status;

        Status = AFCLinkPath(*this, LinkType, Target.Encode(CP_UTF8), Link.Encode(CP_UTF8));

        DebugLog(L"link %p: %s --> %s", Status, Target, Link);

        return Status;
    }

    NTSTATUS OpenFileInfo(const String& Path, PAFCDictionary* Info)
    {
        return AFCFileInfoOpen(*this, Path.Encode(CP_UTF8), Info);
    }

    NTSTATUS ReadKeyValue(PAFCDictionary Info, String& Key, String& Value)
    {
        PCSTR k, v;
        NTSTATUS Status;

        Status = AFCKeyValueRead(Info, &k, &v);
        if (Status != kAFCSuccess)
            return Status;

        Key = String::Decode(k, StrLengthA(k), CP_UTF8);
        Value = String::Decode(v, StrLengthA(v), CP_UTF8);

        return Status;
    }

    NTSTATUS CloseKeyValue(PAFCDictionary Info)
    {
        return AFCKeyValueClose(Info);
    }

    NTSTATUS OpenFile(const String& Path, AFC_FILE_MODE Mode, AFCFileRef* Handle)
    {
        NTSTATUS Status;
        ULONG_PTR Index;

        Index = Path.LastIndexOf(L"/");
        if (Index != Path.kInvalidIndex)
        {
            Status = CreateDirectory(Path.SubString(0, Index));
            if (Status != kAFCSuccess)
                return Status;
        }

        Status = AFCFileRefOpen(*this, Path.Encode(CP_UTF8), Mode, Handle);
        
        DebugLog(L"create file %p: %s", Status, Path);

        return Status;
    }

    NTSTATUS ReadFile(AFCFileRef Handle, PVOID Buffer, ULONG_PTR Length, PULONG_PTR BytesRead = nullptr)
    {
        NTSTATUS Status;
        ULONG_PTR LocalBytesRead;

        Status = AFCFileRefRead(*this, Handle, Buffer, &Length);

        BytesRead = BytesRead == nullptr ? &LocalBytesRead : BytesRead;
        *BytesRead = Status == kAFCSuccess ? Length : 0;

        return Status;
    }

    NTSTATUS WriteFile(AFCFileRef Handle, PVOID Buffer, ULONG_PTR Length)
    {
        return AFCFileRefWrite(*this, Handle, Buffer, Length);
    }

    ULONG64 FileTell(AFCFileRef Handle)
    {
        ULONG64 Offset;

        return AFCFileRefTell(*this, Handle, &Offset) == kAFCSuccess ? Offset : -1ull;
    }

    NTSTATUS SeekFile(AFCFileRef Handle, ULONG_PTR Origin, ULONG64 Offset)
    {
        return AFCFileRefSeek(*this, Handle, Offset, Origin);
    }

    NTSTATUS SetFileSize(AFCFileRef Handle, ULONG64 Size)
    {
        return AFCFileRefSetFileSize(*this, Handle, Size);
    }

    NTSTATUS LockFile(AFCFileRef Handle, BOOL ExclusiveLock)
    {
        return AFCFileRefLock(*this, Handle, ExclusiveLock);
    }

    NTSTATUS UnlockFile(AFCFileRef Handle)
    {
        return AFCFileRefUnlock(*this, Handle);
    }

    NTSTATUS CloseFile(AFCFileRef Handle)
    {
        DebugLog(L"close file: %p", (ULONG_PTR)Handle);
        return AFCFileRefClose(*this, Handle);
    }

    NTSTATUS SendData(PVOID Buffer, ULONG_PTR Length)
    {
        return AFCSendData(*this, Buffer, Length);
    }

    NTSTATUS ReadData(PVOID Buffer, ULONG_PTR Length)
    {
        return AFCReadData(*this, Buffer, Length);
    }

    NTSTATUS ReadPacket(PBYTE* PacketHeader, PBYTE* PacketBody, PULONG PacketSize)
    {
        return AFCReadPacket(*this, PacketHeader, PacketBody, PacketSize);
    }

protected:
    AFCConnection Connection;
};

#endif // _IOSAFC_H_14493e0b_32a8_4ec7_9984_f29312865b14_
