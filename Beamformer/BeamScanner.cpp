#include "BeamScanner.h"


BeamScanner::BeamScanner(int samplingRate, const std::vector<cv::Point3d>& arrayGeometry, double speed) : mSamplingRate(samplingRate), mArrayGeometry(arrayGeometry), mSpeed(speed){

}


BeamScanner::~BeamScanner()
{
}

double BeamScanner::scan(const std::vector<double>& input, const std::vector<double>& prevInput){
	double radius = 1.0;
	const double PI = 3.1415926;
	double angleStep = 5.0 * PI / 180.0;
	double maxEnergy = 0.0;
	double maxAngle = 0.0;
	for (double angle = 0.0; angle < PI; angle += angleStep){
		double x = cos(angle);
		double y = sin(angle);
		double z = 0.0;
		cv::Point3d source(x, y, z);
		DelaySumBeamformer bf(mSamplingRate, mArrayGeometry, source, mSpeed);
		std::vector<double> output;
		bf.compute(output, input, prevInput, static_cast<int>(mArrayGeometry.size()));
		double energy = getEnergy(output);
		if (energy > maxEnergy){
			maxEnergy = energy;
			maxAngle = angle;
		}
	}
	return maxAngle;
}

double BeamScanner::getEnergy(const std::vector<double>& input){
	double sum = 0.0;
	for (double v : input){
		sum += v * v;
	}
	return sum;
}