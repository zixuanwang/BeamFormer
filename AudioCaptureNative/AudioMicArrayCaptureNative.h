#pragma once

#include <Windows.h>
#include <dmo.h>
#include <wmcodecdsp.h>
#include <mmreg.h>
#include <uuids.h>
#include <propsys.h>
#include <NuiApi.h>
#include <stdexcept>
namespace AudioCaptureNative
{
	class AudioMicArrayCaptureNative
	{
	public:
		AudioMicArrayCaptureNative(int captureDeviceIndex,
								   WAVEFORMATEX* outWavFromat);
		~AudioMicArrayCaptureNative();
		void Start();
		void Stop();
		void GetData(byte* dataBuffer, int bufferSize, int* filledSize);
		bool IsCapturing() const;
		double GetEnergy(){ return (double) mEnergy / (double) mSamples; }
	private:
		INuiSensor *mNuiSensor;
		INuiAudioBeam *mNuiAudioBeam;
		IMediaObject *mCaptureResampler;
		IPropertyStore *mPropertyStore;
		BYTE* mOutputCaptureBuf;
		int mOutputCaptureBufSize;
		int mOutputCaptureBufLeft;
		bool mIsInitialized;
		bool mIsCapturing;
		WAVEFORMATEX mWaveFormatMic;
		WAVEFORMATEX mWaveFormatOutput;
		HRESULT InitializeAudioSource();
		long long mEnergy;
		long long mSamples;
		int mDeviceIndex;
	};
}

