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
 
#ifndef STEREO_METHOD_SEMI_GLOBAL_BLOCK_MATCHING
#define STEREO_METHOD_SEMI_GLOBAL_BLOCK_MATCHING

#include <QtCore>
#include <opencv2/calib3d/calib3d.hpp>

#include "StereoMethod.h"

class StereoMethodSemiGlobalBlockMatching : public StereoMethod
{
    Q_OBJECT

public:
    StereoMethodSemiGlobalBlockMatching (QObject * = 0);
    virtual ~StereoMethodSemiGlobalBlockMatching ();

    // Disparity image computation
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &);

    // Parameter import/export
    virtual void loadParameters (const cv::FileStorage &);
    virtual void saveParameters (cv::FileStorage &) const;
    
    // Parameters
    int getMinDisparity () const;
    int getNumDisparities () const;
    int getSADWindowSize () const;

    int getPreFilterCap () const;
    int getUniquenessRatio () const;

    int getP1 () const;
    int getP2 () const;

    int getSpeckleWindowSize () const;
    int getSpeckleRange () const;

    int getDisp12MaxDiff () const;

    bool getFullDP () const;

    enum {
        OpenCV,
        StereoMatch,
    } PresetType;

public slots:
    void usePreset (int);
    
    void setMinDisparity (int);
    void setNumDisparities (int);   
    void setSADWindowSize (int);
    
    void setPreFilterCap (int);
    void setUniquenessRatio (int);
    
    void setP1 (int);
    void setP2 (int);
    
    void setSpeckleWindowSize (int);
    void setSpeckleRange (int);
    
    void setDisp12MaxDiff (int);

    void setFullDP (bool);
    
protected:
    // Semi-global block matcher
    cv::StereoSGBM sgbm;

    cv::Mat tmpDepth;
};


class ConfigTabSemiGlobalBlockMatching : public QWidget
{
    Q_OBJECT
    
public:
    ConfigTabSemiGlobalBlockMatching (StereoMethodSemiGlobalBlockMatching *, QWidget * = 0);
    virtual ~ConfigTabSemiGlobalBlockMatching ();

protected slots:
    void presetChanged (int);

    void fullDPChanged (int);
    
    void updateParameters ();

protected:
    StereoMethodSemiGlobalBlockMatching *method;

    QComboBox *comboBoxPreset;
    QSpinBox *spinBoxMinDisparity;
    QSpinBox *spinBoxNumDisparities;
    QSpinBox *spinBoxSADWindowSize;
    QSpinBox *spinBoxPreFilterCap;
    QSpinBox *spinBoxUniquenessRatio;
    QSpinBox *spinBoxP1;
    QSpinBox *spinBoxP2;
    QSpinBox *spinBoxSpeckleWindowSize;
    QSpinBox *spinBoxSpeckleRange;
    QSpinBox *spinBoxDisp12MaxDiff;
    QCheckBox *checkBoxFullDP;
};



#endif
