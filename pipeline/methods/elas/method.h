/*
 * Efficient LArge-scale Stereo: method
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__METHODS__ELAS__METHOD_H
#define MVL_STEREO_TOOLBOX__PIPELINE__METHODS__ELAS__METHOD_H

#include <stereo_method.h>

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
    Method (QObject * = 0);
    virtual ~Method ();

    virtual QString getShortName () const;
    virtual QWidget *createConfigWidget (QWidget * = 0);
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &, int &);
    virtual void loadParameters (const QString &);
    virtual void saveParameters (const QString &) const;


    // Parameters
    enum {
        ElasRobotics,
        ElasMiddlebury,
    } PresetType;

    void usePreset (int type);

    void setMinDisparity (int);
    int getMinDisparity () const;

    void setMaxDisparity (int);
    int getMaxDisparity () const;


    void setSupportThreshold (double);
    double getSupportThreshold () const;

    void setSupportTexture (int);
    int getSupportTexture () const;

    void setCandidateStepSize (int);
    int getCandidateStepSize () const;

    void setInconsistentWindowSize (int);
    int getInconsistentWindowSize () const;

    void setInconsistentThreshold (int);
    int getInconsistentThreshold () const;

    void setInconsistentMinSupport (int);
    int getInconsistentMinSupport () const;


    void setAddCorners (bool);
    bool getAddCorners () const;

    void setGridSize (int);
    int getGridSize () const;


    void setBeta (double);
    double getBeta () const;

    void setGamma (double);
    double getGamma () const;

    void setSigma (double);
    double getSigma () const;

    void setSigmaRadius (double);
    double getSigmaRadius () const;


    void setMatchTexture (int);
    int getMatchTexture () const;

    void setLRThreshold (int);
    int getLRThreshold () const;


    void setSpeckleSimThreshold (double);
    double getSpeckleSimThreshold () const;

    void setSpeckleSize (int);
    int getSpeckleSize () const;

    void setInterpolationGapWidth (int);
    int getInterpolationGapWidth () const;


    void setFilterMedian (bool);
    bool getFilterMedian () const;

    void setFilterAdaptiveMean (bool);
    bool getFilterAdaptiveMean () const;

    void setPostProcessOnlyLeft (bool);
    bool getPostProcessOnlyLeft () const;

    void setSubsampling (bool);
    bool getSubsampling () const;


    void setReturnLeft (bool);
    bool getReturnLeft () const;

protected:
    // Generic parameter setting
    template <typename T> void setParameter (T &parameter, const T &newValue) {
        // Set only if necessary
        if (parameter != newValue) {
            parameter = newValue;

            // Create ELAS object
            createElasObject();
        }
    }

    void createElasObject ();

signals:
    // Signals from interface
    void parameterChanged ();

protected:
    // Method implementation
    Elas::parameters param;
    Elas elas;

    bool returnLeft;

    QMutex mutex;

    cv::Mat tmpImg1, tmpImg2, tmpDisp1, tmpDisp2;
};


}; // StereoMethodELAS
}; // Pipeline
}; // StereoToolbox
}; // MVL


#endif
