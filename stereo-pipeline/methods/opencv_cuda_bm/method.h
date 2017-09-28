/*
 * OpenCV CUDA Block Matching: method
 * Copyright (C) 2013-2017 Rok Mandeljc
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__METHODS__OPENCV_CUDA_BM__METHOD_H
#define MVL_STEREO_TOOLBOX__PIPELINE__METHODS__OPENCV_CUDA_BM__METHOD_H

#include <stereo-pipeline/stereo_method.h>

#include <opencv2/cudastereo.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodOpenCvCudaBm {


class Method : public QObject, public StereoMethod
{
    Q_OBJECT
    Q_INTERFACES(MVL::StereoToolbox::Pipeline::StereoMethod)

public:
    Method (QObject *parent = Q_NULLPTR);
    virtual ~Method ();

    virtual QString getShortName () const override;
    virtual QWidget *createConfigWidget (QWidget *parent = Q_NULLPTR) override;
    virtual void computeDisparity (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities) override;
    virtual void loadParameters (const QString &filename) override;
    virtual void saveParameters (const QString &filename) const override;

    // Parameters
    void resetToDefaults ();

    void setPreFilterType (int value);
    int getPreFilterType () const;

    void setPreFilterCap (int value);
    int getPreFilterCap () const;

    void setNumDisparities (int value);
    int getNumDisparities () const;

    void setWindowSize (int value);
    int getWindowSize () const;

    void setAverageTextureThreshold (double value);
    double getAverageTextureThreshold () const;

signals:
    // Signals from interface
    void parameterChanged () override;

protected:
    // Method implementation
    cv::Ptr<cv::cuda::StereoBM> bm;
    QMutex mutex;

    cv::Mat tmpImg1, tmpImg2;
    cv::Mat tmpDisparity;
};


} // StereoMethodOpenCvCudaBm
} // Pipeline
} // StereoToolbox
} // MVL


#endif
