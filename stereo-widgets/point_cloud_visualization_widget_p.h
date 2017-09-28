/*
 * Stereo Widgets: point cloud visualization widget
 * Copyright (C) 2015-2017 Rok Mandeljc
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

#ifndef MVL_STEREO_TOOLBOX__WIDGETS__POINT_CLOUD_VISUALIZATION_WIDGET_P_H
#define MVL_STEREO_TOOLBOX__WIDGETS__POINT_CLOUD_VISUALIZATION_WIDGET_P_H


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


// QOpenGLWidget is available from Qt 5.4 on...
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)

class ShaderProgramBasicElement : public QOpenGLShaderProgram
{
public:
    void compile ();

    void setVertex (GLenum type, int offset, int tupleSize, int stride = 0);
    void setColor (GLenum type, int offset, int tupleSize, int stride = 0);
    void setColor (const QColor &color);

    void setPvmMatrix (const QMatrix4x4 &pvm);

protected:
    int attributeVertex;
    int attributeColor;
    int uniformPvm;
};

class ShaderProgramVolumetricLine : public QOpenGLShaderProgram
{
public:
    void compile ();

    void setVertex (GLenum type, int offset, int tupleSize, int stride = 0);

    void setPvmMatrix (const QMatrix4x4 &pvm);
    void setColor (const QColor &color);
    void setLineWidth (float width);

protected:
    int attributeVertex;
    int uniformPvm;
    int uniformColor;
    int uniformRadius;
};



class PointCloudVisualizationWidgetPrivate : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PointCloudVisualizationWidgetPrivate)
    Q_DECLARE_PUBLIC(PointCloudVisualizationWidget)

protected:
    PointCloudVisualizationWidgetPrivate (PointCloudVisualizationWidget *parent);
    PointCloudVisualizationWidget * const q_ptr;

protected:
    enum RotationConstraint {
        ConstraintNoAxes,
        ConstraintCameraAxes,
        ConstraintObjectAxes,
    };

    void initializePointCloud ();
    void initializeTrackBall ();

    void uploadPointCloud ();

    void renderPointCloud (QOpenGLFunctions *glFunctions);
    void renderTrackBall (QOpenGLFunctions *glFunctions);

    void beginRotation (const QPointF &pos, RotationConstraint constraint);
    void doRotation (const QPointF &pos);
    void endRotation ();

    void resetView ();
    void addRotationVelocity (float vx, float vy, float vz);
    void addTranslationVelocity (float vx, float vy, float vz);

    void switchRotationAxis ();

    void performZooming (int steps);

    void performMovement ();

    void projectOnSphere (QVector3D &v) const;
    QQuaternion rotationFromMove (const QVector3D &vFrom, const QVector3D &vTo) const;
    QVector3D constrainToAxis (const QVector3D &vector, const QVector3D &axis) const;
    int findNearestConstraintAxis (const QVector3D &vector) const;

protected:
    QMatrix4x4 projectionMatrix;

    // Point-cloud data
    cv::Mat image;
    cv::Mat points;
    int numPoints;
    bool freshData;

    // Trackball
    int width, height;
    const float trackBallRadius;

    QVector3D cameraAxes[3];
    QVector3D objectAxes[3];

    RotationConstraint rotationConstraint;
    int rotationAxisIndex;

    QQuaternion orientation, defaultOrientation;
    QVector3D position, defaultPosition;

    bool rotationActive;
    QPointF prevRotationPos;

    QVector3D translationVelocity, translationAcceleration;
    QVector3D rotationVelocity, rotationAcceleration;

    QTimer *timer;

    // OpenGL shader programs
    ShaderProgramBasicElement shaderProgramBasicElement;
    ShaderProgramVolumetricLine shaderProgramVolumetricLine;

    // OpenGL data buffers
    QOpenGLBuffer vboPoints; // 6 (XYZ+RGB) floats per vertex
    QOpenGLVertexArrayObject vaoPointCloud;

    QOpenGLBuffer vboCircle;
    QOpenGLVertexArrayObject vaoCircle;
    int numCircleVertices;
};

#else

class PointCloudVisualizationWidgetPrivate : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PointCloudVisualizationWidgetPrivate)
    Q_DECLARE_PUBLIC(PointCloudVisualizationWidget)

protected:
    PointCloudVisualizationWidgetPrivate (PointCloudVisualizationWidget *parent);
    PointCloudVisualizationWidget * const q_ptr;
};


#endif // QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)



} // Widgets
} // StereoToolbox
} // MVL


#endif
