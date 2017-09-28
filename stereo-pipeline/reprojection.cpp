/*
 * Stereo Pipeline: reprojection
 * Copyright (C) 2013-2017 Rok Mandeljc
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

#include <stereo-pipeline/reprojection.h>

#include <opencv2/opencv_modules.hpp>
#include <opencv2/calib3d.hpp>

#ifdef HAVE_OPENCV_CUDASTEREO
#include <opencv2/cudastereo.hpp>
#endif


#include "reprojection_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


ReprojectionPrivate::ReprojectionPrivate (Reprojection *parent)
    : q_ptr(parent)
{
    // Create list of supported methods
    supportedMethods.append(Reprojection::MethodOpenCvCpu);
#ifdef HAVE_OPENCV_CUDASTEREO
    try {
        if (cv::cuda::getCudaEnabledDeviceCount()) {
            supportedMethods.append(Reprojection::MethodOpenCvCuda);
        }
    } catch (...) {
        // Nothing to do :)
    }
#endif

    // Default method: OpenCV CPU
    reprojectionMethod = Reprojection::MethodOpenCvCpu;
}


Reprojection::Reprojection (QObject *parent)
    : QObject(parent), d_ptr(new ReprojectionPrivate(this))
{
}

Reprojection::~Reprojection ()
{
}


// *********************************************************************
// *                        Reprojection method                        *
// *********************************************************************
void Reprojection::setReprojectionMethod (int method)
{
    Q_D(Reprojection);

    if (method == d->reprojectionMethod) {
        return;
    }

    // Make sure method is supported
    if (!d->supportedMethods.contains(method)) {
        d->reprojectionMethod = MethodOpenCvCpu;
        emit error(QString("Reprojection method %1 not supported!").arg(method));
    } else {
        d->reprojectionMethod = method;
    }

    // Emit in any case
    emit reprojectionMethodChanged(d->reprojectionMethod);
}


int Reprojection::getReprojectionMethod () const
{
    Q_D(const Reprojection);
    return d->reprojectionMethod;
}


const QList<int> &Reprojection::getSupportedReprojectionMethods () const
{
    Q_D(const Reprojection);
    return d->supportedMethods;
}


// *********************************************************************
// *                        Reprojection matrix                        *
// *********************************************************************
void Reprojection::setReprojectionMatrix (const cv::Mat &Q)
{
    Q_D(Reprojection);

    // By default, OpenCV stereo calibration produces reprojection
    // matrix that is 4x4 CV_64F... however, GPU reprojection code
    // requires it to be 4x4 CV_32F. For performance reasons, we do
    // the conversion here
    if (Q.type() == CV_64F) {
        Q.convertTo(d->Q, CV_32F); // Convert: CV_64F -> CV_32F
    } else {
        Q.copyTo(d->Q); // Copy
    }

    emit reprojectionMatrixChanged();
}

const cv::Mat &Reprojection::getReprojectionMatrix () const
{
    Q_D(const Reprojection);
    return d->Q;
}


// *********************************************************************
// *                           Reprojection                            *
// *********************************************************************
void Reprojection::reprojectDisparity (const cv::Mat &disparity, cv::Mat &points) const
{
    Q_D(const Reprojection);

    // Validate reprojection matrix
    if (d->Q.rows != 4 || d->Q.cols != 4) {
        points = cv::Mat();
        return;
    }

    // Filter out negative disparities before reprojection
    cv::Mat filteredDisparity = cv::max(disparity, 0);

    // Choose reprojection method
    switch (d->reprojectionMethod) {
        case MethodOpenCvCpu: {
            // Stock OpenCV method; does not handle ROI offset
            cv::reprojectImageTo3D(filteredDisparity, points, d->Q, false, CV_32F);
            break;
        }
#ifdef HAVE_OPENCV_CUDASTEREO
        case MethodOpenCvCuda: {
            // OpenCV CUDA method; does not handle ROI offset
            cv::cuda::GpuMat gpu_disparity, gpu_points;
            gpu_disparity.upload(filteredDisparity);
            cv::cuda::reprojectImageTo3D(gpu_disparity, gpu_points, d->Q, 3);
            gpu_points.download(points);
            break;
        }
#endif
        default: {
            points = cv::Mat();
            break;
        }
    }
}


} // Pipeline
} // StereoToolbox
} // MVL
