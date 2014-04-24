#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <dmo.h>
#include <atlbase.h>
#include <atlcomcli.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <uuids.h>

#define EXIT_ON_ERROR(hr)           if (FAILED(hr)) { goto Exit; }
#define IF_FAILED_JUMP(hr, label)   if (FAILED(hr)) { goto label; }
#define IF_FAILED_RETURN(hr)        if (FAILED(hr)) { return hr; }
#define SAFE_RELEASE(ptr)           if ((ptr) != NULL) { (ptr)->Release(); (ptr) = NULL; }

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC        10000000
#define REFTIMES_PER_MILLISEC   10000

namespace AudioCaptureNative
{
    const IID IID_IAudioClient = __uuidof(IAudioClient);
    const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);

    class DECLSPEC_UUID("f447b69e-1884-4a7e-8055-346f74d6edb3") CResamplerMediaObject;
    const CLSID CLSID_CResamplerMediaObject = __uuidof(CResamplerMediaObject);
}
