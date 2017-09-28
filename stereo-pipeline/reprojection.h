/*
 * Stereo Pipeline: reprojection
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__REPROJECTION_H
#define MVL_STEREO_TOOLBOX__PIPELINE__REPROJECTION_H

#include <stereo-pipeline/export.h>

#include <QtCore>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class ReprojectionPrivate;

class MVL_STEREO_PIPELINE_EXPORT Reprojection : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Reprojection)
    Q_DECLARE_PRIVATE(Reprojection)
    QScopedPointer<ReprojectionPrivate> const d_ptr;

public:
    Reprojection (QObject *parent = Q_NULLPTR);
    ~Reprojection ();

    enum {
        MethodOpenCvCpu,
        MethodOpenCvCuda,
    };

    void setReprojectionMethod (int method);
    int getReprojectionMethod () const;
    const QList<int> &getSupportedReprojectionMethods () const;

    void setReprojectionMatrix (const cv::Mat &Q);
    const cv::Mat &getReprojectionMatrix () const;

    void reprojectDisparity (const cv::Mat &disparity, cv::Mat &points) const;

signals:
    void reprojectionMethodChanged (int method);
    void reprojectionMatrixChanged ();

    void error (const QString &message);
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
