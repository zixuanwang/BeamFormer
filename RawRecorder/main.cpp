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
		
		std::stringstream raw_ss;
		std::stringstream array_ss;
		raw_ss << output_dir << "/" << "raw.wav";
		std::string raw_file;
		std::string array_file;
		raw_ss >> raw_file;
		array_ss << output_dir << "/" << "array.wav";
		array_ss >> array_file;
		Beam::WavWriter rawWriter(raw_file, 16000, 4);
		Beam::WavWriter arrayWriter(array_file, 16000, 1);
		bool recording = true;

		int raw_buffer_filled = 0;
		int array_buffer_filled = 0;
		int raw_buffer_size = 16000 * 8;
		int array_buffer_size = 16000 * 2;
		byte* raw_buffer = new byte[raw_buffer_size];
		byte* array_buffer = new byte[array_buffer_size];
		std::condition_variable raw_condition_var;
		std::condition_variable array_condition_var;
		std::mutex raw_mutex;
		std::mutex array_mutex;
		std::thread raw_producer([&]{
			while (recording){
				std::unique_lock<std::mutex> lk(raw_mutex);
				int filled = 0;
				rawCapture->GetData(raw_buffer + raw_buffer_filled, raw_buffer_size, &filled);
				raw_buffer_filled += filled;
				if (raw_buffer_filled > FRAME_SIZE * 8){
					raw_condition_var.notify_one();
				}
				std::chrono::milliseconds dura(5);
				std::this_thread::sleep_for(dura);
			}
			raw_condition_var.notify_one();
		});
		std::thread raw_consumer([&]{
			const int frame_size_bytes = FRAME_SIZE * 8;
			char consumer_buffer[frame_size_bytes];
			while (recording){
				std::unique_lock<std::mutex> lk(raw_mutex);
				raw_condition_var.wait(lk);
				while (raw_buffer_filled > frame_size_bytes){
					memcpy(consumer_buffer, raw_buffer, frame_size_bytes);
					memcpy(raw_buffer, raw_buffer + frame_size_bytes, raw_buffer_filled - frame_size_bytes);
					raw_buffer_filled -= frame_size_bytes;
					// process the frame
					rawWriter.write(consumer_buffer, frame_size_bytes);
				}
			}
		});

		std::thread array_producer([&]{
			while (recording){
				std::unique_lock<std::mutex> lk(array_mutex);
				int filled = 0;
				arrayCapture->GetData(array_buffer + array_buffer_filled, array_buffer_size, &filled);
				array_buffer_filled += filled;
				if (array_buffer_filled > FRAME_SIZE * 8){
					array_condition_var.notify_one();
				}
				std::chrono::milliseconds dura(5);
				std::this_thread::sleep_for(dura);
			}
			array_condition_var.notify_one();
		});
		std::thread array_consumer([&]{
			const int frame_size_bytes = FRAME_SIZE * 8;
			char consumer_buffer[frame_size_bytes];
			while (recording){
				std::unique_lock<std::mutex> lk(array_mutex);
				array_condition_var.wait(lk);
				while (array_buffer_filled > frame_size_bytes){
					memcpy(consumer_buffer, array_buffer, frame_size_bytes);
					memcpy(array_buffer, array_buffer + frame_size_bytes, array_buffer_filled - frame_size_bytes);
					array_buffer_filled -= frame_size_bytes;
					// process the frame
					arrayWriter.write(consumer_buffer, frame_size_bytes);
				}
			}
		});
		std::thread control([&]{
			std::string c;
			std::cin >> c;
			recording = false;
		});
		raw_producer.join();
		raw_consumer.join();
		array_producer.join();
		array_consumer.join();
		control.join();
		delete[] raw_buffer;
		delete[] array_buffer;
		delete rawCapture;
		delete arrayCapture;
		CoUninitialize();
	}
	return 0;
}