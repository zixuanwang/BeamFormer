#pragma once
#include "DelaySumBeamformer.h"

class BeamScanner
{
public:
	BeamScanner(int samplingRate, const std::vector<cv::Point3d>& arrayGeometry, double speed);
	~BeamScanner();
	double scan(const std::vector<double>& input, const std::vector<double>& prevInput);
private:
	double getEnergy(const std::vector<double>& input);
	int mSamplingRate;
	std::vector<cv::Point3d> mArrayGeometry;
	double mSpeed;
};

