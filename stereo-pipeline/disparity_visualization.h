/*
 * Stereo Pipeline: disparity visualization
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__DISPARITY_VISUALIZATION_H
#define MVL_STEREO_TOOLBOX__PIPELINE__DISPARITY_VISUALIZATION_H

#include <stereo-pipeline/export.h>

#include <QtCore>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class DisparityVisualizationPrivate;

class MVL_STEREO_PIPELINE_EXPORT DisparityVisualization : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DisparityVisualization)
    Q_DECLARE_PRIVATE(DisparityVisualization)
    QScopedPointer<DisparityVisualizationPrivate> const d_ptr;

public:
    DisparityVisualization (QObject *parent = Q_NULLPTR);
    ~DisparityVisualization ();

    enum {
        MethodGrayscale,
        MethodColorCuda,
        MethodColorCpu,
    };

    void setVisualizationMethod (int method);
    int getVisualizationMethod () const;
    const QList<int> &getSupportedVisualizationMethods () const;

    void visualizeDisparity (const cv::Mat &disparity, int disparityLevels, cv::Mat &visualization) const;

signals:
    void visualizationMethodChanged (int method);

    void error (const QString &message);
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
