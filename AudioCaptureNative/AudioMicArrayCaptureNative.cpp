#include "AudioMicArrayCaptureNative.h"
#include "CStaticMediaBuffer.h"


using namespace AudioCaptureNative;

AudioMicArrayCaptureNative::AudioMicArrayCaptureNative(int captureDeviceIndex,
	WAVEFORMATEX* outWavFromat) : mNuiSensor(NULL), mNuiAudioBeam(NULL), 
	mCaptureResampler(NULL), mPropertyStore(NULL), mIsInitialized(false), mIsCapturing(false), mEnergy(0), mSamples(0), mDeviceIndex(captureDeviceIndex)
{
	HRESULT hr;
	hr = NuiCreateSensorByIndex(captureDeviceIndex, &mNuiSensor);
	if (FAILED(hr))
	{
		throw std::invalid_argument("Invalid capture device ID");
	}
	memcpy(&mWaveFormatOutput, outWavFromat, sizeof(WAVEFORMATEX));
	InitializeAudioSource();
	mIsInitialized = true;
}


AudioMicArrayCaptureNative::~AudioMicArrayCaptureNative()
{
	if (mNuiSensor)
	{
		mNuiSensor->NuiShutdown();
	}
}

HRESULT AudioMicArrayCaptureNative::InitializeAudioSource()
{
	HRESULT hr = mNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_AUDIO);
	if (FAILED(hr))
	{
		throw std::exception("Failure in sensor initialization");
	}
	if (FAILED(hr))
	{
		throw std::exception("Failure in audio source initialization");
	}
	hr = mNuiSensor->NuiGetAudioSource(&mNuiAudioBeam);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = mNuiAudioBeam->QueryInterface(IID_IMediaObject, (void**)&mCaptureResampler);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = mNuiAudioBeam->QueryInterface(IID_IPropertyStore, (void**)&mPropertyStore);
	if (FAILED(hr))
	{
		return hr;
	}
	PROPVARIANT pvSysMode;
	PropVariantInit(&pvSysMode);
	pvSysMode.vt = VT_I4;
	pvSysMode.lVal = (LONG)(2);
	mPropertyStore->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode);
	PropVariantClear(&pvSysMode);

	DMO_MEDIA_TYPE mt = { 0 };
	hr = MoInitMediaType(&mt, sizeof(WAVEFORMATEX));
	if (FAILED(hr))
	{
		return hr;
	}
	mt.majortype = MEDIATYPE_Audio;
	mt.subtype = MEDIASUBTYPE_PCM;
	mt.lSampleSize = 0;
	mt.bFixedSizeSamples = TRUE;
	mt.bTemporalCompression = FALSE;
	mt.formattype = FORMAT_WaveFormatEx;
	memcpy_s(mt.pbFormat, sizeof(WAVEFORMATEX), &mWaveFormatOutput, sizeof(WAVEFORMATEX));
	hr = mCaptureResampler->SetOutputType(0, &mt, 0);
	MoFreeMediaType(&mt);
	mIsInitialized = true;
	return hr;
}

void AudioMicArrayCaptureNative::GetData(byte* dataBuffer, int bufferSize, int* filledSize)
{
	ULONG cbProduced = 0;
	BYTE *pProduced = NULL;
	DWORD dwStatus = 0;
	DMO_OUTPUT_DATA_BUFFER outputBuffer = { 0 };
	CStaticMediaBuffer outputCaptureBuffer;
	outputBuffer.pBuffer = &outputCaptureBuffer;
	HRESULT hr = S_OK;
	*filledSize = 0;
	do
	{
		outputCaptureBuffer.Init(0);
		outputBuffer.dwStatus = 0;
		hr = mCaptureResampler->ProcessOutput(0, 1, &outputBuffer, &dwStatus);
		if (FAILED(hr))
		{
			throw std::exception("Error from ProcessOutput of capture client");
		}
		if (hr == S_FALSE)
		{
			cbProduced = 0;
		}
		else
		{
			outputCaptureBuffer.GetBufferAndLength(&pProduced, &cbProduced);
		}
		if (cbProduced > 0)
		{
			double beamAngle, sourceAngle, sourceConfidence;

			// Obtain beam angle from INuiAudioBeam afforded by microphone array
			mNuiAudioBeam->GetBeam(&beamAngle);
			mNuiAudioBeam->GetPosition(&sourceAngle, &sourceConfidence);

			// Calculate energy from audio
			for (UINT i = 0; i < cbProduced; i += 2)
			{
				// compute the sum of squares of audio samples that will get accumulated
				// into a single energy value.
				short audioSample = static_cast<short>(pProduced[i] | (pProduced[i + 1] << 8));
				mEnergy += audioSample * audioSample;
				++mSamples;
			}
		}
		memcpy(dataBuffer + *filledSize, pProduced, cbProduced);
		*filledSize += cbProduced;
		if (!mIsCapturing)
		{
			break;
		}
	} while (outputBuffer.dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE);

}

void AudioMicArrayCaptureNative::Start()
{
	mOutputCaptureBufLeft = 0;
	mIsCapturing = true;
	mEnergy = 0;
	mSamples = 0;
}

void AudioMicArrayCaptureNative::Stop()
{
	mIsCapturing = false;
}

bool AudioMicArrayCaptureNative::IsCapturing() const
{
	return mIsCapturing;
}