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
    void getImages (cv::Mat &imageLeft, cv::Mat &imageRight) const;

    int getImageCaptureTime () const;
    int getImageCaptureDroppedFrames () const;
    float getImageCaptureFramerate () const;

    void setImageCaptureFramerateLimit (double limit);
    double getImageCaptureFramerateLimit () const;

    // Rectification
    Rectification *getRectification ();

    void setRectificationState (bool active);
    bool getRectificationState () const;

    cv::Mat getLeftRectifiedImage () const;
    cv::Mat getRightRectifiedImage () const;
    void getRectifiedImages (cv::Mat &imageLeft, cv::Mat &imageRight) const;

    int getRectificationTime () const;
    int getRectificationDroppedFrames () const;
    float getRectificationFramerate () const;


    // Stereo method
    void setStereoMethod (QObject *method);
    QObject *getStereoMethod ();

    void loadStereoMethodParameters (const QString &filename);
    void saveStereoMethodParameters (const QString &filename);

    void setStereoMethodState (bool active);
    bool getStereoMethodState () const;

    cv::Mat getDisparity () const;
    void getDisparity (cv::Mat &disparity) const;
    void getDisparity (cv::Mat &disparity, int &numDisparityLevels) const;

    int getStereoMethodTime () const;
    int getStereoMethodDroppedFrames () const;
    float getStereoMethodFramerate () const;

    // Disparity visualization
    DisparityVisualization *getVisualization ();

    void setVisualizationState (bool active);
    bool getVisualizationState () const;

    cv::Mat getDisparityVisualization () const;
    void getDisparityVisualization (cv::Mat &image) const;

    int getVisualizationTime () const;
    int getVisualizationDroppedFrames () const;
    float getVisualizationFramerate () const;

    // Reprojection
    Reprojection *getReprojection ();

    void setReprojectionState (bool active);
    bool getReprojectionState () const;

    cv::Mat getPoints () const;
    void getPoints (cv::Mat &points) const;

    int getReprojectionTime () const;
    int getReprojectionDroppedFrames () const;
    float getReprojectionFramerate () const;


    // Error types
    enum {
        ErrorGeneral,
        ErrorImagePairSource,
        ErrorRectification,
        ErrorStereoMethod,
        ErrorVisualization,
        ErrorReprojection,
    };

protected:
    // Processing steps
    void rectifyImages ();
    void computeDisparity ();
    void reprojectPoints ();
    void visualizeDisparity ();

signals:
    void inputImagesChanged ();
    void rectifiedImagesChanged ();
    void disparityChanged ();
    void pointsChanged ();
    void visualizationChanged ();

    void imageCaptureFramerateLimitChanged (double limit);

    void imageCaptureFrameDropped (int count);
    void rectificationFrameDropped (int count);
    void stereoMethodFrameDropped (int count);
    void visualizationFrameDropped (int count);
    void reprojectionFrameDropped (int count);

    void imageCaptureFramerateUpdated (float fps);
    void rectificationFramerateUpdated (float fps);
    void stereoMethodFramerateUpdated (float fps);
    void visualizationFramerateUpdated (float fps);
    void reprojectionFramerateUpdated (float fps);

signals:
    void error (int domain, const QString &message);

    void processingCompleted ();

    void imagePairSourceStateChanged (bool active);
    void rectificationStateChanged (bool active);
    void stereoMethodStateChanged (bool active);
    void visualizationStateChanged (bool active);
    void reprojectionStateChanged (bool active);
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
