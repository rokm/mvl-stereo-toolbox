/*
 * OpenCV Binary-descriptor-based Block Matching: method
 * Copyright (C) 2017 Rok Mandeljc
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__METHODS__OPENCV_BINARY_BM__METHOD_H
#define MVL_STEREO_TOOLBOX__PIPELINE__METHODS__OPENCV_BINARY_BM__METHOD_H

#include <stereo-pipeline/stereo_method.h>

#include <opencv2/stereo.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodOpenCvBinaryBm {


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

    void resetParameters ();

    // Parameter-setting helper
    template<typename Method, typename Type>
    void setParameter (Method method, Type value) {
        // Call the setter under lock
        QMutexLocker locker(&mutex);
        (bm->*method)(value);
        locker.unlock();

        // Emit parameter change signal
        emit parameterChanged();
    }

    // Parameters - generic
    int getMinDisparity () const;
    void setMinDisparity (int value);

    int getNumDisparities () const;
    void setNumDisparities (int value);

    int getBlockSize () const;
    void setBlockSize (int value);

    int getSpeckleWindowSize () const;
    void setSpeckleWindowSize (int value);

    int getSpeckleRange () const;
    void setSpeckleRange (int value);

    int getDisp12MaxDiff () const;
    void setDisp12MaxDiff (int value);


    // Parameters - method-specific
    bool getUsePreFilter () const;
    void setUsePreFilter (bool value);

    int getPreFilterType () const;
    void setPreFilterType (int value);

    int getPreFilterSize () const;
    void setPreFilterSize (int value);

    int getPreFilterCap () const;
    void setPreFilterCap (int value);

    int getTextureThreshold () const;
    void setTextureThreshold (int value);

    int getUniquenessRatio () const;
    void setUniquenessRatio (int value);

    int getSmallerBlockSize () const;
    void setSmallerBlockSize (int value);

    int getSpeckleRemovalTechnique () const;
    void setSpeckleRemovalTechnique (int value);

    int getBinaryKernelType () const;
    void setBinaryKernelType (int value);

    int getAggregationWindowSize () const;
    void setAggregationWindowSize (int value);


signals:
    // Signals from interface
    void parameterChanged ();

protected:
    // Block matcher
    cv::Ptr<cv::stereo::StereoBinaryBM> bm;
    QMutex mutex;

    cv::Mat tmpImg1, tmpImg2;
    cv::Mat tmpDisparity;
};


} // StereoMethodOpenCvBinaryBm
} // Pipeline
} // StereoToolbox
} // MVL


#endif
