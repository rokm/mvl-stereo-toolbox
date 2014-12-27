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
#include <QtConcurrent>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class ImagePairSource;
class Rectification;
class StereoMethod;
class Reprojection;

class MVL_STEREO_PIPELINE_EXPORT Pipeline : public QObject
{
    Q_OBJECT

public:
    Pipeline (QObject * = 0);
    virtual ~Pipeline ();

    // GPU/CUDA management
    int getNumberOfGpuDevices ();

    void setGpuDevice (int);
    int getGpuDevice () const;

    // Image pair source
    void setImagePairSource (ImagePairSource *);
    ImagePairSource *getImagePairSource ();

    void setImagePairSourceState (bool);
    bool getImagePairSourceState () const;

    const cv::Mat &getLeftImage () const;
    const cv::Mat &getRightImage () const;

    // Rectification
    void setRectification (Rectification *);
    Rectification *getRectification ();

    void setRectificationState (bool);
    bool getRectificationState () const;

    const cv::Mat &getLeftRectifiedImage () const;
    const cv::Mat &getRightRectifiedImage () const;
    int getRectificationTime () const;

    // Stereo method
    void setStereoMethod (StereoMethod *);
    StereoMethod *getStereoMethod ();

    void setStereoMethodState (bool);
    bool getStereoMethodState () const;

    const cv::Mat &getDisparityImage () const;
    int getNumberOfDisparityLevels () const;
    int getDisparityImageComputationTime () const;

    // Stereo method thread
    void setUseStereoMethodThread (bool);
    bool getUseStereoMethodThread () const;

    int getStereoDroppedFrames () const;

    // Disparity visualization
    enum DisparityVisualization {
        DisparityVisualizationNone,
        DisparityVisualizationGrayscale,
        DisparityVisualizationColorCuda,
        DisparityVisualizationColorCpu
    };

    void setDisparityVisualizationMethod (int);
    int getDisparityVisualizationMethod () const;
    const QList<int> &getSupportedDisparityVisualizationMethods () const;

    const cv::Mat &getDisparityVisualizationImage () const;

    // Reprojection
    void setReprojection (Reprojection *);
    Reprojection *getReprojection ();

    void setReprojectionState (bool);
    bool getReprojectionState () const;

    const cv::Mat &getReprojectedImage () const;
    int getReprojectionComputationTime () const;

// NOTE: we need the old signal/slot syntax here!
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
