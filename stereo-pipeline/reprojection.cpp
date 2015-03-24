/*
 * Stereo Pipeline: reprojection
 * Copyright (C) 2013-2015 Rok Mandeljc
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
#ifdef HAVE_CUDA
#include <cuda_runtime.h>
#endif
#endif


#include "reprojection_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


// Forward-declarations for toolbox-modified methods
void reprojectDisparityImage (const cv::Mat &, cv::Mat &, const cv::Mat &, int, int);

#ifdef HAVE_OPENCV_CUDASTEREO
#ifdef HAVE_CUDA
template <typename TYPE> void reprojectDisparityImageCuda (const cv::cuda::PtrStepSzb, cv::cuda::PtrStepSz<float3>, const float *, unsigned short, unsigned short);
#endif
#endif


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
void Reprojection::setReprojectionMethod (int newMethod)
{
    Q_D(Reprojection);

    if (newMethod == d->reprojectionMethod) {
        return;
    }

    // Make sure method is supported
    if (!d->supportedMethods.contains(newMethod)) {
        d->reprojectionMethod = MethodToolboxCpu;
        emit error(QString("Reprojection method %1 not supported!").arg(newMethod));
    } else {
        d->reprojectionMethod = newMethod;
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
void Reprojection::setReprojectionMatrix (const cv::Mat &newQ)
{
    Q_D(Reprojection);

    // By default, OpenCV stereo calibration produces reprojection
    // matrix that is 4x4 CV_64F... however, GPU reprojection code
    // requires it to be 4x4 CV_32F. For performance reasons, we do
    // the conversion here
    if (newQ.type() == CV_64F) {
        newQ.convertTo(d->Q, CV_32F); // Convert: CV_64F -> CV_32F
    } else {
        d->Q = newQ.clone(); // Copy
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
void Reprojection::reprojectStereoDisparity (const cv::Mat &disparity_, cv::Mat &points, int offsetX, int offsetY) const
{
    Q_D(const Reprojection);

    // Validate reprojection matrix
    if (d->Q.rows != 4 || d->Q.cols != 4) {
        points = cv::Mat();
        return;
    }

    // Filter out negative disparities before reprojection
    cv::Mat disparity = max(disparity_, 0);

    // Choose reprojection method
    switch (d->reprojectionMethod) {
        case MethodToolboxCpu: {
            // Toolbox-modified method; handles ROI offset
            reprojectDisparityImage(disparity, points, d->Q, offsetX, offsetY);
            break;
        }
        case MethodOpenCvCpu: {
            // Stock OpenCV method; does not handle ROI offset
            cv::reprojectImageTo3D(disparity, points, d->Q, false, CV_32F);
            break;
        }
#ifdef HAVE_OPENCV_CUDASTEREO
        case MethodOpenCvCuda: {
            // OpenCV CUDA method; does not handle ROI offset
            cv::cuda::GpuMat gpu_disparity, gpu_points;
            gpu_disparity.upload(disparity);
            cv::cuda::reprojectImageTo3D(gpu_disparity, gpu_points, d->Q, 3);
            gpu_points.download(points);
            break;
        }
#ifdef HAVE_CUDA
        case MethodToolboxCuda: {
            // Toolbox-modified CUDA method; handles ROI offset
            cv::cuda::GpuMat gpu_disparity, gpu_points;
            gpu_disparity.upload(disparity);
            gpu_points.create(disparity.size(), CV_32FC3);
            switch (disparity.type()) {
                case CV_8UC1: {
                    reprojectDisparityImageCuda<unsigned char>(gpu_disparity, gpu_points, d->Q.ptr<float>(), offsetX, offsetY);
                    break;
                }
                case CV_16SC1: {
                    reprojectDisparityImageCuda<short>(gpu_disparity, gpu_points, d->Q.ptr<float>(), offsetX, offsetY);
                    break;
                }
                case CV_32SC1: {
                    reprojectDisparityImageCuda<int>(gpu_disparity, gpu_points, d->Q.ptr<float>(), offsetX, offsetY);
                    break;
                }
                case CV_32FC1: {
                    reprojectDisparityImageCuda<float>(gpu_disparity, gpu_points, d->Q.ptr<float>(), offsetX, offsetY);
                    break;
                }
                default: {
                    throw QString("Unhandled disparity format %1!").arg(disparity.type());
                }
            }
            gpu_points.download(points);
            break;
        }
#endif
#endif
        default: {
            points = cv::Mat();
            break;
        }
    }
}


// *********************************************************************
// *                      Reprojection functions                       *
// *********************************************************************
// We use modified versions of OpenCV reprojectImageTo3D functions; the
// original ones do not support setting x and y coordinate offset, which
// we need to properly support ROIs.
template <typename TYPE>
void __reprojectDisparityImage (const cv::Mat &disparity, cv::Mat &points, const cv::Mat &Q, int offsetX, int offsetY)
{
    points.create(disparity.size(), CV_32FC3);

    // Get Q coefficients
    float q[4][4];
    Q.convertTo(cv::Mat(4, 4, CV_32F, q), CV_32F);

    float qx, qy, qz, qw, d, iw;

    // Go over all
    for (int yi = 0; yi < disparity.rows; yi++) {
        const TYPE *disp_ptr = disparity.ptr<TYPE>(yi);
        cv::Vec3f *point_ptr = points.ptr<cv::Vec3f>(yi);

        int y = yi + offsetY;

        qx = q[0][1]*y + q[0][3];
        qy = q[1][1]*y + q[1][3];
        qz = q[2][1]*y + q[2][3];
        qw = q[3][1]*y + q[3][3];

        for (int xi = 0; xi < disparity.cols; xi++) {
            int x = xi + offsetX;

            d = disp_ptr[xi];

            iw = 1.0 / (q[3][0]*x + qw + q[3][2]*d);

            point_ptr[xi][0] = (q[0][0]*x + qx + q[0][2]*d)*iw;
            point_ptr[xi][1] = (q[1][0]*x + qy + q[1][2]*d)*iw;
            point_ptr[xi][2] = (q[2][0]*x + qz + q[2][2]*d)*iw;
        }
    }
}

void reprojectDisparityImage (const cv::Mat &disparity, cv::Mat &points, const cv::Mat &Q, int offsetX, int offsetY)
{
    switch (disparity.type()) {
        case CV_8UC1: {
            return __reprojectDisparityImage<unsigned char>(disparity, points, Q, offsetX, offsetY);
        }
        case CV_16SC1: {
            return __reprojectDisparityImage<short>(disparity, points, Q, offsetX, offsetY);
        }
        case CV_32SC1: {
            return __reprojectDisparityImage<int>(disparity, points, Q, offsetX, offsetY);
        }
        case CV_32FC1: {
            return __reprojectDisparityImage<float>(disparity, points, Q, offsetX, offsetY);
        }
        default: {
            throw QString("Unhandled disparity format %1!").arg(disparity.type());
        }
    }
}


} // Pipeline
} // StereoToolbox
} // MVL
