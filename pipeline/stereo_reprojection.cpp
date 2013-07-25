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

#include "stereo_reprojection.h"

#include <opencv2/calib3d/calib3d.hpp>

#ifdef HAVE_OPENCV_GPU
#include <opencv2/gpu/gpu.hpp>
#ifdef HAVE_CUDA
#include <cuda_runtime.h>
#endif
#endif


// Forward-declarations for toolbox-modified methods
void reprojectDisparityImage (const cv::Mat &, cv::Mat &, const cv::Mat &, int, int);
#ifdef HAVE_OPENCV_GPU
#ifdef HAVE_CUDA
void reprojectDisparityImageGpu (const cv::gpu::PtrStepSz<unsigned char>, cv::gpu::PtrStepSz<float3>, const float *, unsigned short, unsigned short);
#endif
#endif


StereoReprojection::StereoReprojection (QObject *parent)
{
    // Create list of supported methods
    supportedMethods.append(ToolboxCpu);
    supportedMethods.append(OpenCvCpu);
#ifdef HAVE_OPENCV_GPU
    try {
        if (cv::gpu::getCudaEnabledDeviceCount()) {
#ifdef HAVE_CUDA
            supportedMethods.append(ToolboxGpu);
#endif
            supportedMethods.append(OpenCvGpu);
        }
    } catch (...) {
        // Nothing to do :)
    }
#endif

    // Default method: Toolbox CPU    
    reprojectionMethod = ToolboxCpu;
}

StereoReprojection::~StereoReprojection ()
{
}


// *********************************************************************
// *                        Reprojection method                        *
// *********************************************************************
void StereoReprojection::setReprojectionMethod (int newMethod)
{
    if (newMethod == reprojectionMethod) {
        return;
    }

    // Make sure method is supported
    if (!supportedMethods.contains(newMethod)) {
        reprojectionMethod = ToolboxCpu;
        emit error(QString("Reprojection method %1 not supported!").arg(newMethod));
    } else {
        reprojectionMethod = newMethod;
    }

    // Emit in any case
    emit reprojectionMethodChanged(reprojectionMethod);
}


int StereoReprojection::getReprojectionMethod () const
{
    return reprojectionMethod;
}


const QList<int> &StereoReprojection::getSupportedReprojectionMethods () const
{
    return supportedMethods;
}


// *********************************************************************
// *                        Reprojection matrix                        *
// *********************************************************************
void StereoReprojection::setReprojectionMatrix (const cv::Mat &newQ)
{
    // By default, OpenCV stereo calibration produces reprojection
    // matrix that is 4x4 CV_64F... however, GPU reprojection code
    // requires it to be 4x4 CV_32F. For performance reasons, we do
    // the conversion here
    if (newQ.type() == CV_64F) {
        newQ.convertTo(Q, CV_32F); // Convert: CV_64F -> CV_32F
    } else {
        Q = newQ.clone(); // Copy
    }
    
    emit reprojectionMatrixChanged();
}

const cv::Mat &StereoReprojection::getReprojectionMatrix () const
{
    return Q;
}


// *********************************************************************
// *                           Reprojection                            *
// *********************************************************************
void StereoReprojection::reprojectStereoDisparity (const cv::Mat &disparity, cv::Mat &points, int offsetX, int offsetY) const
{
    // Validate reprojection matrix
    if (Q.rows != 4 || Q.cols != 4) {
        points = cv::Mat();
        return;
    }

    switch (reprojectionMethod) {
        case ToolboxCpu: {
            // Toolbox-modified method; handles ROI offset
            reprojectDisparityImage(disparity, points, Q, offsetX, offsetY);
            break;
        }
        case OpenCvCpu: {
            // Stock OpenCV method; does not handle ROI offset
            cv::reprojectImageTo3D(disparity, points, Q, false, CV_32F);
            break;
        }
#ifdef HAVE_OPENCV_GPU
        case OpenCvGpu: {
            // OpenCV GPU method; does not handle ROI offset
            cv::gpu::GpuMat gpu_disparity, gpu_points;
            gpu_disparity.upload(disparity);
            cv::gpu::reprojectImageTo3D(gpu_disparity, gpu_points, Q, 3);
            gpu_points.download(points);
            break;
        }
#ifdef HAVE_CUDA
        case ToolboxGpu: {
            // Toolbox-modified GPU method; handles ROI offset
            cv::gpu::GpuMat gpu_disparity, gpu_points;
            gpu_disparity.upload(disparity);
            gpu_points.create(disparity.size(), CV_32FC3);
            reprojectDisparityImageGpu(gpu_disparity, gpu_points, Q.ptr<float>(), offsetX, offsetY);
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
void reprojectDisparityImage (const cv::Mat &disparity, cv::Mat &points, const cv::Mat &Q, int offsetX, int offsetY)
{
    points.create(disparity.size(), CV_32FC3);

    // Get Q coefficients
    float q[4][4];
    Q.convertTo(cv::Mat(4, 4, CV_32F, q), CV_32F);

    float qx, qy, qz, qw, d, iw;

    // Go over all
    for (int yi = 0; yi < disparity.rows; yi++) {
        const unsigned char *disp_ptr = disparity.ptr<unsigned char>(yi);
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

