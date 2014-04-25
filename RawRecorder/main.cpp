#include "..\AudioCaptureNative\AudioMicArrayCaptureNative.h"
#include "..\AudioCaptureNative\AudioMicCaptureNative.h"
#include <chrono>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <Pipeline.h>

int main(int argc, char* argv[]){
	if (argc == 1){
		std::cout << "BeamFormer.exe output_dir" << std::endl;
		return 0;
	}
	std::string output_dir = argv[1];
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr)){
		WAVEFORMATEX rawFormat;
		rawFormat.wFormatTag = WAVE_FORMAT_PCM;
		rawFormat.nChannels = 4;
		rawFormat.wBitsPerSample = 16;
		rawFormat.nBlockAlign = (rawFormat.wBitsPerSample / 8) * rawFormat.nChannels;
		rawFormat.nSamplesPerSec = 16000;
		rawFormat.nAvgBytesPerSec = rawFormat.nBlockAlign * rawFormat.nSamplesPerSec;
		rawFormat.cbSize = 0;

		WAVEFORMATEX arrayFormat;
		arrayFormat.wFormatTag = WAVE_FORMAT_PCM;
		arrayFormat.nChannels = 1;
		arrayFormat.wBitsPerSample = 16;
		arrayFormat.nBlockAlign = (arrayFormat.wBitsPerSample / 8) * arrayFormat.nChannels;
		arrayFormat.nSamplesPerSec = 16000;
		arrayFormat.nAvgBytesPerSec = arrayFormat.nBlockAlign * arrayFormat.nSamplesPerSec;
		arrayFormat.cbSize = 0;

		AudioCaptureNative::AudioMicCaptureNative* rawCapture = new AudioCaptureNative::AudioMicCaptureNative(NULL, &rawFormat);
		rawCapture->Start();

		AudioCaptureNative::AudioMicArrayCaptureNative* arrayCapture = new AudioCaptureNative::AudioMicArrayCaptureNative(0, &arrayFormat);
		arrayCapture->Start();
		// define the output
		WAVEFORMATEX outputFormat;
		outputFormat.wFormatTag = WAVE_FORMAT_PCM;
		outputFormat.nChannels = 1;
		outputFormat.wBitsPerSample = 16;
		outputFormat.nBlockAlign = (outputFormat.wBitsPerSample / 8) * outputFormat.nChannels;
		outputFormat.nSamplesPerSec = 16000;
		outputFormat.nAvgBytesPerSec = outputFormat.nBlockAlign * outputFormat.nSamplesPerSec;
		outputFormat.cbSize = 0;
		
		std::stringstream ss;
		ss << output_dir << "/" << "raw.wav";
		std::string raw_file;
		std::string array_file;
		ss >> raw_file;
		ss << output_dir << "/" << "array.wav";
		ss >> array_file;
		Beam::WavWriter rawWriter(raw_file, 16000, 4);
		Beam::WavWriter arrayWriter(array_file, 16000, 1);

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
				rawCapture->GetData(buffer + buffer_filled, buffer_size, &filled);
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
					rawWriter.write(consumer_buffer, frame_size_bytes);
				}
			}
		});
		producer.join();
		consumer.join();
		delete[] buffer;
		delete rawCapture;
		delete arrayCapture;
		CoUninitialize();
	}
	return 0;
}