/*
 * Stereo Widgets: point cloud visualization widget
 * Copyright (C) 2015 Rok Mandeljc
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

#ifndef MVL_STEREO_TOOLBOX__WIDGETS__POINT_CLOUD_VISUALIZATION_WIDGET_H
#define MVL_STEREO_TOOLBOX__WIDGETS__POINT_CLOUD_VISUALIZATION_WIDGET_H

#include <stereo-widgets/export.h>

#include <QtWidgets>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


class PointCloudVisualizationWidgetPrivate;


// QOpenGLWidget is available from Qt 5.4 on...
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)

class MVL_STEREO_WIDGETS_EXPORT PointCloudVisualizationWidget : public QOpenGLWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PointCloudVisualizationWidget)
    Q_DECLARE_PRIVATE(PointCloudVisualizationWidget)

protected:
    QScopedPointer<PointCloudVisualizationWidgetPrivate> const d_ptr;
    PointCloudVisualizationWidget (PointCloudVisualizationWidgetPrivate *d, QWidget *parent = 0);

public:
    PointCloudVisualizationWidget (QWidget *parent = 0);
    virtual ~PointCloudVisualizationWidget ();

    void setImage (const cv::Mat &image);
    void setPoints (const cv::Mat &points);
    void setPointCloud (const cv::Mat &image, const cv::Mat &points);

protected:
    virtual void mousePressEvent (QMouseEvent *event);
    virtual void mouseReleaseEvent (QMouseEvent *event);
    virtual void mouseMoveEvent (QMouseEvent *event);

    virtual void keyPressEvent (QKeyEvent *event);

    virtual void wheelEvent (QWheelEvent *event);

protected:
    virtual void initializeGL ();
    virtual void resizeGL (int w, int h);
    virtual void paintGL ();
};

#else

class MVL_STEREO_WIDGETS_EXPORT PointCloudVisualizationWidget : public QLabel
{
    Q_OBJECT
    Q_DISABLE_COPY(PointCloudVisualizationWidget)
    Q_DECLARE_PRIVATE(PointCloudVisualizationWidget)

protected:
    QScopedPointer<PointCloudVisualizationWidgetPrivate> const d_ptr;
    PointCloudVisualizationWidget (PointCloudVisualizationWidgetPrivate *d, QWidget *parent = 0);

public:
    PointCloudVisualizationWidget (QWidget *parent = 0);
    virtual ~PointCloudVisualizationWidget ();

    void setImage (const cv::Mat &image);
    void setPoints (const cv::Mat &points);
    void setPointCloud (const cv::Mat &image, const cv::Mat &points);
};


#endif // QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)


} // Widgets
} // StereoToolbox
} // MVL


#endif
