/*
 * Stereo Pipeline: stereo reprojection
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "StereoReprojection.h"

#include <opencv2/calib3d/calib3d.hpp>

#ifdef HAVE_OPENCV_GPU
#include <opencv2/gpu/gpu.hpp>
#endif


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
#ifdef HAVE_OPENCV_GPU
        useGpu = enable;
        if (useGpu && !cv::gpu::getCudaEnabledDeviceCount()) {
            emit error("No GPU found!");
            useGpu = false;
        }
#else
        if (enable) {
            emit error("Pipeline was compiled without GPU support!");
        }
        useGpu = false;
#endif

        // Emit in any case
        emit useGpuChanged(useGpu);
    }
}


bool StereoReprojection::getUseGpu () const
{
    return useGpu;
}


void StereoReprojection::setReprojectionMatrix (const cv::Mat &newQ)
{
    Q = newQ.clone(); // Copy
    emit reprojectionMatrixChanged();
}

const cv::Mat &StereoReprojection::getReprojectionMatrix () const
{
    return Q;
}


void StereoReprojection::reprojectStereoDisparity (const cv::Mat &disparity, cv::Mat &points) const
{
    // Validate reprojection matrix
    if (Q.rows != 4 || Q.cols != 4 /*|| Q.type() != CV_32F || !Q.isContinuous()*/) {
        qWarning() << "Reprojection: invalid Q!";
        points = cv::Mat();
        return;
    }

    // Reproject
    if (!useGpu) {
        cv::reprojectImageTo3D(disparity, points, Q, false, CV_32F);
    } else {
#ifdef HAVE_OPENCV_GPU
        cv::gpu::GpuMat gpu_disparity, gpu_points;

        gpu_disparity.upload(disparity);

        cv::gpu::reprojectImageTo3D(gpu_disparity, gpu_points, Q, 3);

        gpu_points.download(points);
#else
        points = cv::Mat();
#endif
    }
}
