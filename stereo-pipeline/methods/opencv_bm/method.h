/*
 * OpenCV Block Matching: method
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__METHODS__OPENCV_BM__METHOD_H
#define MVL_STEREO_TOOLBOX__PIPELINE__METHODS__OPENCV_BM__METHOD_H

#include <stereo-pipeline/stereo_method.h>

#include <opencv2/calib3d.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodOpenCvBm {


class Method : public QObject, public StereoMethod
{
    Q_OBJECT
    Q_INTERFACES(MVL::StereoToolbox::Pipeline::StereoMethod)

public:
    Method (QObject * = 0);
    virtual ~Method ();

    virtual QString getShortName () const;
    virtual QWidget *createConfigWidget (QWidget * = 0);
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &, int &);
    virtual void loadParameters (const QString &);
    virtual void saveParameters (const QString &) const;

    // Parameters
    enum {
        OpenCV,
        StereoMatch,
    } PresetType;

    void usePreset (int type);

    void setPreFilterType (int);
    int getPreFilterType () const;

    void setPreFilterSize (int);
    int getPreFilterSize () const;

    void setPreFilterCap (int);
    int getPreFilterCap () const;


    void setSADWindowSize (int);
    int getSADWindowSize () const;

    void setMinDisparity (int);
    int getMinDisparity () const;

    void setNumDisparities (int);
    int getNumDisparities () const;


    void setTextureThreshold (int);
    int getTextureThreshold () const;

    void setUniquenessRatio (int);
    int getUniquenessRatio () const;

    void setSpeckleWindowSize (int);
    int getSpeckleWindowSize () const;

    void setSpeckleRange (int);
    int getSpeckleRange () const;


    void setDisp12MaxDiff (int);
    int getDisp12MaxDiff () const;

signals:
    // Signals from interface
    void parameterChanged ();

protected:
    // Block matcher
    cv::Ptr<cv::StereoBM> bm;
    QMutex mutex;

    int imageWidth;

    cv::Mat tmpImg1, tmpImg2;
    cv::Mat tmpDisparity;
};


} // StereoMethodOpenCvBm
} // Pipeline
} // StereoToolbox
} // MVL


#endif
