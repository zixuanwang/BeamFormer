#pragma once

#include "..\AudioCaptureNative\AudioDeviceNative.h"

namespace AudioCaptureInterop
{
    public enum class AudioDeviceTypeInterop: int
    {
        CaptureDevice,
        RenderDevice
    };

    public ref struct AudioDeviceInfoInterop
    {
    public:
        System::String^ DeviceID;
        System::String^ DeviceName;
        System::String^ DeviceEnumerator;
    };

    public ref class AudioDeviceInterop
    {
    public:
        AudioDeviceInterop(AudioDeviceTypeInterop type);

        ~AudioDeviceInterop();
        !AudioDeviceInterop();

        property int NumDevices { virtual int get(); }
        AudioDeviceInfoInterop^ GetDeviceInfo(int index);

    private:
        AudioCaptureNative::AudioDeviceNative* mAudioDevice;
    };
}
