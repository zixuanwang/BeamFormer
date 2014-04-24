#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>

class SimpleVAD
{
public:
	SimpleVAD(int samplingRate, int frameLength);
	~SimpleVAD();
	double detect(double* frameData, int frameSize);
private:
	double mTauDown;
	double mTauUp;
	double mTDown;
	double mTUp;
	double mPrevResult;
	int mFrameLength; // the number of samples within one frame.
	double mPrevL;
	double mPrevLMin;
	double mFrameSizeInSeconds;
	cv::Mat mWeightFunction; // give weights to frequency bins.
};

