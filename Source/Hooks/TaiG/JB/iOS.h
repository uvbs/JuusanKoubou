#ifndef _IOS_H_651f444c_14a4_4602_81a3_03a3237daa65_
#define _IOS_H_651f444c_14a4_4602_81a3_03a3237daa65_

#include "ml.h"
#include "../../../Test_Con/iTunes/iTunes.h"

#define DebugLog(...) (PrintConsole(L"[%S:%d] ", __FUNCTION__, __LINE__), PrintConsole(__VA_ARGS__), PrintConsole(L"\n"))

CFDataRef CFObjectToBinaryData(const CFObjectRef object);

template<class T>
class CFObjectT
{
public:
    CFObjectT(T object = nullptr)
    {
        this->object = object;
    }

    ~CFObjectT()
    {
        if (this->object != nullptr)
        {
            iTunesApi::CF::CFRelease(this->object);
            this->object = nullptr;
        }
    }

    CFObjectT(const CFObjectT& that)
    {
        this->object = that.object;
        CFRetain(*this);
    }

    operator T() const
    {
        return this->object;
    }

    CFObjectT& operator=(T object)
    {
        this->~CFObjectT();
        this->object = object;
        return *this;
    }

    CFObjectT& operator=(const CFObjectT& that)
    {
        this->~CFObjectT();
        this->object = that.object;
        CFRetain(this->object);
        return *this;
    }

    ml::String ToString();

protected:
    T object;
};

inline ml::String CFObjectT<CFStringRef>::ToString()
{
    CFIndex Length;
    PWSTR   Buffer;

    Length = iTunesApi::CF::CFStringGetLength(*this) + 1;
    Length *= sizeof(*Buffer);
    Buffer = (PWSTR)AllocStack(Length);
    iTunesApi::CF::CFStringGetCString(*this, Buffer, Length, kCFStringEncodingUTF16LE);

    Buffer[(Length - 1) / sizeof(*Buffer)] = 0;

    return ml::String(Buffer);
}

typedef CFObjectT<CFObjectRef>              CFObject;
typedef CFObjectT<CFDataRef>                CFData;
typedef CFObjectT<CFMutableDataRef>         CFMutableData;
typedef CFObjectT<CFNumberRef>              CFNumber;
typedef CFObjectT<CFStringRef>              CFString;
typedef CFObjectT<CFServiceRef>             CFService;
typedef CFObjectT<CFArrayRef>               CFArray;
typedef CFObjectT<CFMutableArrayRef>        CFMutableArray;
typedef CFObjectT<CFDictionaryRef>          CFDictionary;
typedef CFObjectT<CFMutableDictionaryRef>   CFMutableDictionary;
typedef CFObjectT<CFPropertyListRef>        CFPropertyList;


class iOSDevice;

class iOSDeviceConnector
{
public:
    iOSDeviceConnector(iOSDevice& device);
    ~iOSDeviceConnector();

    iOSDeviceConnector& operator=(const iOSDeviceConnector&) = delete;

public:
    BOOL connected;
    NTSTATUS status;

protected:
    iOSDevice& device;
};

class CFDict
{
public:
    CFDict();
    ~CFDict();

    CFDict(const CFDict&) = delete;

    operator CFPropertyListRef() const
    {
        return (CFPropertyListRef)(CFDictionaryRef)this->dict;
    }

    VOID SetValue(const ml::String& Key, const ml::String& Value);
    VOID SetValue(const ml::String& Key, CFObjectRef Value);
    VOID SetValue(CFObjectRef Key, CFObjectRef Value);

    CFDataRef ToData() const;

protected:
    CFMutableDictionary dict;
};

class CFList
{
public:
    CFList();
    ~CFList();

    VOID Clear();

    CFList& operator<<(INT32 value);
    CFList& operator<<(INT64 value);
    CFList& operator<<(PCSTR value);
    CFList& operator<<(const ml::String& value);
    CFList& operator<<(LONG value);
    CFList& operator<<(CFObjectRef object);

    CFDataRef ToData() const;

    operator CFObjectRef() const
    {
        return (CFObjectRef)this->array;
    }

    operator CFArrayRef() const
    {
        return (CFArrayRef)(CFMutableArrayRef)this->array;
    }

    operator CFMutableArrayRef() const
    {
        return (CFMutableArrayRef)this->array;
    }

protected:
    VOID AppendValue(CFObjectRef object)
    {
        iTunesApi::CF::CFArrayAppendValue(this->array, object);
    }

protected:
    CFMutableArray array;
};

#endif // _IOS_H_651f444c_14a4_4602_81a3_03a3237daa65_
