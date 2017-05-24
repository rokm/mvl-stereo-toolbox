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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__PIPELINE_H
#define MVL_STEREO_TOOLBOX__PIPELINE__PIPELINE_H

#include <stereo-pipeline/export.h>

#include <QtCore>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class ImagePairSource;
class Rectification;
class StereoMethod;
class DisparityVisualization;
class Reprojection;

class PipelinePrivate;

class MVL_STEREO_PIPELINE_EXPORT Pipeline : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Pipeline)
    Q_DECLARE_PRIVATE(Pipeline)
    QScopedPointer<PipelinePrivate> const d_ptr;

public:
    Pipeline (QObject *parent = 0);
    virtual ~Pipeline ();

    // GPU/CUDA management
    int getNumberOfGpuDevices ();

    void setGpuDevice (int dev);
    int getGpuDevice () const;


    // Image pair source
    void setImagePairSource (QObject *source);
    QObject *getImagePairSource ();

    void setImagePairSourceState (bool active);
    bool getImagePairSourceState () const;

    cv::Mat getLeftImage () const;
    cv::Mat getRightImage () const;


    // Rectification
    Rectification *getRectification ();

    void setRectificationState (bool active);
    bool getRectificationState () const;

    cv::Mat getLeftRectifiedImage () const;
    cv::Mat getRightRectifiedImage () const;

    int getRectificationTime () const;
    int getRectificationDroppedFrames () const;


    // Stereo method
    void setStereoMethod (QObject *method);
    QObject *getStereoMethod ();

    void loadStereoMethodParameters (const QString &filename);
    void saveStereoMethodParameters (const QString &filename);

    void setStereoMethodState (bool active);
    bool getStereoMethodState () const;

    cv::Mat getDisparity () const;
    void getDisparity (cv::Mat &disparity, int &numDisparityLevels) const;

    int getStereoMethodTime () const;
    int getStereoMethodDroppedFrames () const;


    // Disparity visualization
    DisparityVisualization *getVisualization ();

    void setVisualizationState (bool active);
    bool getVisualizationState () const;

    int getVisualizationTime () const;
    int getVisualizationDroppedFrames () const;

    cv::Mat getDisparityVisualization () const;

    // Reprojection
    Reprojection *getReprojection ();

    void setReprojectionState (bool active);
    bool getReprojectionState () const;

    int getReprojectionTime () const;
    int getReprojectionDroppedFrames () const;

    cv::Mat getPoints () const;

#if 0
    // FIXME

    const cv::Mat &getPointCloudXyz () const;
    const cv::Mat &getPointCloudRgb () const;
#endif


    // Error types
    enum {
        ErrorGeneral,
        ErrorImagePairSource,
        ErrorRectification,
        ErrorStereoMethod,
        ErrorVisualization,
        ErrorReprojection,
    };

signals:
    void inputImagesChanged (cv::Mat imgL, cv::Mat imgR);
    void rectifiedImagesChanged (cv::Mat imgL, cv::Mat imgR);
    void disparityChanged (cv::Mat disparity);
    void pointsChanged (cv::Mat points);
    void visualizationChanged (cv::Mat visualization);

signals:
    void error (int domain, const QString &message);

    void processingCompleted ();

    void imagePairSourceStateChanged (bool active);
    void rectificationStateChanged (bool active);
    void stereoMethodStateChanged (bool active);
    void visualizationStateChanged (bool active);
    void reprojectionStateChanged (bool active);

    friend PipelinePrivate;
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
