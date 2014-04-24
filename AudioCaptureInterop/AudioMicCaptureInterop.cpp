#include "AudioMicCaptureInterop.h"

#include <cstdio>
#include <cstdlib>
#include <vcclr.h>

using namespace AudioCaptureInterop;
using namespace AudioCaptureNative;
using namespace System;
using namespace System::Threading;

AudioMicCaptureInterop::AudioMicCaptureInterop(String^ captureDeviceID,
                                               AudioWaveFormatInterop^ captureWaveFormat,
                                               DataReadyCallback^ callbackFunction,
                                               int dataBufferDurationInMilliSecond) :
    mMicCapture(NULL),
    mCaptureWaveFormat(nullptr),
    mCallback(nullptr),
    mCaptureThread(nullptr),
    mDataBufferDurationInMilliSecond(0)
{
    if (captureWaveFormat == nullptr)
    {
        throw gcnew ArgumentException("Must supply captureWaveFormat");
    }
    if (callbackFunction == nullptr)
    {
        throw gcnew ArgumentException("Must supply callbackFunction");
    }
    if (dataBufferDurationInMilliSecond <= 0)
    {
        throw gcnew ArgumentException("mDataBufferDurationInMilliSecond must be positive");
    }

    mCaptureWaveFormat = captureWaveFormat;

    WAVEFORMATEX waveFormat;
    waveFormat.wFormatTag = captureWaveFormat->wFormatTag;
    waveFormat.nChannels = captureWaveFormat->nChannels;
    waveFormat.nSamplesPerSec = captureWaveFormat->nSamplesPerSec;
    waveFormat.nAvgBytesPerSec = captureWaveFormat->nAvgBytesPerSec;
    waveFormat.nBlockAlign = captureWaveFormat->nBlockAlign;
    waveFormat.wBitsPerSample = captureWaveFormat->wBitsPerSample;
    waveFormat.cbSize = captureWaveFormat->cbSize;

    if (captureDeviceID == nullptr)
    {
        mMicCapture = new AudioMicCaptureNative(NULL, &waveFormat);
    }
    else
    {
        pin_ptr<const wchar_t> wch = PtrToStringChars(captureDeviceID);
        mMicCapture = new AudioMicCaptureNative(wch, &waveFormat);
    }

    mCallback = callbackFunction;
    mDataBufferDurationInMilliSecond = dataBufferDurationInMilliSecond;
}

AudioMicCaptureInterop::~AudioMicCaptureInterop()
{
    this->!AudioMicCaptureInterop();
}

AudioMicCaptureInterop::!AudioMicCaptureInterop()
{
    if (mMicCapture)
    {
        if (mMicCapture->IsCapturing())
        {
            mMicCapture->Stop();
        }
        delete mMicCapture;
        mMicCapture = NULL;
    }
    if (mCaptureThread)
    {
        mCaptureThread->Abort();
        mCaptureThread->Join();
        mCaptureThread = nullptr;
    }
}

void AudioMicCaptureInterop::Start()
{
    Stop();

    mMicCapture->Start();

    ThreadStart^ task = gcnew ThreadStart(this, 
                                          &AudioMicCaptureInterop::ThreadTask);
    mCaptureThread = gcnew Thread(task);
    mCaptureThread->Priority = ThreadPriority::Normal;
    mCaptureThread->Start();
}

void AudioMicCaptureInterop::Stop()
{
    if (mMicCapture->IsCapturing())
    {
        mMicCapture->Stop();
    }
    if (mCaptureThread)
    {
        mCaptureThread->Abort();
        mCaptureThread->Join();
        mCaptureThread = nullptr;
    }
}

bool AudioMicCaptureInterop::IsCapturing::get()
{
    return mMicCapture->IsCapturing();
}

void AudioMicCaptureInterop::ThreadTask()
{
    int bufSize = mCaptureWaveFormat->nBlockAlign * 
                  mCaptureWaveFormat->nSamplesPerSec *
                  mDataBufferDurationInMilliSecond / 1000;
    int bufFilled = 0;
    int bufFilledPrev = 0;
    array<unsigned char>^ dataBuf = gcnew array<unsigned char>(bufSize);
    pin_ptr<unsigned char> nativeBuf = &dataBuf[0];

    while (IsCapturing)
    {
        bufFilled = 0;
        mMicCapture->GetData(nativeBuf, bufSize, &bufFilled);

        if (bufFilled > 0)
        {
            mCallback(dataBuf, bufFilled);
            bufFilledPrev = bufFilled;
        }
        else
        {
            Thread::Sleep(500 * bufFilledPrev / 
                          mCaptureWaveFormat->nBlockAlign / 
                          mCaptureWaveFormat->nSamplesPerSec);
        }
    }
}
