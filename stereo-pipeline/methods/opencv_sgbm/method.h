/*
 * OpenCV Semi-Global Block Matching: method
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__METHODS__OPENCV_SGBM__METHOD_H
#define MVL_STEREO_TOOLBOX__PIPELINE__METHODS__OPENCV_SGBM__METHOD_H

#include <QtCore>
#include <opencv2/calib3d.hpp>

#include <stereo-pipeline/stereo_method.h>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodOpenCvSgbm {


class Method : public QObject, public StereoMethod
{
    Q_OBJECT
    Q_INTERFACES(MVL::StereoToolbox::Pipeline::StereoMethod)

public:
    Method (QObject * = 0);
    virtual ~Method ();

    virtual QString getShortName () const;
    virtual QWidget *createConfigWidget (QWidget * = 0);
    virtual void computeDisparity (const cv::Mat &, const cv::Mat &, cv::Mat &, int &);
    virtual void loadParameters (const QString &);
    virtual void saveParameters (const QString &) const;

    // Parameters
    enum {
        OpenCV,
        StereoMatch,
    } PresetType;

    void usePreset (int);

    void setMinDisparity (int);
    int getMinDisparity () const;

    void setNumDisparities (int);
    int getNumDisparities () const;

    void setSADWindowSize (int);
    int getSADWindowSize () const;


    void setPreFilterCap (int);
    int getPreFilterCap () const;

    void setUniquenessRatio (int);
    int getUniquenessRatio () const;


    void setP1 (int);
    int getP1 () const;

    void setP2 (int);
    int getP2 () const;


    void setSpeckleWindowSize (int);
    int getSpeckleWindowSize () const;

    void setSpeckleRange (int);
    int getSpeckleRange () const;


    void setDisp12MaxDiff (int);
    int getDisp12MaxDiff () const;


    void setMode (int);
    int getMode () const;

signals:
    // Signals from interface
    void parameterChanged ();

protected:
    // Method implementation
    cv::Ptr<cv::StereoSGBM> sgbm;
    QMutex mutex;

    int imageWidth;
    int imageChannels;

    cv::Mat tmpDisparity;
};


} // StereoMethodOpenCvSgbm
} // Pipeline
} // StereoToolbox
} // MVL


#endif
