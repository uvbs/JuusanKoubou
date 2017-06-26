#ifndef _USBMUXD_H_549ed101_d7c6_4292_8aae_122b2022e580_
#define _USBMUXD_H_549ed101_d7c6_4292_8aae_122b2022e580_

#include "ml.h"

using ml::String;

#pragma pack(push, 4)

typedef struct
{
	ULONG   Handle;
	ULONG   ProductID;
	CHAR    Udid[41];

} USBMUXD_DEVICE_INFO, *PUSBMUXD_DEVICE_INFO;

enum USBMuxdEventType
{
    UE_DEVICE_ADD = 1,
    UE_DEVICE_REMOVE,
    UE_DEVICE_PAIRED
};

typedef struct
{
    ULONG               Event;
    USBMUXD_DEVICE_INFO Device;

} USBMUXD_EVENT, *PUSBMUXD_EVENT;

#pragma pack(pop)

typedef VOID (CDECL *USBMUXD_EVENT_CALLBACK) (PUSBMUXD_EVENT Event, PVOID UserData);

typedef struct
{
    PVOID Context;
    USBMUXD_EVENT_CALLBACK  Callbak;

} USBMUXD_CALLBACK_CONTEXT, *PUSBMUXD_CALLBACK_CONTEXT;

typedef struct
{
    String IPA;
    String UserName;
    String Password;

} IMPACTOR_CONFIG, *PIMPACTOR_CONFIG;

#endif // _USBMUXD_H_549ed101_d7c6_4292_8aae_122b2022e580_
