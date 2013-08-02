/*
 * OpenCV Semi-Global Block Matching: method
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
 
#ifndef STEREO_METHOD_SEMI_GLOBAL_BLOCK_MATCHING_H
#define STEREO_METHOD_SEMI_GLOBAL_BLOCK_MATCHING_H

#include <QtCore>
#include <opencv2/calib3d/calib3d.hpp>

#include <stereo_method.h>


namespace StereoMethodSemiGlobalBlockMatching {

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

    // Generic parameter setting
    template <typename T> void setParameter (T &parameter, const T &newValue) {
        // Set only if necessary
        if (parameter != newValue) {
            QMutexLocker locker(&mutex);
            parameter = newValue;
            locker.unlock();
            
            emit parameterChanged();
        }
    }
    
    // Parameters
    int getMinDisparity () const;
    int getNumDisparities () const;
    int getSADWindowSize () const;

    int getPreFilterCap () const;
    int getUniquenessRatio () const;

    int getP1 () const;
    int getP2 () const;

    int getSpeckleWindowSize () const;
    int getSpeckleRange () const;

    int getDisp12MaxDiff () const;

    bool getFullDP () const;

    enum {
        OpenCV,
        StereoMatch,
    } PresetType;

public slots:
    void usePreset (int);
    
    void setMinDisparity (int);
    void setNumDisparities (int);   
    void setSADWindowSize (int);
    
    void setPreFilterCap (int);
    void setUniquenessRatio (int);
    
    void setP1 (int);
    void setP2 (int);
    
    void setSpeckleWindowSize (int);
    void setSpeckleRange (int);
    
    void setDisp12MaxDiff (int);

    void setFullDP (bool);

signals:
    // Signals from interface
    void parameterChanged ();

protected:
    // Semi-global block matcher
    cv::StereoSGBM sgbm;
    QMutex mutex;

    int imageWidth;
    int imageChannels;

    cv::Mat tmpDisparity;
};

}

#endif
