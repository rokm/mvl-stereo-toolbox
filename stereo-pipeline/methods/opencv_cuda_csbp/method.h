/*
 * OpenCV CUDA Constant Space Belief Propagation: method
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__METHODS__OPENCV_CUDA_CSBP__METHOD_H
#define MVL_STEREO_TOOLBOX__PIPELINE__METHODS__OPENCV_CUDA_CSBP__METHOD_H

#include <stereo-pipeline/stereo_method.h>

#include <opencv2/cudastereo.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodOpenCvCudaCsbp {


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
        OpenCVInit,
        OpenCVRecommended,
    } PresetType;

    void usePreset (int);


    void setNumDisparities (int);
    int getNumDisparities () const;

    void setIterations (int);
    int getIterations () const;

    void setLevels (int);
    int getLevels () const;

    void setNrPlane (int);
    int getNrPlane () const;


    void setMaxDataTerm (double);
    double getMaxDataTerm () const;

    void setDataWeight (double);
    double getDataWeight () const;

    void setMaxDiscTerm (double);
    double getMaxDiscTerm () const;

    void setDiscSingleJump (double);
    double getDiscSingleJump () const;


    void setUseLocalCost (bool);
    bool getUseLocalCost () const;

signals:
    // Signals from interface
    void parameterChanged ();

protected:
    // Method implementation
    cv::Ptr<cv::cuda::StereoConstantSpaceBP> bp;
    QMutex mutex;

    int imageWidth, imageHeight;

    cv::Mat tmpDisparity;
};


} // StereoMethodOpenCvCudaCsbp
} // Pipeline
} // StereoToolbox
} // MVL


#endif
