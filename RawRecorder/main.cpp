#include "..\AudioCaptureNative\AudioMicCaptureNative.h"
#include <chrono>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <iostream>
#include <thread>
#include <Pipeline.h>

int main(int argc, char* argv[]){
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr)){
		WAVEFORMATEX waveFormat;
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nChannels = 4;
		waveFormat.wBitsPerSample = 16;
		waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
		waveFormat.nSamplesPerSec = 16000;
		waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec;
		waveFormat.cbSize = 0;
		AudioCaptureNative::AudioMicCaptureNative* mMicCapture = new AudioCaptureNative::AudioMicCaptureNative(NULL, &waveFormat);
		mMicCapture->Start();
		// define the output
		WAVEFORMATEX outputFormat;
		outputFormat.wFormatTag = WAVE_FORMAT_PCM;
		outputFormat.nChannels = 1;
		outputFormat.wBitsPerSample = 16;
		outputFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
		outputFormat.nSamplesPerSec = 16000;
		outputFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec;
		outputFormat.cbSize = 0;
		Beam::WavWriter ww("c:/users/danwa/desktop/1.wav", 16000, 4);

		// define the buffer
		int buffer_filled = 0;
		int buffer_size = 16000 * 8;
		byte* buffer = new byte[buffer_size];
		std::condition_variable condition_var;
		std::mutex mutex;
		int output_offset = 0;
		// initialize profile
		std::thread producer([&]{
			while (true){
				std::unique_lock<std::mutex> lk(mutex);
				int filled = 0;
				mMicCapture->GetData(buffer + buffer_filled, buffer_size, &filled);
				buffer_filled += filled;
				if (buffer_filled > FRAME_SIZE * 8){
					condition_var.notify_one();
				}
				std::chrono::milliseconds dura(5);
				std::this_thread::sleep_for(dura);
			}
		});
		std::thread consumer([&]{
			const int frame_size_bytes = FRAME_SIZE * 8;
			char consumer_buffer[frame_size_bytes];
			std::vector<float> input[MAX_MICROPHONES];
			std::vector<float> input_prev[MAX_MICROPHONES];
			std::vector<std::complex<float> > frequency_input[MAX_MICROPHONES];
			std::vector<std::complex<float> > beamformer_output(FRAME_SIZE);
			std::vector<float> output(2 * FRAME_SIZE);
			for (int channel = 0; channel < MAX_MICROPHONES; ++channel){
				input[channel].assign(FRAME_SIZE * 2, 0.f);
				input_prev[channel].assign(FRAME_SIZE * 2, 0.f);
				frequency_input[channel].assign(FRAME_SIZE, std::complex<float>(0.f, 0.f));
			}
			while (true){
				std::unique_lock<std::mutex> lk(mutex);
				condition_var.wait(lk);
				while (buffer_filled > frame_size_bytes){
					memcpy(consumer_buffer, buffer, frame_size_bytes);
					memcpy(buffer, buffer + frame_size_bytes, buffer_filled - frame_size_bytes);
					buffer_filled -= frame_size_bytes;
					// process the frame
					ww.write(consumer_buffer, frame_size_bytes);
				}
			}
		});
		producer.join();
		consumer.join();
		delete[] buffer;
		delete mMicCapture;
		CoUninitialize();
	}
	return 0;
}