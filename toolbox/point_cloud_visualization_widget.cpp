/*
 * MVL Stereo Toolbox: point cloud visualization widget
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


namespace MVL {
namespace StereoToolbox {
namespace GUI {


// *********************************************************************
// *                  Point cloud visualization widget                 *
// *********************************************************************
PointCloudVisualizationWidget::PointCloudVisualizationWidget (QWidget *parent)
    : QOpenGLWidget(parent), numPoints(0), freshData(false)
{
}

PointCloudVisualizationWidget::~PointCloudVisualizationWidget ()
{
}


void PointCloudVisualizationWidget::setPointCloud (const cv::Mat &image, const cv::Mat &points)
{
    // Sanity check: image and points matrix must be of same dimensions
    if (image.rows*image.cols != points.rows*points.cols) {
        return;
    }

    // Number of points
    numPoints = image.rows*image.cols;

    // Copy image and points...
    image.copyTo(this->image);
    points.copyTo(this->points);

    // ... and toggle data upload flag
    freshData = true;

    // Force redraw
    update();
}


void PointCloudVisualizationWidget::initializeGL ()
{
    QOpenGLFunctions *glFunctions = QOpenGLContext::currentContext()->functions();

    // Clear
    glFunctions->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Enable relevant states
    glFunctions->glEnable(GL_DEPTH_TEST);

    glFunctions->glEnable(GL_MULTISAMPLE);
    glFunctions->glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    // Shader program
    shaderProgramPointCloud.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/pointcloud.vert");
    shaderProgramPointCloud.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/pointcloud.frag");
    if (!shaderProgramPointCloud.link()) {
        qWarning() << this << "Failed to link shader program for surface!";
    }
    shaderProgramPointCloud.bind();

    // Create VAO
    vaoPointCloud.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vaoPointCloud);

    // Create VBO
    if (!vboPoints.create()) {
        qWarning() << "Failed to create point-cloud VBO!";
    }
    vboPoints.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    vboPoints.bind();

    // Set up attribute buffers
    shaderProgramPointCloud.enableAttributeArray("vertex");
    shaderProgramPointCloud.enableAttributeArray("color");

    shaderProgramPointCloud.setAttributeBuffer("vertex", GL_FLOAT, PointCloudData_VertexX*sizeof(float), 3, PointCloudData_Size*sizeof(float));
    shaderProgramPointCloud.setAttributeBuffer("color", GL_FLOAT, PointCloudData_ColorR*sizeof(float), 3, PointCloudData_Size*sizeof(float));
}

void PointCloudVisualizationWidget::resizeGL (int w, int h)
{
    QOpenGLFunctions *glFunctions = QOpenGLContext::currentContext()->functions();

    // Setup viewport
    glFunctions->glViewport(0, 0, w, h);

    // Perspective projection
    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(60.0f, float(w)/float(h), 0.001f, 100.0f);
}

void PointCloudVisualizationWidget::paintGL ()
{
    QOpenGLFunctions *glFunctions = QOpenGLContext::currentContext()->functions();

    // Clear
    glFunctions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Upload the data, if necessary
    if (freshData) {
        freshData = false;

        // Bind
        vboPoints.bind();

        // Resize buffer if necessary
        int bufferSize = numPoints*PointCloudData_Size*sizeof(float);
        if (vboPoints.size() != bufferSize) {
            vboPoints.allocate(bufferSize);
        }

        // Map
        float *bufferPtr = static_cast<float *>(vboPoints.map(QOpenGLBuffer::WriteOnly));

        // Fill
        for (int y = 0; y < image.rows; y++) {
            const cv::Vec3b *imagePtr = image.ptr<cv::Vec3b>(y);
            const cv::Vec3f *pointsPtr = points.ptr<cv::Vec3f>(y);

            for (int x = 0; x < image.cols; x++) {
                const cv::Vec3b &bgr = imagePtr[x];
                const cv::Vec3f &xyz = pointsPtr[x];

                float *ptr = bufferPtr + (y*image.cols + x)*PointCloudData_Size;
                ptr[PointCloudData_VertexX] = xyz[0]/1000.0;
                ptr[PointCloudData_VertexY] = -xyz[1]/1000.0;
                ptr[PointCloudData_VertexZ] = -xyz[2]/1000.0;
                ptr[PointCloudData_ColorR] = bgr[2]/255.0f;
                ptr[PointCloudData_ColorG] = bgr[1]/255.0f;
                ptr[PointCloudData_ColorB] = bgr[0]/255.0f;
            }
        }

        vboPoints.unmap();
    }

    // Render
    QOpenGLVertexArrayObject::Binder vaoBinder(&vaoPointCloud);
    shaderProgramPointCloud.bind();
    shaderProgramPointCloud.setUniformValue("pvm", projectionMatrix);

    glFunctions->glDrawArrays(GL_POINTS, 0, numPoints);

    shaderProgramPointCloud.release();
}


} // GUI
} // StereoToolbox
} // MVL
