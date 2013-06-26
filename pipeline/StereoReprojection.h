/*
 * Stereo Pipeline: stereo reprojection
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

#ifndef STEREO_REPROJECTION_H
#define STEREO_REPROJECTION_H

#include <QtCore>
#include <opencv2/core/core.hpp>


class StereoReprojection : public QObject
{
    Q_OBJECT
    
public:
    StereoReprojection (QObject * = 0);
    ~StereoReprojection ();

    bool getUseGpu () const;

    void setReprojectionMatrix (const cv::Mat &);
    const cv::Mat &getReprojectionMatrix () const;

    void reprojectStereoDisparity (const cv::Mat &, cv::Mat &) const;

public slots:
    void setUseGpu (bool);

signals:
    void useGpuChanged (bool);
    void reprojectionMatrixChanged ();

    void error (QString);

protected:
    cv::Mat Q;
    bool useGpu;
};


#endif
