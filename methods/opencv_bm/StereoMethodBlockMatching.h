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
 
#ifndef STEREO_METHOD_BLOCK_MATCHING
#define STEREO_METHOD_BLOCK_MATCHING

#include "StereoMethod.h"

#include <opencv2/calib3d/calib3d.hpp>


class StereoMethodBlockMatching : public StereoMethod
{
    Q_OBJECT

public:
    StereoMethodBlockMatching (QObject * = 0);
    virtual ~StereoMethodBlockMatching ();

    // Disparity image computation
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &, int &);

    // Parameter import/export
    virtual void loadParameters (const cv::FileStorage &);
    virtual void saveParameters (cv::FileStorage &) const;

    // Parameters
    int getPreFilterType () const;
    int getPreFilterSize () const;
    int getPreFilterCap () const;

    int getSADWindowSize () const;
    int getMinDisparity () const;
    int getNumDisparities () const;

    int getTextureThreshold () const;
    int getUniquenessRatio () const;
    int getSpeckleWindowSize () const;
    int getSpeckleRange () const;

    bool getTrySmallerWindows () const;

    int getDisp12MaxDiff () const;

    enum {
        OpenCVBasic,
        OpenCVFishEye,
        OpenCVNarrow,
        StereoMatch,
    } PresetType;

public slots:
    void usePreset (int type);

    void setPreFilterType (int);
    void setPreFilterSize (int);
    void setPreFilterCap (int);
        
    void setSADWindowSize (int);
    void setMinDisparity (int);
    void setNumDisparities (int);
    
    
    void setTextureThreshold (int);
    void setUniquenessRatio (int);
    void setSpeckleWindowSize (int);
    void setSpeckleRange (int);
    
    void setTrySmallerWindows (bool);
    
    void setDisp12MaxDiff (int);
    
protected:
    // Block matcher
    cv::StereoBM bm;

    cv::Mat tmpImg1, tmpImg2;
    cv::Mat tmpDisparity;
};


class ConfigTabBlockMatching : public QWidget
{
    Q_OBJECT
    
public:
    ConfigTabBlockMatching (StereoMethodBlockMatching *, QWidget * = 0);
    virtual ~ConfigTabBlockMatching ();

protected slots:
    void presetChanged (int);

    void preFilterTypeChanged (int);
    void trySmallerWindowsChanged (int);

    void updateParameters ();

protected:
    StereoMethodBlockMatching *method;

    QComboBox *comboBoxPreset;
    QComboBox *comboBoxPreFilterType;
    QSpinBox *spinBoxPreFilterSize;
    QSpinBox *spinBoxPreFilterCap;
    QSpinBox *spinBoxSADWindowSize;
    QSpinBox *spinBoxMinDisparity;
    QSpinBox *spinBoxNumDisparities;
    QSpinBox *spinBoxTextureThreshold;
    QSpinBox *spinBoxUniquenessRatio;
    QSpinBox *spinBoxSpeckleWindowSize;
    QSpinBox *spinBoxSpeckleRange;
    QCheckBox *checkBoxTrySmallerWindow;
    QSpinBox *spinBoxDisp12MaxDiff;
};


#endif
