/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#ifndef STEREO_PIPELINE_H
#define STEREO_PIPELINE_H

#include <QtCore>

#include <opencv2/core/core.hpp>

class ImageSource;
class StereoCalibration;
class StereoMethod;

class StereoPipeline : public QObject
{
    Q_OBJECT
    
public:
    StereoPipeline (QObject * = 0);
    virtual ~StereoPipeline ();

    // Image source
    void setImageSource (ImageSource *);

    void setImageSourceState (bool);
    bool getImageSourceState () const;

    const cv::Mat &getLeftImage () const;
    const cv::Mat &getRightImage () const;
    
    // Calibration
    void setCalibration (StereoCalibration *);

    void setCalibrationState (bool);
    bool getCalibrationState () const;

    int getRectificationTime () const;

    const cv::Mat &getLeftRectifiedImage () const;
    const cv::Mat &getRightRectifiedImage () const;
        
    // Stereo method
    void setStereoMethod (StereoMethod *);

    void setStereoMethodState (bool);
    bool getStereoMethodState () const;
    
    const cv::Mat &getDisparityImage () const;
    int getNumberOfDisparityLevels () const;
    int getDisparityImageComputationTime () const;

protected slots:
    void beginProcessing ();
    void rectifyImages ();
    void computeDisparityImage ();
    
signals:
    void error (const QString &);

    void imageSourceStateChanged (bool);
    void calibrationStateChanged (bool);
    void stereoMethodStateChanged (bool);

    void inputImagesChanged ();
    void rectifiedImagesChanged ();
    void disparityImageChanged ();

protected:
    // Image source
    bool imageSourceActive;
    ImageSource *imageSource;

    // Cached input images
    cv::Mat inputImageL;
    cv::Mat inputImageR;


    // Stereo calibration & rectification
    bool calibrationActive;
    StereoCalibration *calibration;

    // Cached rectified input images
    cv::Mat rectifiedImageL;
    cv::Mat rectifiedImageR;
    int rectificationTime;


    // Stereo method
    bool stereoMethodActive;
    StereoMethod *stereoMethod;
    
    // Cached disparity image
    cv::Mat disparityImage;
    int disparityLevels;
    int disparityImageComputationTime;
};

#endif
