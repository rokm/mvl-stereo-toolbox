/*
 * Efficient LArge-scale Stereo: method
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__METHODS__ELAS__METHOD_H
#define MVL_STEREO_TOOLBOX__PIPELINE__METHODS__ELAS__METHOD_H

#include <stereo-pipeline/stereo_method.h>

#include <elas.h>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodELAS {


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
        ElasRobotics,
        ElasMiddlebury,
    };

    void usePreset (int preset);

    void setMinDisparity (int value);
    int getMinDisparity () const;

    void setMaxDisparity (int value);
    int getMaxDisparity () const;


    void setSupportThreshold (double value);
    double getSupportThreshold () const;

    void setSupportTexture (int value);
    int getSupportTexture () const;

    void setCandidateStepSize (int value);
    int getCandidateStepSize () const;

    void setInconsistentWindowSize (in valuet);
    int getInconsistentWindowSize () const;

    void setInconsistentThreshold (int value);
    int getInconsistentThreshold () const;

    void setInconsistentMinSupport (int value);
    int getInconsistentMinSupport () const;


    void setAddCorners (bool value);
    bool getAddCorners () const;

    void setGridSize (int value);
    int getGridSize () const;


    void setBeta (double value);
    double getBeta () const;

    void setGamma (double value);
    double getGamma () const;

    void setSigma (double value);
    double getSigma () const;

    void setSigmaRadius (double value);
    double getSigmaRadius () const;


    void setMatchTexture (int value);
    int getMatchTexture () const;

    void setLRThreshold (int value);
    int getLRThreshold () const;


    void setSpeckleSimThreshold (double value);
    double getSpeckleSimThreshold () const;

    void setSpeckleSize (int value);
    int getSpeckleSize () const;

    void setInterpolationGapWidth (int value);
    int getInterpolationGapWidth () const;


    void setFilterMedian (bool value);
    bool getFilterMedian () const;

    void setFilterAdaptiveMean (bool value);
    bool getFilterAdaptiveMean () const;

    void setPostProcessOnlyLeft (bool value);
    bool getPostProcessOnlyLeft () const;

    void setSubsampling (bool value);
    bool getSubsampling () const;


    void setReturnLeft (bool value);
    bool getReturnLeft () const;

protected:
    // Generic parameter setting
    template <typename T> void setParameter (T &parameter, const T &value) {
        // Set only if necessary
        if (parameter != value) {
            parameter = value;

            // Create ELAS object
            createElasObject();
        }
    }

    void createElasObject ();

signals:
    // Signals from interface
    void parameterChanged () override;

protected:
    // Method implementation
    Elas::parameters param;
    Elas elas;

    bool returnLeft;

    QMutex mutex;

    cv::Mat tmpImg1, tmpImg2, tmpDisp1, tmpDisp2;
};


} // StereoMethodELAS
} // Pipeline
} // StereoToolbox
} // MVL


#endif
