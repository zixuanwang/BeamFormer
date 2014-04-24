#pragma once

#include "AudioWaveFormatInterop.h"
#include "AudioCaptureCommonInterop.h"
#include "..\AudioCaptureNative\AudioLoopbackCaptureNative.h"

namespace AudioCaptureInterop
{
    public ref class AudioLoopbackCaptureInterop : IAudioCaptureInterop
    {
    public:
        AudioLoopbackCaptureInterop(AudioWaveFormatInterop^ captureWaveFormat,
                                    DataReadyCallback^ callbackFunction,
                                    int callbackIntervalInMilliSecond);
        ~AudioLoopbackCaptureInterop();
        !AudioLoopbackCaptureInterop();

        virtual void Start();
        virtual void Stop();
        virtual property bool IsCapturing { virtual bool get(); }

    private:
        AudioCaptureNative::AudioLoopbackCaptureNative* mLoopbackCapture;
        AudioWaveFormatInterop^ mCaptureWaveFormat;
        DataReadyCallback^ mCallback;
        System::Threading::Thread^ mCaptureThread;
        int mDataBufferDurationInMilliSecond;

        void ThreadTask();
    };
}
