/*
 * OpenCV Semi-Global Block Matching: config widget
 * Copyright (C) 2013 Rok Mandeljc
 *
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
 
#ifndef STEREO_METHOD_SEMI_GLOBAL_BLOCK_MATCHING_CONFIG_WIDGET_H
#define STEREO_METHOD_SEMI_GLOBAL_BLOCK_MATCHING_CONFIG_WIDGET_H

#include <QtGui>


class StereoMethodSemiGlobalBlockMatching;

class StereoMethodSemiGlobalBlockMatchingConfigWidget : public QWidget
{
    Q_OBJECT
    
public:
    StereoMethodSemiGlobalBlockMatchingConfigWidget (StereoMethodSemiGlobalBlockMatching *, QWidget * = 0);
    virtual ~StereoMethodSemiGlobalBlockMatchingConfigWidget ();

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
