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
 
#ifndef STEREO_METHOD_VAR
#define STEREO_METHOD_VAR

#include <QtCore>
#include <opencv2/contrib/contrib.hpp>

#include "StereoMethod.h"

class StereoMethodVar : public StereoMethod
{
    Q_OBJECT

public:
    StereoMethodVar (QObject * = 0);
    virtual ~StereoMethodVar ();

    // Disparity image computation
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &);

    // Parameter import/export
    virtual void loadParameters (const cv::FileStorage &);
    virtual void saveParameters (cv::FileStorage &) const;
    
    // Parameters
    int getLevels () const;
    double getPyrScale () const;
    int getNumIterations () const;
    int getMinDisparity () const;
    int getMaxDisparity () const;
    int getPolyN () const;
    double getPolySigma () const;
    double getFi () const;
    double getLambda () const;
    int getPenalization () const;
    int getCycle () const;
    int getFlags () const;

    enum {
        OpenCV,
        StereoMatch,
    } PresetType;

public slots:
    void usePreset (int type);
    
    void setLevels (int);
    void setPyrScale (double);
    void setNumIterations (int);
    void setMinDisparity (int);
    void setMaxDisparity (int);
    void setPolyN (int);
    void setPolySigma (double);
    void setFi (double);
    void setLambda (double);
    void setPenalization (int);
    void setCycle (int);
    void setFlags (int);
    
protected:
    // Semi-global block matcher
    cv::StereoVar var;

    cv::Mat tmpDepth;
};


class ConfigTabVar : public QWidget
{
    Q_OBJECT
    
public:
    ConfigTabVar (StereoMethodVar *, QWidget * = 0);
    virtual ~ConfigTabVar ();

protected slots:
    void presetChanged (int);

    void penalizationChanged (int);
    void cycleChanged (int);
    void flagsChanged ();

    void updateParameters ();

protected:
    StereoMethodVar *method;

    QComboBox *comboBoxPreset;

    QSpinBox *spinBoxLevels;
    QDoubleSpinBox *spinBoxPyrScale;
    QSpinBox *spinBoxNumIterations;
    QSpinBox *spinBoxMinDisparity;
    QSpinBox *spinBoxMaxDisparity;
    QSpinBox *spinBoxPolyN;
    QDoubleSpinBox *spinBoxPolySigma;
    QDoubleSpinBox *spinBoxFi;
    QDoubleSpinBox *spinBoxLambda;

    QComboBox *comboBoxPenalization;

    QComboBox *comboBoxCycle;
    
    QCheckBox *checkBoxUseInitialDisparity;
    QCheckBox *checkBoxUseEqualizeHist;
    QCheckBox *checkBoxUseSmartId;
    QCheckBox *checkBoxUseAutoParams;
    QCheckBox *checkBoxUseMedianFiltering;
};



#endif
