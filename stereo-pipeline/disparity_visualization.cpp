/*
 * Stereo Pipeline: disparity visualization
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

#include <stereo-pipeline/disparity_visualization.h>
#include <stereo-pipeline/utils.h>

#include <opencv2/opencv_modules.hpp>

#ifdef HAVE_OPENCV_CUDASTEREO
#include <opencv2/cudastereo.hpp>
#endif


#include "disparity_visualization_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


DisparityVisualizationPrivate::DisparityVisualizationPrivate (DisparityVisualization *parent)
    : q_ptr(parent)
{
    // Create list of supported methods
    supportedMethods.append(DisparityVisualization::MethodGrayscale);
#ifdef HAVE_OPENCV_CUDASTEREO
    try {
        if (cv::cuda::getCudaEnabledDeviceCount()) {
            supportedMethods.append(DisparityVisualization::MethodColorCuda);
        }
    } catch (...) {
        // Nothing to do :)
    }
#endif
    supportedMethods.append(DisparityVisualization::MethodColorCpu);

    // Default method: OpenCV CPU
    method = DisparityVisualization::MethodGrayscale;
}


DisparityVisualization::DisparityVisualization (QObject *parent)
    : QObject(parent), d_ptr(new DisparityVisualizationPrivate(this))
{
}

DisparityVisualization::~DisparityVisualization ()
{
}


// *********************************************************************
// *                       Visualization method                        *
// *********************************************************************
void DisparityVisualization::setVisualizationMethod (int method)
{
    Q_D(DisparityVisualization);

    if (method == d->method) {
        return;
    }

    // Make sure method is supported
    if (!d->supportedMethods.contains(method)) {
        emit error(QString("Reprojection method %1 not supported!").arg(method));
    } else {
        d->method = method;
    }

    // Emit in any case
    emit visualizationMethodChanged(d->method);
}


int DisparityVisualization::getVisualizationMethod () const
{
    Q_D(const DisparityVisualization);
    return d->method;
}


const QList<int> &DisparityVisualization::getSupportedVisualizationMethods () const
{
    Q_D(const DisparityVisualization);
    return d->supportedMethods;
}


// *********************************************************************
// *                           Visualization                           *
// *********************************************************************
void DisparityVisualization::visualizeDisparity (const cv::Mat &disparity, int disparityLevels, cv::Mat &visualization) const
{
    Q_D(const DisparityVisualization);

    switch (d->method) {
        case MethodGrayscale: {
            // Raw grayscale disparity
            disparity.convertTo(visualization, CV_8U, 255.0/disparityLevels);
            break;
        }
#ifdef HAVE_OPENCV_CUDASTEREO
        case MethodColorCuda: {
            try {
                // Hue-color-coded disparity
                cv::cuda::GpuMat gpu_disp(disparity);
                cv::cuda::GpuMat gpu_disp_color;
                cv::Mat disp_color;

                cv::cuda::drawColorDisp(gpu_disp, gpu_disp_color, disparityLevels);
                gpu_disp_color.download(visualization);
            } catch (...) {
                // The above calls can fail
                visualization = cv::Mat();
            }

            break;
        }
#endif
        case MethodColorCpu :{
            Utils::createColorCodedDisparityCpu(disparity, visualization, disparityLevels);
            break;
        }
        default: {
            visualization = cv::Mat();
            break;
        }
    }
}


} // Pipeline
} // StereoToolbox
} // MVL
