#pragma once

#include "AudioCaptureCommonInterop.h"
#include "AudioWaveFormatInterop.h"
#include "..\AudioCaptureNative\AudioMicCaptureNative.h"

namespace AudioCaptureInterop
{
    public ref class AudioMicCaptureInterop : IAudioCaptureInterop
    {
    public:
        // default capture device will be used with captureDeviceID=nullptr
        AudioMicCaptureInterop(System::String^ captureDeviceID,
                               AudioWaveFormatInterop^ captureWaveFormat,
                               DataReadyCallback^ callbackFunction,
                               int callbackIntervalInMilliSecond);
        ~AudioMicCaptureInterop();
        !AudioMicCaptureInterop();

        virtual void Start();
        virtual void Stop();
        virtual property bool IsCapturing { virtual bool get(); }

    private:
        AudioCaptureNative::AudioMicCaptureNative* mMicCapture;
        AudioWaveFormatInterop^ mCaptureWaveFormat;
        DataReadyCallback^ mCallback;
        System::Threading::Thread^ mCaptureThread;
        int mDataBufferDurationInMilliSecond;

        void ThreadTask();
    };
}
