/*
 * OpenCV GPU Dense Optical Flow - Farneback: method
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
 
#ifndef STEREO_METHOD_FLOW_FARNEBACK_GPU_H
#define STEREO_METHOD_FLOW_FARNEBACK_GPU_H

#include <stereo_method.h>

#include <opencv2/gpu/gpu.hpp>


namespace StereoMethodFlowFarnebackGpu {
    
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
    bool getReverseImages () const;
    int getNumberOfLevels () const;
    double getPyramidScale () const;
    bool getFastPyramids () const;
    int getWindowSize () const;
    int getNumberOfIterations () const;
    int getPolyN () const;
    double getPolySigma () const;
    bool getUseGaussianFilter () const;

public slots:
    void setReverseImages (bool);
    void setNumberOfLevels (int);
    void setPyramidScale (double);
    void setFastPyramids (bool);
    void setWindowSize (int);
    void setNumberOfIterations (int);
    void setPolyN (int);
    void setPolySigma (double);
    void setUseGaussianFilter (bool);

signals:
    // Signals from interface
    void parameterChanged ();

protected:
    // Optical flow
    cv::gpu::FarnebackOpticalFlow flow;
    bool reverseImages;
    QMutex mutex;
    
    cv::Mat tmpImg1, tmpImg2;
    cv::Mat tmpDisp;
};

};

#endif
