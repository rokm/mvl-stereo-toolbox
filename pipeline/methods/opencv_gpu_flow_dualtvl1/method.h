/*
 * OpenCV GPU Dense Optical Flow - Dual TVL1: method
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
 
#ifndef STEREO_METHOD_FLOW_DUALTVL1_GPU_H
#define STEREO_METHOD_FLOW_DUALTVL1_GPU_H

#include <stereo_method.h>

#include <opencv2/gpu/gpu.hpp>


namespace StereoMethodFlowDualTVL1Gpu {
    
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
    double getTau () const;
    double getLambda () const;
    double getTheta () const;
    int getNumberOfScales () const;
    int getNumberOfWarps () const;
    double getEpsilon () const;
    int getNumberOfIterations () const;
    
public slots:
    void setReverseImages (bool);
    void setTau (double);
    void setLambda (double);
    void setTheta (double);
    void setNumberOfScales (int);
    void setNumberOfWarps (int);
    void setEpsilon (double);
    void setNumberOfIterations (int);
    
signals:
    // Signals from interface
    void parameterChanged ();

protected:
    // Optical flow
    cv::gpu::OpticalFlowDual_TVL1_GPU flow;
    bool reverseImages;
    QMutex mutex;
    
    cv::Mat tmpImg1, tmpImg2;
    cv::Mat tmpDisp;
};

};

#endif
