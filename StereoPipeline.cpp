#include "StereoPipeline.h"

StereoPipeline::StereoPipeline (QObject *parent)
    : QObject(parent)
{
}

StereoPipeline::~StereoPipeline ()
{
}

void StereoPipeline::setCalibration (const QString &, const QString &)
{
}


void StereoPipeline::setStereoMethod (StereoMethod *)
{
}

void StereoPipeline::processImagePair (const cv::Mat &, const cv::Mat &)
{
}
