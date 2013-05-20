/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */
 
#ifndef STEREO_METHOD_ELAS
#define STEREO_METHOD_ELAS

#include "StereoMethod.h"

#include <elas.h>


class StereoMethodELAS : public StereoMethod
{
    Q_OBJECT

public:
    StereoMethodELAS (QObject * = 0);
    virtual ~StereoMethodELAS ();

    // Disparity image computation
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &, int &);

    // Parameter import/export
    virtual void loadParameters (const cv::FileStorage &);
    virtual void saveParameters (cv::FileStorage &) const;

    // Generic parameter setting - we override it because elas object
    // needs to be reinitialized every time a parameter changes
    template <typename T> void setParameter (T &parameter, const T &newValue) {
        // Set only if necessary
        if (parameter != newValue) {
            parameter = newValue;

            elas = Elas(param);
            emit parameterChanged();
        }
    }

    // Parameters
    int getMinDisparity () const;
    int getMaxDisparity () const;

    double getSupportThreshold () const;
    int getSupportTexture () const;
    int getCandidateStepSize () const;
    int getInconsistentWindowSize () const;
    int getInconsistentThreshold () const;
    int getInconsistentMinSupport () const;

    bool getAddCorners () const;
    int getGridSize () const;

    double getBeta () const;
    double getGamma () const;
    double getSigma () const;
    double getSigmaRadius () const;

    int getMatchTexture () const;
    int getLRThreshold () const;

    double getSpeckleSimThreshold () const;
    int getSpeckleSize () const;
    int getInterpolationGapWidth () const;

    bool getFilterMedian () const;
    bool getFilterAdaptiveMean () const;
    bool getPostProcessOnlyLeft () const;
    bool getSubsampling () const;

    bool getReturnLeft () const;

    enum {
        ElasRobotics,
        ElasMiddlebury,
    } PresetType;

public slots:
    void usePreset (int type);

    void setMinDisparity (int);
    void setMaxDisparity (int);

    void setSupportThreshold (double);
    void setSupportTexture (int);
    void setCandidateStepSize (int);
    void setInconsistentWindowSize (int);
    void setInconsistentThreshold (int);
    void setInconsistentMinSupport (int);

    void setAddCorners (bool);
    void setGridSize (int);

    void setBeta (double);
    void setGamma (double);
    void setSigma (double);
    void setSigmaRadius (double);

    void setMatchTexture (int);
    void setLRThreshold (int);

    void setSpeckleSimThreshold (double);
    void setSpeckleSize (int);
    void setInterpolationGapWidth (int);

    void setFilterMedian (bool);
    void setFilterAdaptiveMean (bool);
    void setPostProcessOnlyLeft (bool);
    void setSubsampling (bool);

    void setReturnLeft (bool);
    
protected:
    // ELAS
    Elas::parameters param;
    Elas elas;

    bool returnLeft;

    cv::Mat tmpImg1, tmpImg2, tmpDisp1, tmpDisp2;
};


class ConfigTabELAS : public QWidget
{
    Q_OBJECT
    
public:
    ConfigTabELAS (StereoMethodELAS *, QWidget * = 0);
    virtual ~ConfigTabELAS ();

protected slots:
    void presetChanged (int);

    void addCornersChanged (int);
    void medianFilterChanged (int);
    void adaptiveMeanFilterChanged (int);
    void postProcessOnlyLeftChanged (int);
    void subsamplingChanged (int);
    void returnLeftChanged (int);
    
    void updateParameters ();

protected:
    StereoMethodELAS *method;

    QComboBox *comboBoxPreset;

    QSpinBox *spinBoxMinDisparity;
    QSpinBox *spinBoxMaxDisparity;

    QDoubleSpinBox *spinBoxSupportThreshold;
    QSpinBox *spinBoxSupportTexture;
    QSpinBox *spinBoxCandidateStepSize;
    QSpinBox *spinBoxInconsistentWindowSize;
    QSpinBox *spinBoxInconsistentThreshold;
    QSpinBox *spinBoxInconsistentMinSupport;

    QCheckBox *checkBoxAddCorners;
    QSpinBox *spinBoxGridSize;
    
    QDoubleSpinBox *spinBoxBeta;
    QDoubleSpinBox *spinBoxGamma;
    QDoubleSpinBox *spinBoxSigma;
    QDoubleSpinBox *spinBoxSigmaRadius;

    QSpinBox *spinBoxMatchTexture;
    QSpinBox *spinBoxLRThreshold;

    QDoubleSpinBox *spinBoxSpeckleSimThreshold;
    QSpinBox *spinBoxSpeckleSize;
    QSpinBox *spinBoxInterpolationGapWidth;

    QCheckBox *checkBoxFilterMedian;
    QCheckBox *checkBoxFilterAdaptiveMean;
    QCheckBox *checkBoxPostProcessOnlyLeft;
    QCheckBox *checkBoxSubsampling;

    QCheckBox *checkBoxReturnLeft;
};


#endif
