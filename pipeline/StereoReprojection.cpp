#include "StereoReprojection.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/gpu/gpu.hpp>


StereoReprojection::StereoReprojection (QObject *parent)
{
    useGpu = false;
}

StereoReprojection::~StereoReprojection ()
{
}


void StereoReprojection::setUseGpu (bool enable)
{
    if (enable != useGpu) {
        useGpu = enable;
        emit useGpuChanged(enable);
    }
}


bool StereoReprojection::getUseGpu () const
{
    return useGpu;
}


void StereoReprojection::setReprojectionMatrix (const cv::Mat &newQ)
{
    Q = newQ;
    emit reprojectionMatrixChanged();
}

const cv::Mat &StereoReprojection::getReprojectionMatrix () const
{
    return Q;
}


void StereoReprojection::reprojectStereoDisparity (const cv::Mat &disparity, cv::Mat &points) const
{
    if (!useGpu) {
        cv::reprojectImageTo3D(disparity, points, Q, false, CV_32F);
    } else {
        cv::gpu::GpuMat gpu_disparity, gpu_points;

        gpu_disparity.upload(disparity);

        cv::gpu::reprojectImageTo3D(gpu_disparity, gpu_points, Q, 3);

        gpu_points.download(points);
    }
}
