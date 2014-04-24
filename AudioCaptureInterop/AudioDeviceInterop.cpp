#include "AudioDeviceInterop.h"

using namespace AudioCaptureInterop;
using namespace AudioCaptureNative;

AudioDeviceInterop::AudioDeviceInterop(AudioDeviceTypeInterop type) :
    mAudioDevice(NULL)
{
    if (type == AudioDeviceTypeInterop::CaptureDevice)
    {
        mAudioDevice = new AudioDeviceNative(eCapture);
    }
    else
    {
        mAudioDevice = new AudioDeviceNative(eRender);
    }
}

AudioDeviceInterop::~AudioDeviceInterop()
{
    this->!AudioDeviceInterop();
}

AudioDeviceInterop::!AudioDeviceInterop()
{
    if (mAudioDevice)
    {
        delete mAudioDevice;
        mAudioDevice = NULL;
    }
}

int AudioDeviceInterop::NumDevices::get()
{
    return mAudioDevice->NumDevices();
}

AudioDeviceInfoInterop^ AudioDeviceInterop::GetDeviceInfo(int index)
{
    AUDIO_DEVICE_INFO info = mAudioDevice->GetDeviceInfo(index);

    AudioDeviceInfoInterop^ deviceInfo = gcnew AudioDeviceInfoInterop();
    deviceInfo->DeviceID = gcnew System::String(info.szDeviceID);
    deviceInfo->DeviceName = gcnew System::String(info.szDeviceName);
    deviceInfo->DeviceEnumerator = gcnew System::String(info.szDeviceEnumerator);
    return deviceInfo;
}
