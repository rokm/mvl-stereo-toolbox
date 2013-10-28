/*
 * OpenCV GPU Block Matching: method
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
 
#ifndef STEREO_METHOD_BLOCK_MATCHING_GPU_H
#define STEREO_METHOD_BLOCK_MATCHING_GPU_H

#include <stereo_method.h>

#include <opencv2/gpu/gpu.hpp>


namespace StereoMethodBlockMatchingGPU {
    
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
    int getPreset () const;
    int getNumDisparities () const;
    int getWindowSize () const;

    double getAverageTextureThreshold () const;

public slots:
    void resetToDefaults ();

    void setPreset (int);
    void setNumDisparities (int);
    void setWindowSize (int);
        
    void setAverageTextureThreshold (double);

signals:
    // Signals from interface
    void parameterChanged ();

protected:
    // Block matcher
    cv::gpu::StereoBM_GPU bm;
    QMutex mutex;

    cv::Mat tmpImg1, tmpImg2;
};

}

#endif
