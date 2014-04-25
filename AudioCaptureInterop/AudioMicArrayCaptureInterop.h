#pragma once

#include "AudioCaptureCommonInterop.h"
#include "AudioWaveFormatInterop.h"
#include "..\AudioCaptureNative\AudioMicArrayCaptureNative.h"

namespace AudioCaptureInterop
{
	public ref class AudioMicArrayCaptureInterop : IAudioCaptureInterop
	{
	public:
		// default capture device will be used with captureDeviceID=nullptr
		AudioMicArrayCaptureInterop(int kinectIndex,
			AudioWaveFormatInterop^ captureWaveFormat,
			DataReadyCallback^ callbackFunction,
			int callbackIntervalInMilliSecond);
		~AudioMicArrayCaptureInterop();
		!AudioMicArrayCaptureInterop();

		virtual void Start();
		virtual void Stop();
		virtual property bool IsCapturing { virtual bool get(); }
		double GetEnergy(){ return mMicCapture->GetEnergy(); }

	private:
		AudioCaptureNative::AudioMicArrayCaptureNative* mMicCapture;
		AudioWaveFormatInterop^ mCaptureWaveFormat;
		DataReadyCallback^ mCallback;
		System::Threading::Thread^ mCaptureThread;
		int mDataBufferDurationInMilliSecond;

		void ThreadTask();
	};
}

