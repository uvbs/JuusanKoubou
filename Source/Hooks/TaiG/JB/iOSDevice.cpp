#include "iOSDevice.h"
#include "iOSService.h"

NTSTATUS iOSDevice::IsJailbroken()
{
    // kAMDInvalidServiceError
    return iOSService(*this).Start("com.apple.afc2");
}

NTSTATUS iOSDevice::Restart()
{
    NTSTATUS Status;
    iOSService service(*this);

    DebugLog(L"restart");

    Status = service.Start("com.apple.mobile.diagnostics_relay");
    if (Status != kAMDSuccess)
        return Status;

    CFDict request;

    request.SetValue(L"Request", L"Restart");
    service.SendMessage(request);

    return Status;
}
