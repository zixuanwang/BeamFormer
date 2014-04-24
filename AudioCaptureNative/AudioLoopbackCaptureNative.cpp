#include "AudioCaptureCommonNative.h"
#include "AudioLoopbackCaptureNative.h"
#include "StaticMediaBuffer.h"

#include <cstdlib>
#include <stdexcept>
#include <atlstr.h>

using namespace AudioCaptureNative;

AudioLoopbackCaptureNative::AudioLoopbackCaptureNative(WAVEFORMATEX* outWavFormat) :
    mCaptureClient(NULL),
    mCapturingClient(NULL),
    mCaptureResampler(NULL),
    mOutputCaptureBuf(NULL),
    mOutputCaptureBufSize(0),
    mOutputCaptureBufLeft(0),
    mIsCapturing(false),
    mIsInitialized(false)
{
    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    DMO_MEDIA_TYPE mt = {0};
    WAVEFORMATEXTENSIBLE* loopbackWaveFormatEx = NULL;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    CString errorMessage;

    if (outWavFormat == NULL)
    {
        throw std::invalid_argument("Must provide outWaveFormat");
    }

    if (outWavFormat->wFormatTag != WAVE_FORMAT_PCM ||
        outWavFormat->wBitsPerSample != 16)
    {
        throw std::invalid_argument(
            "Invalid wave format: FormatTag=WAVE_FORMAT_PCM, wBitsPerSample=16");
    }

    // initialize IAudioClient
    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, 
                          NULL,
                          CLSCTX_ALL, 
                          IID_IMMDeviceEnumerator,
                          (void**)&pEnumerator);
    if (FAILED(hr))
    {
        errorMessage = CString("CoCreateInstance returned error");
        goto Exit;
    }

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr))
    {
        errorMessage = CString("Error from GetDevice/GetDefaultAudioEndpoint()");
        goto Exit;
    }

    hr = pDevice->Activate(IID_IAudioClient,
                           CLSCTX_ALL,
                           NULL, 
                           (void**)&mCaptureClient);
    if (FAILED(hr))
    {
        errorMessage = CString("Error from Activate()");
        goto Exit;
    }

    hr = mCaptureClient->GetMixFormat((WAVEFORMATEX**)&loopbackWaveFormatEx);
    if (FAILED(hr))
    {
        errorMessage = CString("Error from GetMixFormat()");
        goto Exit;
    }

    // loopback capture works only with shared mode
    hr = mCaptureClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                    AUDCLNT_STREAMFLAGS_LOOPBACK,
                                    hnsRequestedDuration,
                                    0,
                                    (WAVEFORMATEX*)loopbackWaveFormatEx,
                                    NULL);
    if (FAILED(hr))
    {
        errorMessage = CString("Error from Initialize()");
        goto Exit;
    }

    hr = mCaptureClient->GetService(IID_IAudioCaptureClient,
                                    (void**)&mCapturingClient);
    if (FAILED(hr))
    {
        errorMessage = CString("Error from GetService()");
        goto Exit;
    }

    // initialize resampler
    hr = CoCreateInstance(CLSID_CResamplerMediaObject,
                            NULL, 
                            CLSCTX_INPROC_SERVER, 
                            IID_IMediaObject,
                            (void**)&mCaptureResampler);
    if (FAILED(hr))
    {
        errorMessage = CString("Error from CoCreateInstance() of resampler");
        goto Exit;
    }
    
    hr = MoInitMediaType(&mt, sizeof(WAVEFORMATEXTENSIBLE));
    if (FAILED(hr))
    {
        errorMessage = CString("Error from MoInitMediaType()");
        goto Exit;
    }

    mt.majortype = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;
    mt.lSampleSize = 0;
    mt.bFixedSizeSamples = TRUE;
    mt.bTemporalCompression = FALSE;
    mt.formattype = FORMAT_WaveFormatEx;
    memcpy(mt.pbFormat, loopbackWaveFormatEx, sizeof(WAVEFORMATEXTENSIBLE));
                    
    hr = mCaptureResampler->SetInputType(0, &mt, 0);
    if (FAILED(hr))
    {
        errorMessage = CString("Error from SetInputType() of resampler");
        goto Exit;
    }

    MoFreeMediaType(&mt);

    hr = MoInitMediaType(&mt, sizeof(WAVEFORMATEX));
    if (FAILED(hr))
    {
        errorMessage = CString("Error from MoInitMediaType()");
        goto Exit;
    }
                
    mt.majortype = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;
    mt.lSampleSize = 0;
    mt.bFixedSizeSamples = TRUE;
    mt.bTemporalCompression = FALSE;
    mt.formattype = FORMAT_WaveFormatEx;
    memcpy(mt.pbFormat, outWavFormat, sizeof(WAVEFORMATEX));

    hr = mCaptureResampler->SetOutputType(0, &mt, 0);
    if (FAILED(hr))
    {
        errorMessage = CString("Error from SetOutputType() of resampler");
        goto Exit;
    }
    
    MoFreeMediaType(&mt);

    mOutputCaptureBufSize = (int)(outWavFormat->nAvgBytesPerSec * 
                                    hnsRequestedDuration / 
                                    REFTIMES_PER_SEC);
    mOutputCaptureBuf = new BYTE[mOutputCaptureBufSize];

    memcpy(&mWaveFormatLoopback, loopbackWaveFormatEx, sizeof(WAVEFORMATEX));

    memcpy(&mWaveFormatOutput, outWavFormat, sizeof(WAVEFORMATEX));
    mIsInitialized = true;

Exit:
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pDevice);
    CoTaskMemFree(loopbackWaveFormatEx);

    if (FAILED(hr))
    {
        throw std::exception(errorMessage);
    }
}

AudioLoopbackCaptureNative::~AudioLoopbackCaptureNative()
{
    SAFE_RELEASE(mCaptureClient);
    SAFE_RELEASE(mCapturingClient);
    SAFE_RELEASE(mCaptureResampler);
    if (mOutputCaptureBuf)
    {
        delete mOutputCaptureBuf;
        mOutputCaptureBuf = NULL;
    }
    mIsInitialized = false;
}

void AudioLoopbackCaptureNative::Start()
{
    if (!mIsInitialized)
    {
        throw std::logic_error("This instance is not properly initialized");
    }

    mOutputCaptureBufLeft = 0;
    mCaptureClient->Reset();
    mCaptureClient->Start();
    mIsCapturing = true;
}

void AudioLoopbackCaptureNative::Stop()
{
    if (!mIsInitialized)
    {
        throw std::logic_error("This instance is not properly initialized");
    }

    mCaptureClient->Stop();
    mIsCapturing = false;
}

void AudioLoopbackCaptureNative::GetData(byte* dataBuffer, int bufferSize, int* filledSize)
{
    HRESULT hr;
    UINT32 packetLength = 0;
    UINT32 numFramesAvailable;
    BYTE *pDataMic, *pDataOut;
    ULONG numBytesAvailable;
    DWORD flags;
    StaticMediaBuffer inputCaptureBuffer;
    StaticMediaBuffer outputCaptureBuffer;
    DMO_OUTPUT_DATA_BUFFER OutputCaptureBufferStruct = {0};

    if (!mIsInitialized)
    {
        throw std::logic_error("This instance is not properly initialized");
    }

    if (dataBuffer == NULL || bufferSize <= 0)
    {
        throw std::invalid_argument("Can't put captured samples");
    }

    *filledSize = 0;
    if (!mIsCapturing)
    {
        return;
    }

    // copy leftover to output buffer
    if (mOutputCaptureBufLeft > 0)
    {
        memcpy(dataBuffer, mOutputCaptureBuf, mOutputCaptureBufLeft);
        *filledSize = mOutputCaptureBufLeft;
        mOutputCaptureBufLeft = 0;
    }

    OutputCaptureBufferStruct.pBuffer = &outputCaptureBuffer;
    outputCaptureBuffer.Init(mOutputCaptureBuf, mOutputCaptureBufSize, 0);

    hr = mCapturingClient->GetNextPacketSize(&packetLength);
    if (FAILED(hr))
    {
        throw std::exception("Error from GetNextPacketSize() of capture client");
    }

    while (packetLength > 0)
    {
        // Get available data
        hr = mCapturingClient->GetBuffer(&pDataMic,
                                         &numFramesAvailable,
                                         &flags, 
                                         NULL, 
                                         NULL);
        if (FAILED(hr))
        {
            throw std::exception("Error from GetBuffer() of capture client");
        }

        if ((flags & AUDCLNT_BUFFERFLAGS_SILENT) == 0)
        {
            numBytesAvailable = numFramesAvailable * mWaveFormatLoopback.nBlockAlign;
            if (mCaptureResampler)
            {
                // resampling
                inputCaptureBuffer.Init(pDataMic, numBytesAvailable, numBytesAvailable);
                hr = mCaptureResampler->ProcessInput(0, &inputCaptureBuffer, 0, 0, 0);
                if (FAILED(hr))
                {
                    throw std::exception("Error from ProcessInput() of resampler");
                }

                outputCaptureBuffer.Init(mOutputCaptureBuf, mOutputCaptureBufSize, 0);
                hr = mCaptureResampler->ProcessOutput(0, 1, &OutputCaptureBufferStruct, &flags);
                if (FAILED(hr))
                {
                    throw std::exception("Error from ProcessOutput() of resampler");
                }

                outputCaptureBuffer.GetBufferAndLength(&pDataOut, &numBytesAvailable);
            }
            else
            {
                pDataOut = pDataMic;
            }

            // copy data to output buffer
            if ((int)(*filledSize + numBytesAvailable) <= bufferSize)
            {
                memcpy(dataBuffer + *filledSize, pDataOut, numBytesAvailable);
                *filledSize += numBytesAvailable;
            }
            else
            {
                int pushedSize = bufferSize - *filledSize;
                memcpy(dataBuffer + *filledSize, pDataOut, pushedSize);
                *filledSize += pushedSize;
                // handle carry over
                mOutputCaptureBufLeft = numBytesAvailable - pushedSize;
                memcpy(mOutputCaptureBuf, pDataOut + pushedSize, mOutputCaptureBufLeft);
            }
        }

        hr = mCapturingClient->ReleaseBuffer(numFramesAvailable);
        if (FAILED(hr))
        {
            throw std::exception("Error from ReleaseBuffer() of capture client");
        }

        if (*filledSize == bufferSize)
        {
            break;
        }

        hr = mCapturingClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr))
        {
            throw std::exception("Error from GetNextPacketSize() of capture client");
        }
    }
}

bool AudioLoopbackCaptureNative::IsCapturing() const
{
    if (!mIsInitialized)
    {
        throw std::logic_error("This instance is not properly initialized");
    }
    return mIsCapturing;
}
