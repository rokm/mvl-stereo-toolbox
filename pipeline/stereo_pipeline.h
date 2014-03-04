/*
 * Stereo Pipeline: pipeline
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

#ifndef STEREO_PIPELINE_H
#define STEREO_PIPELINE_H

#include "mvl_stereo_pipeline_export.h"

#include <QtCore>
#include <QtConcurrent>
#include <opencv2/core/core.hpp>


class ImagePairSource;
class StereoRectification;
class StereoMethod;
class StereoReprojection;

class MVL_STEREO_PIPELINE_EXPORT StereoPipeline : public QObject
{
    Q_OBJECT

public:
    StereoPipeline (QObject * = 0);
    virtual ~StereoPipeline ();

    // GPU/CUDA management
    int getNumberOfGpuDevices ();
    void setGpuDevice (int);
    int getGpuDevice () const;

    // Image pair source
    void setImagePairSource (ImagePairSource *);
    ImagePairSource *getImagePairSource ();

    bool getImagePairSourceState () const;

    const cv::Mat &getLeftImage () const;
    const cv::Mat &getRightImage () const;

    // Rectification
    void setRectification (StereoRectification *);
    StereoRectification *getRectification ();

    bool getRectificationState () const;

    const cv::Mat &getLeftRectifiedImage () const;
    const cv::Mat &getRightRectifiedImage () const;
    int getRectificationTime () const;

    // Stereo method
    void setStereoMethod (StereoMethod *);
    StereoMethod *getStereoMethod ();

    bool getStereoMethodState () const;

    const cv::Mat &getDisparityImage () const;
    int getNumberOfDisparityLevels () const;
    int getDisparityImageComputationTime () const;

    // Stereo method thread
    bool getUseStereoMethodThread () const;

    int getStereoDroppedFrames () const;

    // Disparity visualization
    enum DisparityVisualization {
        DisparityVisualizationNone,
        DisparityVisualizationGrayscale,
        DisparityVisualizationColorGpu,
    };

    int getDisparityVisualizationMethod () const;
    const QList<int> &getSupportedDisparityVisualizationMethods () const;

    const cv::Mat &getDisparityVisualizationImage () const;

    // Reprojection
    void setReprojection (StereoReprojection *);
    StereoReprojection *getReprojection ();

    bool getReprojectionState () const;

    const cv::Mat &getReprojectedImage () const;
    int getReprojectionComputationTime () const;

public slots:
    void setImagePairSourceState (bool);
    void setRectificationState (bool);
    void setStereoMethodState (bool);
    void setReprojectionState (bool);

    void setUseStereoMethodThread (bool);
    void setDisparityVisualizationMethod (int);

protected slots:
    void beginProcessing ();
    void rectifyImages ();
    void computeDisparityImage ();
    void computeDisparityImageVisualization ();
    void reprojectDisparityImage ();

    void computeDisparityImageInThread ();

    void updateReprojectionMatrix ();

signals:
    void error (const QString);
    void processingCompleted ();

    void imagePairSourceStateChanged (bool);
    void rectificationStateChanged (bool);
    void stereoMethodStateChanged (bool);
    void reprojectionStateChanged (bool);

    void inputImagesChanged ();
    void rectifiedImagesChanged ();
    void disparityImageChanged ();
    void disparityVisualizationImageChanged ();
    void reprojectedImageChanged ();

    void disparityVisualizationMethodChanged (int);

protected:
    // Image pair source
    bool imagePairSourceActive;
    ImagePairSource *imagePairSource;

    // Cached input images
    cv::Mat inputImageL;
    cv::Mat inputImageR;

    // Stereo rectification
    bool rectificationActive;
    StereoRectification *rectification;

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
    StereoReprojection *reprojection;

    // Cached reprojected image
    cv::Mat reprojectedImage;
    int reprojectionComputationTime;
};

#endif
