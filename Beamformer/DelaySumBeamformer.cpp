#include "DelaySumBeamformer.h"


DelaySumBeamformer::DelaySumBeamformer(int samplingRate, const std::vector<cv::Point3d>& arrayGeometry, const cv::Point3d& source, double speed){
	// find distances of each microphone to the source.
	std::vector<double> distance_vector;
	distance_vector.reserve(arrayGeometry.size());
	for (auto& p : arrayGeometry){
		distance_vector.push_back(distance(source, p));
	}
	mDelay.reserve(distance_vector.size());
	double max_distance = *std::max_element(distance_vector.begin(), distance_vector.end());
	for (double d : distance_vector){
		mDelay.push_back(static_cast<int>((max_distance - d) / speed * (double)samplingRate + 0.5));
	}
}


DelaySumBeamformer::~DelaySumBeamformer()
{
}

void DelaySumBeamformer::compute(std::vector<double>& output, const std::vector<double>& input, const std::vector<double>& prevInput, int nChannel){
	if (input.size() != prevInput.size()){
		throw std::exception("input and prevInput sizes are not equal.");
	}
	if (nChannel != (int)mDelay.size()){
		throw std::exception("wrong number of channels.");
	}
	if (!output.empty())
		output.clear();
	int frameSize = static_cast<int>(input.size()) / nChannel;
	output.reserve(frameSize);
	for (int i = 0; i < frameSize; ++i){
		double sum = 0.0;
		for (int j = 0; j < nChannel; ++j){
			int index = i - mDelay[j];
			if (index >= 0){
				sum += input[nChannel * i + j];
			}
			else{
				sum += prevInput[nChannel * (frameSize + index) + j];
			}
		}
		output.push_back(sum / nChannel);
	}
}

double DelaySumBeamformer::distance(const cv::Point3d& p1, const cv::Point3d& p2){
	double dx = p1.x - p2.x;
	double dy = p1.y - p2.y;
	double dz = p1.z - p2.z;
	return sqrt(dx * dx + dy * dy + dz * dz);
}	