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
    : q_ptr(parent), numPoints(0), freshData(false),
      trackBallRadius(0.7f),
      rotationConstraint(ConstraintNoAxes),
      rotationAxisIndex(0),
      rotationActive(false),
      prevRotationPos(0.0f, 0.0f),
      timer(new QTimer(this))
{
    cameraAxes[0] = objectAxes[0] = QVector3D(1.0f, 0.0f, 0.0f);
    cameraAxes[1] = objectAxes[1] = QVector3D(0.0f, 1.0f, 0.0f);
    cameraAxes[2] = objectAxes[2] = QVector3D(0.0f, 0.0f, 1.0f);

    position = defaultPosition = QVector3D(0.0f, 0.0f, 0.0f);
    orientation = defaultOrientation = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, 180.0f); // This rotates the coordinate system of OpenCV to the one used by OpenGL

    translationAcceleration = QVector3D(-0.1f, -0.1f, -0.1f);
    rotationAcceleration = QVector3D(-0.1f, -0.1f, -0.1f);

    timer->setInterval(1000/30); // 30 FPS
    connect(timer, &QTimer::timeout, this, &PointCloudVisualizationWidgetPrivate::performMovement);
    timer->start();
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
    fmt.setSamples(4);
    setFormat(fmt);

    // Keyboard focus
    setFocusPolicy(Qt::StrongFocus );

    // Enable mouse tracking
    setMouseTracking(true);
}

PointCloudVisualizationWidget::~PointCloudVisualizationWidget ()
{
}



void PointCloudVisualizationWidget::setImage (const cv::Mat &image)
{
    Q_D(PointCloudVisualizationWidget);

    image.copyTo(d->image);
    d->freshData = true;

    update();
}

void PointCloudVisualizationWidget::setPoints (const cv::Mat &points)
{
    Q_D(PointCloudVisualizationWidget);

    points.copyTo(d->points);
    d->freshData = true;

    update();
}


void PointCloudVisualizationWidget::setPointCloud (const cv::Mat &image, const cv::Mat &points)
{
    Q_D(PointCloudVisualizationWidget);

    image.copyTo(d->image);
    points.copyTo(d->points);
    d->freshData = true;

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

    glFunctions->glEnable(GL_BLEND);
    glFunctions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glFunctions->glEnable(GL_MULTISAMPLE);
    glFunctions->glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    // Initialize shader programs, buffers, etc.
    d->initializePointCloud();
    d->initializeTrackBall();
}

void PointCloudVisualizationWidget::resizeGL (int w, int h)
{
    Q_D(PointCloudVisualizationWidget);
    QOpenGLFunctions *glFunctions = QOpenGLContext::currentContext()->functions();

    // Setup viewport
    glFunctions->glViewport(0, 0, w, h);

    // Trackball
    d->width = w;
    d->height = h;

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
    d->uploadPointCloud();

    // Render point-cloud
    d->renderPointCloud(glFunctions);

    // Render track-ball
    d->renderTrackBall(glFunctions);
}


// *********************************************************************
// *                             User input                            *
// *********************************************************************
void PointCloudVisualizationWidget::mousePressEvent (QMouseEvent *event)
{
    Q_D(PointCloudVisualizationWidget);

    // Chain up to parent first...
    QOpenGLWidget::mousePressEvent(event);
    if (event->isAccepted()) {
        return;
    }

    // Left button initiates rotation:
    //  - no modifier: free rotation
    //  - shift modifier: rotation around object axes
    //  - control modifier: rotation around camera axes
    if (event->buttons() & Qt::LeftButton) {
        PointCloudVisualizationWidgetPrivate::RotationConstraint constraint = PointCloudVisualizationWidgetPrivate::ConstraintNoAxes;
        if (event->modifiers() & Qt::ShiftModifier) {
            constraint = PointCloudVisualizationWidgetPrivate::ConstraintObjectAxes;
        } else if (event->modifiers() & Qt::ControlModifier) {
            constraint = PointCloudVisualizationWidgetPrivate::ConstraintCameraAxes;
        }
        d->beginRotation(event->localPos(), constraint);
        event->accept();
    }
}

void PointCloudVisualizationWidget::mouseReleaseEvent (QMouseEvent *event)
{
    Q_D(PointCloudVisualizationWidget);

    // Chain up to parent first...
    QOpenGLWidget::mouseReleaseEvent(event);
    if (event->isAccepted()) {
        return;
    }

    // On release event, buttons() excludes the button that generated the event...
    if (!(event->buttons() & Qt::LeftButton)) {
        d->endRotation();
        event->accept();
    }
}

void PointCloudVisualizationWidget::mouseMoveEvent (QMouseEvent *event)
{
    Q_D(PointCloudVisualizationWidget);

    // Chain up to parent first...
    QOpenGLWidget::mouseMoveEvent(event);
    if (event->isAccepted()) {
        return;
    }

    // Do rotation processing in any case (so that we capture new constraints)
    d->doRotation(event->localPos());
    if (event->buttons() & Qt::LeftButton) {
        event->accept();
    }
}

void PointCloudVisualizationWidget::wheelEvent (QWheelEvent *event)
{
    Q_D(PointCloudVisualizationWidget);

    // Chain up to parent first...
    QOpenGLWidget::wheelEvent(event);
    if (event->isAccepted()) {
        return;
    }

    // Pass zoom event to renderer
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    d->performZooming(numSteps);

    event->accept();
}

void PointCloudVisualizationWidget::keyPressEvent (QKeyEvent *event)
{
    Q_D(PointCloudVisualizationWidget);

    // Chain up to parent first...
    QOpenGLWidget::keyPressEvent(event);
    if (event->isAccepted()) {
        return;
    }

    // Don't catch key input with ctrl modifier ...
    if (event->modifiers() & Qt::ControlModifier) {
        event->ignore();
        return;
    }

    switch (event->key()) {
        // Reset position and orientation - default view
        case Qt::Key_5: {
            d->resetView();
            break;
        }

        // Rotation keys
        case Qt::Key_1: {
            d->addRotationVelocity(0.0f, 0.0f, -1.0f);
            break;
        }
        case Qt::Key_3: {
            d->addRotationVelocity(0.0f, 0.0f, 1.0f);
            break;
        }

        case Qt::Key_4: {
            d->addRotationVelocity(0.0f, -1.0f, 0.0f);
            break;
        }
        case Qt::Key_6: {
            d->addRotationVelocity(0.0f, 1.0f, 0.0f);
            break;
        }

        case Qt::Key_8: {
            d->addRotationVelocity(1.0f, 0.0f, 0.0f);
            break;
        }
        case Qt::Key_2: {
            d->addRotationVelocity(-1.0f, 0.0f, 0.0f);
            break;
        }

        case Qt::Key_X: {
            d->switchRotationAxis();
            break;
        }

        // Zoom keys
        case Qt::Key_Plus: {
            d->performZooming(1);
            break;
        }

        case Qt::Key_Minus: {
            d->performZooming(-1);
            break;
        }

        // Strafe keys
        case Qt::Key_A: {
            d->addTranslationVelocity(0.05f, 0.0f, 0.0f);
            break;
        }

        case Qt::Key_D: {
            d->addTranslationVelocity(-0.05f, 0.0f, 0.0f);
            break;
        }

        case Qt::Key_W: {
            d->addTranslationVelocity(0.0f, -0.05f, 0.0f);
            break;
        }

        case Qt::Key_S: {
            d->addTranslationVelocity(0.0f, 0.05f, 0.0f);
            break;
        }

        default: {
            event->ignore();
            return;
        }
    }

    // If event was processed, mark it as accepted and signal position change
    event->accept();
}


// *********************************************************************
// *              Data initialization, upload and rendering            *
// *********************************************************************
void PointCloudVisualizationWidgetPrivate::initializePointCloud ()
{
    // Shader program
    shaderProgramBasicElement.compile();
    shaderProgramBasicElement.bind();

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
    shaderProgramBasicElement.setVertex(GL_FLOAT, 0*sizeof(float), 3, 6*sizeof(float));
    shaderProgramBasicElement.setColor(GL_FLOAT, 3*sizeof(float), 3, 6*sizeof(float));
}

void PointCloudVisualizationWidgetPrivate::initializeTrackBall ()
{
    // Shader program
    shaderProgramVolumetricLine.compile();
    shaderProgramVolumetricLine.bind();

    // Create VAO
    vaoCircle.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vaoCircle);

    // Create VBO
    numCircleVertices = 30;

    if (!vboCircle.create()) {
        qWarning() << "Failed to create circle VBO!";
    }
    vboCircle.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboCircle.bind();

    // Generate vertices for circle (this assumes that the circle will
    // be drawn using GL_LINE_STRIP_ADJACENCY, hence we start at i = -1
    // and end at numVertices+1)
    vboCircle.allocate((numCircleVertices+3)*2*sizeof(float));
    float *bufferPtr = static_cast<float *>(vboCircle.map(QOpenGLBuffer::WriteOnly));

    for (int i = -1; i <= numCircleVertices+1; i++) {
        float portion = float(i) / float(numCircleVertices) * 2*M_PI;
        *bufferPtr++ = std::cos(portion);
        *bufferPtr++ = std::sin(portion);
    }
    vboCircle.unmap();

    numCircleVertices += 3;

    // Set up attribute buffers
    shaderProgramVolumetricLine.setVertex(GL_FLOAT, 0*sizeof(float), 2);
}


void PointCloudVisualizationWidgetPrivate::uploadPointCloud ()
{
    if (!freshData) {
        return;
    }

    // Validate data
    if (image.empty() || points.empty()) {
        return;
    }

    if (image.rows != points.rows || image.cols != points.cols) {
        freshData = false;
        return;
    }

    // Compute number of points
    numPoints = points.rows*points.cols;

    // Bind
    vboPoints.bind();

    // Resize buffer if necessary
    int bufferSize = numPoints * 6*sizeof(float);
    if (vboPoints.size() != bufferSize) {
        vboPoints.allocate(bufferSize);
    }

    // Map
    float *ptr = static_cast<float *>(vboPoints.map(QOpenGLBuffer::WriteOnly));
    numPoints = 0;

    // Fill
    if (image.channels() == 3) {
        // Three-channel (BGR) image
        for (int y = 0; y < image.rows; y++) {
            const cv::Vec3b *imagePtr = image.ptr<cv::Vec3b>(y);
            const cv::Vec3f *pointsPtr = points.ptr<cv::Vec3f>(y);

            for (int x = 0; x < image.cols; x++) {
                const cv::Vec3b &bgr = imagePtr[x];
                const cv::Vec3f &xyz = pointsPtr[x];

                if (std::isfinite(xyz[0]) && std::isfinite(xyz[1]) && std::isfinite(xyz[2])) {
                    *ptr++ = xyz[0]/1000.0;
                    *ptr++ = xyz[1]/1000.0;
                    *ptr++ = xyz[2]/1000.0;
                    *ptr++ = bgr[2]/255.0f;
                    *ptr++ = bgr[1]/255.0f;
                    *ptr++ = bgr[0]/255.0f;

                    numPoints++;
                }
            }
        }
    } else {
        // Single-channel (grayscale) image
        for (int y = 0; y < image.rows; y++) {
            const unsigned char *imagePtr = image.ptr<unsigned char>(y);
            const cv::Vec3f *pointsPtr = points.ptr<cv::Vec3f>(y);

            for (int x = 0; x < image.cols; x++) {
                const unsigned char &gray = imagePtr[x];
                const cv::Vec3f &xyz = pointsPtr[x];

                if (std::isfinite(xyz[0]) && std::isfinite(xyz[1]) && std::isfinite(xyz[2])) {
                    *ptr++ = xyz[0]/1000.0;
                    *ptr++ = xyz[1]/1000.0;
                    *ptr++ = xyz[2]/1000.0;
                    *ptr++ = gray/255.0f;
                    *ptr++ = gray/255.0f;
                    *ptr++ = gray/255.0f;

                    numPoints++;
                }
            }
        }
    }

    vboPoints.unmap();

    // Mark as updated
    freshData = false;
}

void PointCloudVisualizationWidgetPrivate::renderPointCloud (QOpenGLFunctions *glFunctions)
{
    QMatrix4x4 viewMatrix;
    viewMatrix.translate(position);
    viewMatrix.rotate(orientation.conjugate()); // Conjugate = inverse rotation

    QOpenGLVertexArrayObject::Binder vaoBinder(&vaoPointCloud);
    shaderProgramBasicElement.bind();
    shaderProgramBasicElement.setPvmMatrix(projectionMatrix*viewMatrix);

    glFunctions->glDrawArrays(GL_POINTS, 0, numPoints);

    shaderProgramBasicElement.release();
}

void PointCloudVisualizationWidgetPrivate::renderTrackBall (QOpenGLFunctions *glFunctions)
{
    if (!rotationActive) {
        return;
    }

    QMatrix4x4 modelMatrix;

    QOpenGLVertexArrayObject::Binder vaoBinder(&vaoCircle);
    shaderProgramVolumetricLine.bind();

    modelMatrix.scale(trackBallRadius, trackBallRadius);
    if (rotationConstraint == ConstraintObjectAxes) {
        modelMatrix.rotate(orientation.conjugate()); // Conjugate = inverse rotation
    }

    int backupRGB, backupA;
    glFunctions->glGetIntegerv(GL_BLEND_EQUATION_RGB, &backupRGB);
    glFunctions->glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &backupA);
    glFunctions->glBlendEquationSeparate(GL_MAX, GL_MAX);

    glFunctions->glDisable(GL_DEPTH_TEST);
    shaderProgramVolumetricLine.setLineWidth(0.01f);

    const QColor defaultColor = QColor::fromRgbF(1.0f, 1.0f, 1.0f, 0.25f);

    if (rotationConstraint == ConstraintNoAxes) {
        // "Spherical zone" controller
        shaderProgramVolumetricLine.setColor(defaultColor);
        shaderProgramVolumetricLine.setPvmMatrix(modelMatrix);
        glFunctions->glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, numCircleVertices);
    } else {
        const QColor activeColor = QColor::fromRgbF(1.0f, 1.0f, 1.0f, 0.50f);

        // Three perpendicular circles
        shaderProgramVolumetricLine.setColor((rotationAxisIndex == 2) ? activeColor : defaultColor);
        shaderProgramVolumetricLine.setPvmMatrix(modelMatrix);
        glFunctions->glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, numCircleVertices);

        modelMatrix.rotate(90.0f, 1.0f, 0.0f, 0.0f);
        shaderProgramVolumetricLine.setColor((rotationAxisIndex == 1) ? activeColor : defaultColor);
        shaderProgramVolumetricLine.setPvmMatrix(modelMatrix);
        glFunctions->glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, numCircleVertices);

        modelMatrix.rotate(90.0f, 0.0f, 1.0f, 0.0f);
        shaderProgramVolumetricLine.setColor((rotationAxisIndex == 0) ? activeColor : defaultColor);
        shaderProgramVolumetricLine.setPvmMatrix(modelMatrix);
        glFunctions->glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, numCircleVertices);
    }

    // Restore OpenGL state
    glFunctions->glEnable(GL_DEPTH_TEST);
    glFunctions->glBlendEquationSeparate(backupRGB, backupA);

    shaderProgramVolumetricLine.release();
}


// *********************************************************************
// *                        Viewpoint movement                         *
// *********************************************************************
void PointCloudVisualizationWidgetPrivate::resetView ()
{
    Q_Q(PointCloudVisualizationWidget);

    // Reset position and orientation
    orientation = defaultOrientation;
    position = defaultPosition;

    // Reset the movement, as well
    translationVelocity = QVector3D();
    rotationVelocity = QVector3D();

    q->update();
}

void PointCloudVisualizationWidgetPrivate::addRotationVelocity (float vx, float vy, float vz)
{
    rotationVelocity += QVector3D(vx, vy, vz);
}

void PointCloudVisualizationWidgetPrivate::addTranslationVelocity (float vx, float vy, float vz)
{
    translationVelocity += QVector3D(vx, vy, vz);
}

void PointCloudVisualizationWidgetPrivate::switchRotationAxis ()
{
    if (rotationActive && rotationConstraint != ConstraintNoAxes) {
        // There's three camera/object axes to cycle through
        rotationAxisIndex = (rotationAxisIndex + 1) % 3;
    }
}


void PointCloudVisualizationWidgetPrivate::beginRotation (const QPointF &pos, RotationConstraint constraint)
{
    // Store rotation constraint type
    rotationConstraint = constraint;

    // Transform to [-1,1] range
    prevRotationPos = QPointF((2*pos.x()- width) / width, (height - 2*pos.y()) / height);

    rotationActive = true;
}

void PointCloudVisualizationWidgetPrivate::doRotation (const QPointF &pos)
{
    Q_Q(PointCloudVisualizationWidget);

    // Transform to [-1,1] range
    QPointF curRotationPos((2*pos.x() - width) / width, (height - 2*pos.y()) / height);

    QVector3D vFrom(prevRotationPos);
    QVector3D vTo(curRotationPos);

    if (rotationActive) {
        // Find the two points on sphere according to the projection method
        projectOnSphere(vFrom);
        projectOnSphere(vTo);

        // Modify the vectors according to the active axes constraint
        switch (rotationConstraint) {
            case ConstraintObjectAxes: {
                vFrom = constrainToAxis(vFrom, objectAxes[rotationAxisIndex]);
                vTo = constrainToAxis(vTo, objectAxes[rotationAxisIndex]);
                break;
            }
            case ConstraintCameraAxes: {
                vFrom = constrainToAxis(vFrom, cameraAxes[rotationAxisIndex]);
                vTo = constrainToAxis(vTo, cameraAxes[rotationAxisIndex]);
                break;
            }
            default: {
                /* Nothing to do */
                break;
            }
        }

        // Normalize vectors
        vFrom.normalize();
        vTo.normalize();

        // Get the rotation quaternion
        orientation *= rotationFromMove(vFrom, vTo);

        prevRotationPos = curRotationPos;

        // Update object axes
        QMatrix4x4 rotationMatrix;
        rotationMatrix.rotate(orientation);
        rotationMatrix = rotationMatrix.transposed(); // This transpose is actually an inverse!

        objectAxes[0] = rotationMatrix.column(0).toVector3D();
        objectAxes[1] = rotationMatrix.column(1).toVector3D();
        objectAxes[2] = rotationMatrix.column(2).toVector3D();

        // Update the display
        q->update();
    } else if (rotationConstraint != ConstraintNoAxes) {
        projectOnSphere(vTo);
        rotationAxisIndex = findNearestConstraintAxis(vTo);
    }
}

void PointCloudVisualizationWidgetPrivate::endRotation ()
{
    Q_Q(PointCloudVisualizationWidget);

    rotationActive = false;
    prevRotationPos = QPointF(0.0f, 0.0f);

    // Update the display
    q->update();
}

void PointCloudVisualizationWidgetPrivate::performZooming (int steps)
{
    Q_Q(PointCloudVisualizationWidget);
    translationVelocity += QVector3D(0.0f, 0.0f, steps*0.05f);
    q->update();
}


void PointCloudVisualizationWidgetPrivate::performMovement ()
{
    Q_Q(PointCloudVisualizationWidget);

    bool update = false;

    // Update position (if velocity isn't zero)
    if (translationVelocity.length()) {
        position += translationVelocity;
        update = true;
    }

    // Update rotation
    if (rotationVelocity.length()) {
        orientation *= QQuaternion::fromAxisAndAngle(1.0, 0.0, 0.0, rotationVelocity.x());
        orientation *= QQuaternion::fromAxisAndAngle(0.0, 1.0, 0.0, rotationVelocity.y());
        orientation *= QQuaternion::fromAxisAndAngle(0.0, 0.0, 1.0, rotationVelocity.z());
        update = true;
    }

    // Update translatory velocity
    translationVelocity += QVector3D(translationVelocity.x()*translationAcceleration.x(),
                                     translationVelocity.y()*translationAcceleration.y(),
                                     translationVelocity.z()*translationAcceleration.z());

    // If component is less than 1e-6, round it to zero
    if (fabs(translationVelocity.x()) < 1e-6) translationVelocity.setX(0.0);
    if (fabs(translationVelocity.y()) < 1e-6) translationVelocity.setY(0.0);
    if (fabs(translationVelocity.z()) < 1e-6) translationVelocity.setZ(0.0);

    // Update rotational velocity
    rotationVelocity += QVector3D(rotationVelocity.x()*rotationAcceleration.x(),
                                  rotationVelocity.y()*rotationAcceleration.y(),
                                  rotationVelocity.z()*rotationAcceleration.z());

    if (fabs(rotationVelocity.x()) < 1e-6) rotationVelocity.setX(0.0);
    if (fabs(rotationVelocity.y()) < 1e-6) rotationVelocity.setY(0.0);
    if (fabs(rotationVelocity.z()) < 1e-6) rotationVelocity.setZ(0.0);

    // Signal that position/orientation changed
    if (update) {
        q->update();
    }
}


// *********************************************************************
// *                 Projection-on-sphere mathematics                  *
// *********************************************************************
void PointCloudVisualizationWidgetPrivate::projectOnSphere (QVector3D &v) const
{
    float rsqr = trackBallRadius*trackBallRadius;
    float dsqr = v.x()*v.x() + v.y()*v.y();
    float z;

    // If relatively "inside" the sphere, then project to the sphere,
    // otherwise to a hyperbolic sheet
    if (dsqr < 0.5f*rsqr) {
        z = std::sqrt(rsqr - dsqr);
    } else {
        z = rsqr / (2 * std::sqrt(dsqr));
    }

    v.setZ(z);
}

QQuaternion PointCloudVisualizationWidgetPrivate::rotationFromMove (const QVector3D &vFrom, const QVector3D &vTo) const
{
    QQuaternion q;

    QVector3D rotAxis;
    float t, angle;

    // Rotation axis
    rotAxis = QVector3D::crossProduct(vTo, vFrom);

    // Rotation angle
    t = QVector3D::dotProduct(vTo, vFrom);
    // Clamp due to possible numerical overflow
    t = std::min(std::max(-1.0f, t), 1.0f);
    // Prevent flips, which probably happen due to numerical issues; we
    // know the angle must be always positive, and so should t...
    if (t < 0) {
        t = -t;
    }

    angle = std::acos(t) * 180.0f/M_PI;

    // Create quaternion
    q = QQuaternion::fromAxisAndAngle(rotAxis, angle);
    q.normalize(); // Make it a unit quaternion

    return q;
}

QVector3D PointCloudVisualizationWidgetPrivate::constrainToAxis (const QVector3D &vector, const QVector3D &axis) const
{
    QVector3D onPlane;
    float norm;

    onPlane = vector - axis * QVector3D::dotProduct(vector, axis);
    norm = onPlane.length();

    if (norm > 0.0f) {
        if (onPlane.z() < 0.0f) {
            onPlane = -onPlane;
        }
        onPlane /= sqrt(norm);

        return onPlane;
    }

    if (axis.z() == 1) {
        onPlane = QVector3D(1.0f, 0.0f, 0.0f);
    } else {
        onPlane = QVector3D(-axis.y(), axis.x(), 0.0f);
        onPlane.normalize();
    }

    return onPlane;
}

int PointCloudVisualizationWidgetPrivate::findNearestConstraintAxis (const QVector3D &vector) const
{
    QVector3D onPlane;
    int nearest = 0;
    float max = -1.0f;
    float dot;

    for (int i = 0; i < 3; i++) {
        // Select constraint axis
        switch (rotationConstraint) {
            case ConstraintObjectAxes: {
                onPlane = constrainToAxis(vector, objectAxes[i]);
                break;
            }
            case ConstraintCameraAxes: {
                onPlane = constrainToAxis(vector, cameraAxes[i]);
                break;
            }
            default: {
                // Should not happen
                break;
            }
        }

        // Compute dot product to find closest constraint axis
        dot = QVector3D::dotProduct(vector, onPlane);
        if (dot > max) {
            max = dot;
            nearest = i;
        }
    }

    return nearest;
}


// *********************************************************************
// *                   Shader program: basic element                   *
// *********************************************************************
void ShaderProgramBasicElement::compile ()
{
    // Compile shader program
    addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/basic_element.vert");
    addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/basic_element.frag");
    if (!link()) {
        qWarning() << this << "Failed to link basic element shader program!";
    }

    // Cache attribute and uniform locations
    attributeVertex = attributeLocation("vertex");
    attributeColor = attributeLocation("color");
    uniformPvm = uniformLocation("pvm");
}

void ShaderProgramBasicElement::setPvmMatrix (const QMatrix4x4 &pvm)
{
    setUniformValue(uniformPvm, pvm);
}

void ShaderProgramBasicElement::setVertex (GLenum type, int offset, int tupleSize, int stride)
{
    setAttributeBuffer(attributeVertex, type, offset, tupleSize, stride);
    enableAttributeArray(attributeVertex);
}

void ShaderProgramBasicElement::setColor (GLenum type, int offset, int tupleSize, int stride)
{
    setAttributeBuffer(attributeColor, type, offset, tupleSize, stride);
    enableAttributeArray(attributeColor);
}

void ShaderProgramBasicElement::setColor (const QColor &color)
{
    setAttributeValue(attributeColor, color);
    disableAttributeArray(attributeColor);
}


// *********************************************************************
// *                  Shader program: volumetric line                  *
// *********************************************************************
void ShaderProgramVolumetricLine::compile ()
{
    // Compile shader program
    addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/volumetric_line.vert");
    addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/volumetric_line.geom");
    addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/volumetric_line.frag");
    if (!link()) {
        qWarning() << this << "Failed to link volumetric line shader program!";
    }

    // Cache attribute and uniform locations
    attributeVertex = attributeLocation("vertex");
    uniformPvm = uniformLocation("pvm");
    uniformRadius = uniformLocation("radius");
    uniformColor = uniformLocation("color");
}

void ShaderProgramVolumetricLine::setVertex (GLenum type, int offset, int tupleSize, int stride)
{
    setAttributeBuffer(attributeVertex, type, offset, tupleSize, stride);
    enableAttributeArray(attributeVertex);
}


void ShaderProgramVolumetricLine::setPvmMatrix (const QMatrix4x4 &pvm)
{
    setUniformValue(uniformPvm, pvm);
}

void ShaderProgramVolumetricLine::setLineWidth (float width)
{
    setUniformValue(uniformRadius, width);
}

void ShaderProgramVolumetricLine::setColor (const QColor &color)
{
    setUniformValue(uniformColor, color);
}


} // Widgets
} // StereoToolbox
} // MVL
