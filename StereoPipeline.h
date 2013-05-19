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

    void setImageSource (ImageSource *);
    void setCalibration (StereoCalibration *);
    void setStereoMethod (StereoMethod *);
    
    const cv::Mat &getLeftImage () const;
    const cv::Mat &getRightImage () const;
    
    const cv::Mat &getLeftRectifiedImage () const;
    const cv::Mat &getRightRectifiedImage () const;
    
    const cv::Mat &getDisparityImage () const;
    int getDisparityImageComputationTime () const;

protected slots:
    void beginProcessing ();

    void rectifyImages ();
    void computeDisparityImage ();

    void methodParameterChanged ();

signals:
    void error (const QString &);

    void inputImagesChanged ();
    void rectifiedImagesChanged ();
    void disparityImageChanged ();

protected:
    // Image source
    ImageSource *imageSource;

    // Stereo calibration & rectification
    StereoCalibration *calibration;
    
    // Stereo method
    StereoMethod *method;
    
    // Cached input images
    cv::Mat inputImageL;
    cv::Mat inputImageR;

    // Cached rectified input images
    cv::Mat rectifiedImageL;
    cv::Mat rectifiedImageR;

    // Cached disparity image
    cv::Mat disparityImage;
    int disparityImageComputationTime;
};

#endif
