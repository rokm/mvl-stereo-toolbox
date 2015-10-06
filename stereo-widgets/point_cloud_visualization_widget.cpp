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

#include "point_cloud_visualization_widget.h"
#include "point_cloud_visualization_widget_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


PointCloudVisualizationWidgetPrivate::PointCloudVisualizationWidgetPrivate (PointCloudVisualizationWidget *parent)
    : q_ptr(parent), numPoints(0), freshData(false)
{
}


PointCloudVisualizationWidget::PointCloudVisualizationWidget (QWidget *parent)
    : PointCloudVisualizationWidget(new PointCloudVisualizationWidgetPrivate(this), parent)
{
    // Hooray for C++11 constructor delegation
}

PointCloudVisualizationWidget::PointCloudVisualizationWidget (PointCloudVisualizationWidgetPrivate *d, QWidget *parent)
    : QOpenGLWidget(parent), d_ptr(d)
{
    // Set OpenGL to 3.3 core profile
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(fmt);
}

PointCloudVisualizationWidget::~PointCloudVisualizationWidget ()
{
}


void PointCloudVisualizationWidget::setPointCloud (const cv::Mat &image, const cv::Mat &points)
{
    Q_D(PointCloudVisualizationWidget);

    // Sanity check: image and points matrix must be of same dimensions
    if (image.rows*image.cols != points.rows*points.cols) {
        return;
    }

    // Copy image and points...
    image.copyTo(d->image);
    points.copyTo(d->points);

    // Number of points
    d->numPoints = image.rows*image.cols;

    // ... and toggle data upload flag
    d->freshData = true;

    // Force redraw
    update();
}


// *********************************************************************
// *                          OpenGL functions                         *
// *********************************************************************
void PointCloudVisualizationWidget::initializeGL ()
{
    Q_D(PointCloudVisualizationWidget);
    QOpenGLFunctions *glFunctions = QOpenGLContext::currentContext()->functions();

    // Clear
    glFunctions->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Enable relevant states
    glFunctions->glEnable(GL_DEPTH_TEST);

    glFunctions->glEnable(GL_MULTISAMPLE);
    glFunctions->glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    // Shader program
    d->shaderProgramPointCloud.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/pointcloud.vert");
    d->shaderProgramPointCloud.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/pointcloud.frag");
    if (!d->shaderProgramPointCloud.link()) {
        qWarning() << this << "Failed to link shader program for surface!";
    }
    d->shaderProgramPointCloud.bind();

    // Create VAO
    d->vaoPointCloud.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&d->vaoPointCloud);

    // Create VBO
    if (!d->vboPoints.create()) {
        qWarning() << "Failed to create point-cloud VBO!";
    }
    d->vboPoints.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    d->vboPoints.bind();

    // Set up attribute buffers
    d->shaderProgramPointCloud.enableAttributeArray("vertex");
    d->shaderProgramPointCloud.enableAttributeArray("color");

    d->shaderProgramPointCloud.setAttributeBuffer("vertex", GL_FLOAT, 0*sizeof(float), 3, 6*sizeof(float));
    d->shaderProgramPointCloud.setAttributeBuffer("color", GL_FLOAT, 3*sizeof(float), 3, 6*sizeof(float));
}

void PointCloudVisualizationWidget::resizeGL (int w, int h)
{
    Q_D(PointCloudVisualizationWidget);
    QOpenGLFunctions *glFunctions = QOpenGLContext::currentContext()->functions();

    // Setup viewport
    glFunctions->glViewport(0, 0, w, h);

    // Perspective projection
    d->projectionMatrix.setToIdentity();
    d->projectionMatrix.perspective(60.0f, float(w)/float(h), 0.1f, 10000.0f);
}

void PointCloudVisualizationWidget::paintGL ()
{
    Q_D(PointCloudVisualizationWidget);
    QOpenGLFunctions *glFunctions = QOpenGLContext::currentContext()->functions();

    // Clear
    glFunctions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Upload the data, if necessary
    if (d->freshData) {
        d->freshData = false;

        // Bind
        d->vboPoints.bind();

        // Resize buffer if necessary
        int bufferSize = d->numPoints * 6*sizeof(float);
        if (d->vboPoints.size() != bufferSize) {
            d->vboPoints.allocate(bufferSize);
        }

        // Map
        float *bufferPtr = static_cast<float *>(d->vboPoints.map(QOpenGLBuffer::WriteOnly));

        // Fill
        for (int y = 0; y < d->image.rows; y++) {
            const cv::Vec3b *imagePtr = d->image.ptr<cv::Vec3b>(y);
            const cv::Vec3f *pointsPtr = d->points.ptr<cv::Vec3f>(y);

            for (int x = 0; x < d->image.cols; x++) {
                const cv::Vec3b &bgr = imagePtr[x];
                const cv::Vec3f &xyz = pointsPtr[x];

                float *ptr = bufferPtr + (y*d->image.cols + x)*6;
                *ptr++ =  xyz[0]/1000.0;
                *ptr++ = -xyz[1]/1000.0;
                *ptr++ = -xyz[2]/1000.0;
                *ptr++ =  bgr[2]/255.0f;
                *ptr++ =  bgr[1]/255.0f;
                *ptr++ =  bgr[0]/255.0f;
            }
        }

        d->vboPoints.unmap();
    }

    // Render
    QOpenGLVertexArrayObject::Binder vaoBinder(&d->vaoPointCloud);
    d->shaderProgramPointCloud.bind();
    d->shaderProgramPointCloud.setUniformValue("pvm", d->projectionMatrix);

    glFunctions->glDrawArrays(GL_POINTS, 0, d->numPoints);

    d->shaderProgramPointCloud.release();
}


// *********************************************************************
// *                             User input                            *
// *********************************************************************
void PointCloudVisualizationWidget::mousePressEvent (QMouseEvent *event)
{
    // Chain up to parent first...
    QOpenGLWidget::mousePressEvent(event);
    if (event->isAccepted()) {
        return;
    }

    // TODO: our handling
}

void PointCloudVisualizationWidget::mouseReleaseEvent (QMouseEvent *event)
{
    // Chain up to parent first...
    QOpenGLWidget::mouseReleaseEvent(event);
    if (event->isAccepted()) {
        return;
    }

    // TODO: our handling
}

void PointCloudVisualizationWidget::mouseMoveEvent (QMouseEvent *event)
{
    // Chain up to parent first...
    QOpenGLWidget::mouseMoveEvent(event);
    if (event->isAccepted()) {
        return;
    }

    // TODO: our handling
}

void PointCloudVisualizationWidget::wheelEvent (QWheelEvent *event)
{
    // Chain up to parent first...
    QOpenGLWidget::wheelEvent(event);
    if (event->isAccepted()) {
        return;
    }

    // TODO: our handling
}

void PointCloudVisualizationWidget::keyPressEvent (QKeyEvent *event)
{
    // Chain up to parent first...
    QOpenGLWidget::keyPressEvent(event);
    if (event->isAccepted()) {
        return;
    }

    // TODO: our handling
}


} // Widgets
} // StereoToolbox
} // MVL
