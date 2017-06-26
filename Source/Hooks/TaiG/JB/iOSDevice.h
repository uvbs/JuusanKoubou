#ifndef _IOSDEVICE_H_a5ab90f6_e409_4429_87f3_992d1da0d339_
#define _IOSDEVICE_H_a5ab90f6_e409_4429_87f3_992d1da0d339_

#include "iOS.h"

using namespace iTunesApi::AMD;
using namespace iTunesApi::CF;
using ml::String;

class iOSDevice
{
public:
    iOSDevice()
    {
        this->Notification = nullptr;
        this->Device = nullptr;
        this->MainThreadId = CurrentTid();
        this->MainThread = TidToHandle(this->MainThreadId);

        this->ConnectCount = 0;
        this->SessionCount = 0;
    }

    ~iOSDevice()
    {
        this->NotificationUnsubscribe();
        NtClose(this->MainThread);
    }

    operator PIOS_DEVICE()
    {
        return this->Device;
    }

    /*++

        notification

    --*/

    NTSTATUS WaitForDeviceConnectionChanged(ULONG Timeout = INFINITE)
    {
        NTSTATUS Status;
        LARGE_INTEGER Interval;

        FormatTimeOut(&Interval, Timeout);
        Status = NtDelayExecution(TRUE, &Interval);

        return Status == STATUS_USER_APC ? STATUS_SUCCESS : STATUS_TIMEOUT;
    }

    static ULONG NTAPI USBListenerThread(HANDLE NotificationHandle)
    {
        PNOTIFICATION_OBJECT Object = (PNOTIFICATION_OBJECT)NotificationHandle;

        CFRetain(Object->RunLoopSource);

        CFRunLoopAddSource(CFRunLoopGetCurrent(), Object->RunLoopSource, *kCFRunLoopCommonModes);
        CFRunLoopRun();

        return 0;
    }

    NTSTATUS NotificationSubscribe(ON_DEVICE_CONNECTION_CHANGED Callback, PVOID Context)
    {
        NTSTATUS Status;

        NotificationUnsubscribe();

        auto cb = [] (PDEVICE_CONNECTION_INFO Info, PVOID UserData) -> VOID
        {
            return ((iOSDevice *)UserData)->DeviceConnectionChanged(Info);
        };

        this->NotificationRoutine = Callback;
        this->NotificationContext = Context;

        Status = AMDeviceNotificationSubscribe(cb, 0, 0, this, &this->Notification);
        if (Status != kAMDSuccess)
            return Status;

#if !USE_ITUNES_MOBILE_DEVICE_DLL

        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), ((PNOTIFICATION_OBJECT)this->Notification)->RunLoopSource, *kCFRunLoopCommonModes);

        Status = Ps::CreateThread(USBListenerThread, this->Notification);
        if (NT_FAILED(Status))
            this->NotificationUnsubscribe();

#endif

        return Status;
    }

    VOID NotificationUnsubscribe()
    {
        if (this->Notification != nullptr)
        {
            AMDeviceNotificationUnsubscribe(this->Notification);
            this->Notification = nullptr;
            this->NotificationRoutine = nullptr;
        }
    }

    VOID DeviceConnectionChanged(PDEVICE_CONNECTION_INFO Info)
    {
        this->Device = Info->Device;

        if (this->NotificationRoutine == nullptr)
            return;

        if (CurrentTid() == this->MainThreadId)
        {
            return this->DeviceConnectionChangedApc(Info);
        }

        auto apc = [] (PVOID self, PVOID info, PVOID Context3)
        {
            PDEVICE_CONNECTION_INFO Info = (PDEVICE_CONNECTION_INFO)info;
            ((iOSDevice *)self)->DeviceConnectionChangedApc(Info);
            delete Info;
        };

        NTSTATUS Status;
        PDEVICE_CONNECTION_INFO Captured = new DEVICE_CONNECTION_INFO(*Info);

        Status = NtQueueApcThread(this->MainThread, apc, this, Captured, nullptr);
        if (NT_FAILED(Status))
            delete Captured;
    }

    VOID DeviceConnectionChangedApc(PDEVICE_CONNECTION_INFO Info)
    {
        this->NotificationRoutine(Info, this->NotificationContext);
    }


    /*++

        device connection

    --*/

    NTSTATUS ConnectDevice()
    {
        BOOL        Connected;
        NTSTATUS    Status;

        if (++this->ConnectCount != 1)
            return kAMDSuccess;

        Connected = FALSE;

        LOOP_ONCE
        {
            Status = AMDeviceConnect(this->Device);
            if (Status != kAMDSuccess)
                break;

            Connected = TRUE;

            AMDeviceIsPaired(this->Device);
            if (Status != kAMDSuccess)
                break;

            Status = AMDeviceValidatePairing(this->Device);
            if (Status != kAMDSuccess)
            {
                CFObjectRef ExtendedPairingErrors = CFSTR("ExtendedPairingErrors");
                CFDictionary dict = CFDictionaryCreate(nullptr, &ExtendedPairingErrors, (CFObjectRef *)kCFBooleanTrue, 1, kCFTypeDictionaryKeyCallBacks, kCFTypeDictionaryValueCallBacks);

                Status = AMDevicePairWithOptions(this->Device, dict);

                if (Status != kAMDSuccess)
                    break;

                Status = AMDeviceValidatePairing(this->Device);
            }

            if (Status != kAMDSuccess)
                break;

            Status = StartSession();
            if (Status != kAMDSuccess)
            {
                break;
            }
        }

        if (Status != kAMDSuccess)
        {
            if (Connected != FALSE)
                AMDeviceDisconnect(this->Device);

            --this->ConnectCount;
            DebugLog(L"ConnectDevice failed: %p", Status);
        }

        return Status;
    }

    NTSTATUS DisconnectDevice()
    {
        if (--this->ConnectCount != 0)
            return kAMDSuccess;

        StopSession();
        AMDeviceDisconnect(this->Device);

        return kAMDSuccess;
    }

    NTSTATUS StartSession()
    {
        if (++this->SessionCount != 1)
        {
            return kAMDSuccess;
        }

        NTSTATUS status = AMDeviceStartSession(*this);
        this->SessionCount = status == kAMDSuccess ? this->SessionCount : 0;
        return status;
    }

    NTSTATUS StopSession()
    {
        if (--this->SessionCount != 0)
        {
            return kAMDSuccess;
        }

        return AMDeviceStopSession(*this);
    }

    /*++

        device info

    --*/

    String GetDeviceValue(PCSTR Key, PCSTR Domain = nullptr)
    {
        iOSDeviceConnector conn(*this);

        if (conn.connected == FALSE)
            return L"";

        CFString Value = AMDeviceCopyValue(this->Device, CFSTR(Domain), CFSTR(Key));
        if (Value == nullptr)
            return L"";

        return Value.ToString();
    }

    NTSTATUS IsJailbroken();

    NTSTATUS IsActivated()
    {
        iOSDeviceConnector conn(*this);

        if (conn.connected == FALSE)
            return conn.status;

        return GetDeviceValue("ActivationState") == L"Activated" ? kAMDSuccess : kAMDMissingActivationRecordError;
    }

    BOOL IsPasswordProtected()
    {
        return FALSE;
    }

    NTSTATUS Restart();

    String GetCPUArchitecture()
    {
        return GetDeviceValue("CPUArchitecture");
    }

    String GetProductVersion()
    {
        return GetDeviceValue("ProductVersion");
    }

protected:
    ULONG_PTR                       MainThreadId;
    HANDLE                          MainThread;

    PNOTIFICATION_OBJECT            Notification;
    PIOS_DEVICE                     Device;

    ON_DEVICE_CONNECTION_CHANGED    NotificationRoutine;
    PVOID                           NotificationContext;

    ULONG_PTR                       ConnectCount;
    ULONG_PTR                       SessionCount;
};

#endif // _IOSDEVICE_H_a5ab90f6_e409_4429_87f3_992d1da0d339_
