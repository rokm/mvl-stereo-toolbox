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


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class PipelinePrivate
{
    Q_DISABLE_COPY(PipelinePrivate)
    Q_DECLARE_PUBLIC(Pipeline)

    Pipeline * const q_ptr;

    PipelinePrivate (Pipeline *parent);

protected:
    // Image pair source
    bool imagePairSourceActive;
    ImagePairSource *imagePairSource;

    // Cached input images
    cv::Mat inputImageL;
    cv::Mat inputImageR;

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
    cv::Mat disparityImage;
    int disparityLevels;
    int disparityImageComputationTime;

    // Disparity visualization
    QList<int> supportedDisparityVisualizationMethods;

    int disparityVisualizationMethod;
    cv::Mat disparityVisualizationImage;

    // Reprojection
    bool reprojectionActive;
    Reprojection *reprojection;

    // Cached reprojected image
    cv::Mat reprojectedImage;
    int reprojectionComputationTime;
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
