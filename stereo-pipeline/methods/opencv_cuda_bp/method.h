/*
 * OpenCV CUDA Belief Propagation: method
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__METHODS__OPENCV_CUDA_BP__METHOD_H
#define MVL_STEREO_TOOLBOX__PIPELINE__METHODS__OPENCV_CUDA_BP__METHOD_H

#include <stereo-pipeline/stereo_method.h>

#include <opencv2/cudastereo.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodOpenCvCudaBp {


class Method : public QObject, public StereoMethod
{
    Q_OBJECT
    Q_INTERFACES(MVL::StereoToolbox::Pipeline::StereoMethod)

public:
    Method (QObject *parent = nullptr);
    virtual ~Method ();

    virtual QString getShortName () const override;
    virtual QWidget *createConfigWidget (QWidget *parent = nullptr) override;
    virtual void computeDisparity (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities) override;
    virtual void loadParameters (const QString &filename) override;
    virtual void saveParameters (const QString &filename) const override;

    // Parameters
    enum PresetType {
        OpenCVInit,
        OpenCVRecommended,
    };

    void usePreset (int preset);


    void setNumDisparities (int value);
    int getNumDisparities () const;


    void setIterations (int value);
    int getIterations () const;

    void setLevels (int value);
    int getLevels () const;


    void setMaxDataTerm (double value);
    double getMaxDataTerm () const;

    void setDataWeight (double value);
    double getDataWeight () const;

    void setMaxDiscTerm (double value);
    double getMaxDiscTerm () const;

    void setDiscSingleJump (double value);
    double getDiscSingleJump () const;

signals:
    // Signals from interface
    void parameterChanged () override;

protected:
    // Method implementation
    cv::Ptr<cv::cuda::StereoBeliefPropagation> bp;
    QMutex mutex;

    int imageWidth, imageHeight;

    cv::Mat tmpDisparity;
};


} // StereoMethodOpenCvCudaBp
} // Pipeline
} // StereoToolbox
} // MVL


#endif
