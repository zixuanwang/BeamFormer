#pragma once

#include "AudioCaptureCommonNative.h"

namespace AudioCaptureNative
{
    class AudioMicCaptureNative
    {
    public:
        // default capture device will be used for captureDeviceID=NULL
        AudioMicCaptureNative(const wchar_t* captureDeviceID,
                              WAVEFORMATEX* outWavFromat);
        ~AudioMicCaptureNative();
        void Start();
        void Stop();
        void GetData(byte* dataBuffer, int bufferSize, int* filledSize);
        bool IsCapturing() const;
    private:
        IAudioClient *mCaptureClient;
        IAudioCaptureClient *mCapturingClient;
        IMediaObject *mCaptureResampler;
        BYTE* mOutputCaptureBuf;
        int mOutputCaptureBufSize;
        int mOutputCaptureBufLeft;
        bool mIsInitialized;
        bool mIsCapturing;
        WAVEFORMATEX mWaveFormatMic;
        WAVEFORMATEX mWaveFormatOutput;
    };
}