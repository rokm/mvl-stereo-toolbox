/*
 * Stereo Pipeline: stereo rectification
 * Copyright (C) 2013 Rok Mandeljc
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

#ifndef STEREO_RECTIFICATION_H
#define STEREO_RECTIFICATION_H

#include <QtCore>

#include <opencv2/core/core.hpp>


class StereoRectification : public QObject
{
    Q_OBJECT

public:
    StereoRectification (QObject * = 0);
    virtual ~StereoRectification ();

    void setStereoCalibration (const cv::Mat &, const cv::Mat &, const cv::Mat &, const cv::Mat &, const cv::Mat &, const cv::Mat &, const cv::Size &);
    void loadStereoCalibration (const QString &);
    void saveStereoCalibration (const QString &) const;
    void clearStereoCalibration ();

    void rectifyImagePair (const cv::Mat &, const cv::Mat &, cv::Mat &, cv::Mat &) const;

    bool getState () const;

    const cv::Mat &getReprojectionMatrix () const;

protected:
    void initializeStereoRectification ();

signals:
    void stateChanged (bool);

    void roiChanged ();

protected:
    bool isValid;

    // Raw calibration parameters
    cv::Size imageSize;
        
    cv::Mat M1, M2; // Camera matrices
    cv::Mat D1, D2; // Distortion coefficients

    cv::Mat R, T, E, F;

    // Rectification parameters
    cv::Mat R1, R2;
    cv::Mat P1, P2;
    cv::Mat Q;
    cv::Rect validRoi1, validRoi2;

    bool isVerticalStereo;

    // Rectification maps
    cv::Mat map11, map12, map21, map22;
};

#endif