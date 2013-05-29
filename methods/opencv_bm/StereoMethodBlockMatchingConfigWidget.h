/*
 * OpenCV Block Matching: config widget
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

#ifndef STEREO_METHOD_BLOCK_MATCHING_CONFIG_WIDGET_H
#define STEREO_METHOD_BLOCK_MATCHING_CONFIG_WIDGET_H

#include <QtGui>


class StereoMethodBlockMatching;

class StereoMethodBlockMatchingConfigWidget : public QWidget
{
    Q_OBJECT
    
public:
    StereoMethodBlockMatchingConfigWidget (StereoMethodBlockMatching *, QWidget * = 0);
    virtual ~StereoMethodBlockMatchingConfigWidget ();

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