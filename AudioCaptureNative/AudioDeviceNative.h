#pragma once

#include "AudioCaptureCommonNative.h"

namespace AudioCaptureNative
{
    #define MAX_STR_LEN 512
    typedef struct
    {
        wchar_t szDeviceName[MAX_STR_LEN];
        wchar_t szDeviceID[MAX_STR_LEN];
        wchar_t szDeviceEnumerator[MAX_STR_LEN];
    } AUDIO_DEVICE_INFO;

    class AudioDeviceNative
    {
    public:
        AudioDeviceNative(EDataFlow flow);
        ~AudioDeviceNative();
        int NumDevices() const;
        AUDIO_DEVICE_INFO GetDeviceInfo(int index) const;

    private:
        int mNumDevices;
        bool mIsInitialized;
        AUDIO_DEVICE_INFO* mDeviceInfo;
    };
}