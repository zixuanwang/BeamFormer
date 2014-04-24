#include "AudioLoopbackCaptureInterop.h"

#include <cstdio>
#include <cstdlib>
#include <vcclr.h>

using namespace AudioCaptureInterop;
using namespace AudioCaptureNative;
using namespace System;
using namespace System::Threading;

AudioLoopbackCaptureInterop::AudioLoopbackCaptureInterop(AudioWaveFormatInterop^ captureWaveFormat,
                                                         DataReadyCallback^ callbackFunction,
                                                         int dataBufferDurationInMilliSecond) :
    mLoopbackCapture(NULL),
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

    mLoopbackCapture = new AudioLoopbackCaptureNative(&waveFormat);

    mCallback = callbackFunction;
    mDataBufferDurationInMilliSecond = dataBufferDurationInMilliSecond;
}

AudioLoopbackCaptureInterop::~AudioLoopbackCaptureInterop()
{
    this->!AudioLoopbackCaptureInterop();
}

AudioLoopbackCaptureInterop::!AudioLoopbackCaptureInterop()
{
    if (mLoopbackCapture)
    {
        if (mLoopbackCapture->IsCapturing())
        {
            mLoopbackCapture->Stop();
        }
        delete mLoopbackCapture;
        mLoopbackCapture = NULL;
    }
    if (mCaptureThread)
    {
        mCaptureThread->Abort();
        mCaptureThread->Join();
        mCaptureThread = nullptr;
    }
}

void AudioLoopbackCaptureInterop::Start()
{
    Stop();

    mLoopbackCapture->Start();

    ThreadStart^ task = gcnew ThreadStart(this, 
                                          &AudioLoopbackCaptureInterop::ThreadTask);
    mCaptureThread = gcnew Thread(task);
    mCaptureThread->Priority = ThreadPriority::Normal;
    mCaptureThread->Start();
}

void AudioLoopbackCaptureInterop::Stop()
{
    if (mLoopbackCapture->IsCapturing())
    {
        mLoopbackCapture->Stop();
    }
    if (mCaptureThread)
    {
        mCaptureThread->Abort();
        mCaptureThread->Join();
        mCaptureThread = nullptr;
    }
}

bool AudioLoopbackCaptureInterop::IsCapturing::get()
{
    return mLoopbackCapture->IsCapturing();
}

void AudioLoopbackCaptureInterop::ThreadTask()
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
        mLoopbackCapture->GetData(nativeBuf, bufSize, &bufFilled);

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
