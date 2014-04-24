#include "SimpleVAD.h"


SimpleVAD::SimpleVAD(int samplingRate, int frameLength) : mTauDown(0.04), mTauUp(1.0), mTDown(1.2), mTUp(4.0), mPrevResult(0.0), mFrameLength(frameLength), mPrevL(0.0), mPrevLMin(0.0){
	mFrameSizeInSeconds = (double)mFrameLength / samplingRate;
	// initialize the windows function. bandpass filter.
	// each element should be the square of the original value.
	double frequencyGap = (double)samplingRate / frameLength;
	int lowIndex = static_cast<int>(300.0 / frequencyGap);
	int highIndex = static_cast<int>(4000.0 / frequencyGap);
	mWeightFunction = cv::Mat(mFrameLength, 1, CV_64FC1, cv::Scalar(0.0));
	cv::Mat window = mWeightFunction(cv::Rect(0, lowIndex, 1, highIndex - lowIndex));
	window.setTo(1.0);
}


SimpleVAD::~SimpleVAD()
{
}

double SimpleVAD::detect(double* frameData, int frameSize){
	if (frameSize != mFrameLength){
		throw std::exception("wrong size of the input frame.");
	}
	cv::Mat frame(frameSize, 1, CV_64FC1, frameData);
	cv::Mat frameFrequencyComplex;
	cv::dft(frame, frameFrequencyComplex, cv::DFT_COMPLEX_OUTPUT);
	// compute norm for each row.
	cv::Mat frameFrequencyNorm(frameSize, 1, CV_64FC1, cv::Scalar(0));
	for (int i = 0; i <= frameSize / 2; ++i){
		double* cPtr = frameFrequencyComplex.ptr<double>(i);
		double norm = cPtr[0] * cPtr[0] + cPtr[1] * cPtr[1];
		frameFrequencyNorm.ptr<double>(i)[0] = norm;
		if (frameSize - i < frameSize){
			frameFrequencyNorm.ptr<double>(frameSize - i)[0] = norm;
		}
	}
	cv::Mat weightFrequencyNorm;
	cv::multiply(frameFrequencyNorm, mWeightFunction, weightFrequencyNorm);
	double L = sqrt(cv::mean(weightFrequencyNorm).val[0]);
	double tau = L > mPrevLMin ? mTauUp : mTauDown;
	mPrevLMin = (1.0 - mFrameSizeInSeconds / tau) * mPrevLMin + mFrameSizeInSeconds / tau * L;
	if (L / mPrevLMin < mTDown){
		mPrevResult = 0.0;
	}
	if (L / mPrevLMin > mTUp){
		mPrevResult = 1.0;
	}
	//std::cout << "L: " << L << std::endl;
	//std::cout << "L_Min: " << mPrevLMin << std::endl;
	//std::cout << "L / L_Min: " << L / mPrevLMin << std::endl;
	return mPrevResult;
}