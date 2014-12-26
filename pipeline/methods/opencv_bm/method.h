/*
 * OpenCV Block Matching: method
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
 
#ifndef STEREO_METHOD_BLOCK_MATCHING_H
#define STEREO_METHOD_BLOCK_MATCHING_H

#include <stereo_method.h>

#include <opencv2/calib3d.hpp>


namespace StereoMethodBlockMatching {
    
class Method : public QObject, public StereoMethod
{
    Q_OBJECT
    Q_INTERFACES(StereoMethod)

public:
    Method (QObject * = 0);
    virtual ~Method ();

    virtual QString getShortName () const;
    virtual QWidget *createConfigWidget (QWidget * = 0);
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &, int &);
    virtual void loadParameters (const QString &);
    virtual void saveParameters (const QString &) const;

    // Parameters
    int getPreFilterType () const;
    int getPreFilterSize () const;
    int getPreFilterCap () const;

    int getSADWindowSize () const;
    int getMinDisparity () const;
    int getNumDisparities () const;

    int getTextureThreshold () const;
    int getUniquenessRatio () const;
    int getSpeckleWindowSize () const;
    int getSpeckleRange () const;

    int getDisp12MaxDiff () const;

    enum {
        OpenCV,
        StereoMatch,
    } PresetType;

public slots:
    void usePreset (int type);

    void setPreFilterType (int);
    void setPreFilterSize (int);
    void setPreFilterCap (int);
        
    void setSADWindowSize (int);
    void setMinDisparity (int);
    void setNumDisparities (int);
    
    
    void setTextureThreshold (int);
    void setUniquenessRatio (int);
    void setSpeckleWindowSize (int);
    void setSpeckleRange (int);
        
    void setDisp12MaxDiff (int);

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

};

#endif
