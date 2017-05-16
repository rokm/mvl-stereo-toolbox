/*
 * Stereo Pipeline: pipeline
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__PIPELINE_P_H
#define MVL_STEREO_TOOLBOX__PIPELINE__PIPELINE_P_H

#include <stereo-pipeline/export.h>

#include <QtCore>
#include <QtConcurrent>
#include <opencv2/core.hpp>


Q_DECLARE_METATYPE(cv::Mat);


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {

namespace AsyncPipeline {
    class SourceElement;
}


class PipelinePrivate
{
    Q_DISABLE_COPY(PipelinePrivate)
    Q_DECLARE_PUBLIC(Pipeline)

    Pipeline * const q_ptr;

    PipelinePrivate (Pipeline *parent);

protected:
    // Image pair source
    AsyncPipeline::SourceElement *source;
    cv::Mat inputImageL, inputImageR;

    // Stereo rectification
    bool rectificationActive;
    Rectification *rectification;

    // Cached rectified input images
    cv::Mat rectifiedImageL;
    cv::Mat rectifiedImageR;
    int rectificationTime;

    // Stereo method
    bool stereoMethodActive;
    StereoMethod *stereoMethod;

    bool useStereoMethodThread;
    QFutureWatcher<void> stereoMethodWatcher;
    int stereoDroppedFramesCounter;

    // Cached disparity image
    cv::Mat disparity;
    int disparityLevels;
    int disparityTime;

    // Disparity visualization
    DisparityVisualization *visualization;
    bool visualizationActive;
    int visualizationTime;

    cv::Mat disparityVisualization;

    // Reprojection
    bool reprojectionActive;
    Reprojection *reprojection;

    // Cached point cloud
    cv::Mat pointCloudRgb;
    cv::Mat pointCloudXyz;
    int reprojectionTime;
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
