#pragma once
#include <opencv2/opencv.hpp>
class DelaySumBeamformer
{
public:
	DelaySumBeamformer(int samplingRate, const std::vector<cv::Point3d>& arrayGeometry, const cv::Point3d& source, double speed);
	~DelaySumBeamformer();
	void compute(std::vector<double>& output, const std::vector<double>& input, const std::vector<double>& prevInput, int nChannel);
private:
	double distance(const cv::Point3d& p1, const cv::Point3d& p2);
	std::vector<int> mDelay;
};

