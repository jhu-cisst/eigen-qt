// SPDX-License-Identifier: MIT

/*
  Author(s):  Zihan Chen, Anton Deguet
  Created on: 2013-03-20

  (C) Copyright 2013-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#include <unsupported/Eigen/EulerAngles>

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>

#include <eigenqt/RotationWidget.h>

// all these widgets should be replaced to use static vectors/matrices
#include <eigenqt/VectorWidget.h>
#include <eigenqt/MatrixWidget.h>
#include <eigenqt/detail/Constants.h>
#include <cassert>
#include <cmath>

namespace EigenQt {

// =============================================
// RotationView
// =============================================

RotationView::RotationView(QWidget *parent)
    : OpenGLBaseWidget(parent), mRotation(Eigen::Matrix3d::Identity())
{
    setFocusPolicy(Qt::StrongFocus);
    setToolTip(QString("'z' to reset orientation"));
    setMinimumHeight(100);
    setMinimumWidth(100);
    setContentsMargins(0, 0, 0, 0);
    resetOrientation();
}

void RotationView::resetOrientation()
{
    // start with default Z up, x toward left, y towards right
    mCurrentOrientation =
        Eigen::Quaterniond(Eigen::AngleAxisd(0.1 * detail::pi, Eigen::Vector3d::UnitX()) *
                           Eigen::AngleAxisd(-0.75 * detail::pi, Eigen::Vector3d::UnitY()) *
                           Eigen::AngleAxisd(-0.5 * detail::pi, Eigen::Vector3d::UnitX()));
    mStartMousePosition = Eigen::Vector2i::Zero();
    mDeltaOrientation = Eigen::Quaterniond::Identity();
    mDragging = false;
}

void RotationView::setValue(const Eigen::Matrix3d &rot)
{
    mRotation = rot;
    // update GL display
    update();
}

void RotationView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        mStartMousePosition = Eigen::Vector2i(event->position().x(), event->position().y());
#else
        mStartMousePosition = Eigen::Vector2i(event->x(), event->y());
#endif
        mDragging = true;
    }
}

void RotationView::mouseMoveEvent(QMouseEvent *event)
{
    const double sensitivity = 0.01;
    if (event->buttons() & Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const Eigen::Vector2i newMousePosition(event->position().x(), event->position().y());
#else
        const Eigen::Vector2i newMousePosition(event->x(), event->y());
#endif
        if (mDragging) {
            const Eigen::Vector2d deltaMouse =
                sensitivity * (newMousePosition - mStartMousePosition).cast<double>();
            Eigen::Vector3d rodrigues(deltaMouse.y(), deltaMouse.x(), 0.0);
            if (rodrigues.norm() > 0.0) {
                mDeltaOrientation =
                    Eigen::Quaterniond(Eigen::AngleAxis(rodrigues.norm(), rodrigues.normalized()));
            } else {
                mDeltaOrientation = Eigen::Quaterniond::Identity();
            }
            update();
        } else {
            mStartMousePosition = newMousePosition;
            mDragging = true;
        }
    }
}

void RotationView::mouseReleaseEvent(QMouseEvent *)
{
    mStartMousePosition = Eigen::Vector2i::Zero();
    mDragging = false;
    // save current rotation
    mCurrentOrientation = mDeltaOrientation * mCurrentOrientation;
    mDeltaOrientation = Eigen::Quaterniond::Identity();
}

void RotationView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Z:
        resetOrientation();
        update();
        break;
    default:
        break;
    }
}

void RotationView::initializeGL()
{
    const QColor bgColor = palette().color(QPalette::Base);
    glClearColor(bgColor.redF(), bgColor.greenF(), bgColor.blueF(), 1.0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
}

void RotationView::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // disable lighting
    glDisable(GL_LIGHTING);

    // set line width
    glLineWidth(2.0);

    // user orientation
    glTranslatef(0.0f, 0.0f, -10.0f);
    Eigen::AngleAxisd rot(mDeltaOrientation * mCurrentOrientation);
    glRotated(rot.angle() * detail::degreesPerRadian, rot.axis().x(), rot.axis().y(),
              rot.axis().z());

    // draw reference coordinate frame here
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x00FF); // dashed

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glEnd();

    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glEnd();

    glDisable(GL_LINE_STIPPLE);

    // orientation
    const Eigen::AngleAxisd currentRotation(mRotation);
    glRotated(currentRotation.angle() * detail::degreesPerRadian, currentRotation.axis().x(),
              currentRotation.axis().y(), currentRotation.axis().z());

    // draw current frame
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glEnd();

    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glEnd();

    glFlush();
}

void RotationView::resizeGL(int width, int height)
{
    const int w = std::max(width, 1);
    const int h = std::max(height, 1);
    const qreal ratio = devicePixelRatioF();
    glViewport(0, 0, qRound(w * ratio), qRound(h * ratio));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    GLdouble dWt;
    GLdouble dHt;

    if (w > h) {
        dHt = 1.0;
        dWt = static_cast<double>(w) / static_cast<double>(h);
    } else {
        dHt = static_cast<double>(h) / static_cast<double>(w);
        dWt = 1.0;
    }

    glOrtho(-dWt, dWt, -dHt, dHt, 8.0, 12.0);

    glMatrixMode(GL_MODELVIEW);
}

// =============================================
// RotationWidget
// =============================================
RotationWidget::RotationWidget(const DisplayMode displayMode, QWidget *parent)
    : QWidget(parent), mDisplayMode(Undefined), Rotation(Eigen::Matrix3d::Identity()),
      CurrentWidget(0), mRevoluteFactor(detail::degreesPerRadian)
{
    qRegisterMetaType<EigenQt::RotationWidget::DisplayMode>("EigenQt::RotationWidget::DisplayMode");
    // Matrix Groupbox
    MatrixWidget = new MatrixView(this);
    MatrixWidget->setMinimumSize(80, 80);
    MatrixWidget->resize(MatrixWidget->sizeHint());

    // Axis Angle Groupbox
    AxisWidget = new VectorView(this);
    AngleWidget = new VectorView(this);

    QGridLayout *axisAngleLayout = new QGridLayout;
    axisAngleLayout->setSpacing(1);
    axisAngleLayout->setContentsMargins(0, 0, 0, 0);
    axisAngleLayout->addWidget(new QLabel("Axis"), 0, 0, 1, 1);
    axisAngleLayout->addWidget(AxisWidget, 0, 1, 1, 1);
    axisAngleLayout->addWidget(new QLabel("Angle"), 1, 0, 1, 1);
    axisAngleLayout->addWidget(AngleWidget, 1, 1, 1, 1);

    AxisAngleWidget = new QWidget(this);
    AxisAngleWidget->setLayout(axisAngleLayout);

    // Quaternion Groupbox
    QuaternionWidget = new VectorView(this);

    // Euler ZYZ Groupbox
    EulerZYZWidget = new VectorView(this);

    // Euler ZYX Groupbox
    EulerZYXWidget = new VectorView(this);

    // Visualization Groupbox
    OpenGLWidget = new RotationView(this);
    OpenGLWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    MatrixWidget->hide();
    AxisAngleWidget->hide();
    QuaternionWidget->hide();
    EulerZYZWidget->hide();
    EulerZYXWidget->hide();
    OpenGLWidget->hide();

    Layout = new QVBoxLayout;
    Layout->setSpacing(1);
    Layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(Layout);
    this->setWindowTitle(tr("Rotation"));

    // Set display mode
    setDisplayMode(displayMode);

    // Contextual menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &RotationWidget::showContextMenu);
    setToolTip(QString("right click for menu"));
}

void RotationWidget::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = this->mapToGlobal(pos);
    QMenu menu;
    QAction *matrix = new QAction("Matrix", &menu);
    QAction *axisAngle = new QAction("Axis Angle", &menu);
    QAction *quaternion = new QAction("Quaternion", &menu);
    QAction *eulerZYZ = new QAction("Euler ZYZ", &menu);
    QAction *eulerZYX = new QAction("Euler ZYX", &menu);
    QAction *openGL = new QAction("3D", &menu);
    if (mDisplayMode != RotationMatrix) {
        menu.addAction(matrix);
    }
    if (mDisplayMode != AxisAngle) {
        menu.addAction(axisAngle);
    }
    if (mDisplayMode != Quaternion) {
        menu.addAction(quaternion);
    }
    if (mDisplayMode != EulerZYZ) {
        menu.addAction(eulerZYZ);
    }
    if (mDisplayMode != EulerZYX) {
        menu.addAction(eulerZYX);
    }
    if (mDisplayMode != OpenGL) {
        menu.addAction(openGL);
    }

    QAction *selectedItem = menu.exec(globalPos);
    if (selectedItem) {
        if (selectedItem == matrix) {
            setDisplayMode(RotationMatrix);
        } else if (selectedItem == axisAngle) {
            setDisplayMode(AxisAngle);
        } else if (selectedItem == quaternion) {
            setDisplayMode(Quaternion);
        } else if (selectedItem == eulerZYZ) {
            setDisplayMode(EulerZYZ);
        } else if (selectedItem == eulerZYX) {
            setDisplayMode(EulerZYX);
        } else if (selectedItem == openGL) {
            setDisplayMode(OpenGL);
        }
    }
}

void RotationWidget::updateCurrentWidget()
{
    using OrderZYZ = Eigen::EulerSystem<Eigen::EULER_Z, Eigen::EULER_Y, Eigen::EULER_Z>;
    using OrderZYX = Eigen::EulerSystem<Eigen::EULER_Z, Eigen::EULER_Y, Eigen::EULER_X>;

    Eigen::AngleAxisd axis_angle;
    Eigen::Quaterniond quat;
    Eigen::EulerAngles<double, OrderZYZ> eulerZYZ;
    Eigen::EulerAngles<double, OrderZYX> eulerZYX;

    // compute the value based on the internal Rotation
    switch (mDisplayMode) {
    case RotationMatrix:
        MatrixWidget->setValue(Rotation);
        break;
    case AxisAngle:
        axis_angle = Eigen::AngleAxisd(Rotation);
        AxisWidget->setValue(axis_angle.axis());
        AngleWidget->setValue(Eigen::Vector<double, 1>(axis_angle.angle() * mRevoluteFactor));
        break;
    case Quaternion:
        quat = Eigen::Quaterniond(Rotation);
        QuaternionWidget->setValue(quat.coeffs());
        break;
    case OpenGL:
        OpenGLWidget->setValue(Rotation);
        break;
    case EulerZYZ:
        eulerZYZ = Eigen::EulerAngles<double, OrderZYZ>(Rotation);
        EulerZYZWidget->setValue(eulerZYZ.angles() * mRevoluteFactor);
        break;
    case EulerZYX:
        eulerZYX = Eigen::EulerAngles<double, OrderZYX>(Rotation);
        EulerZYXWidget->setValue(eulerZYX.angles() * mRevoluteFactor);
        break;
    default:
        break;
    }
}

void RotationWidget::setDisplayMode(const DisplayMode displayMode)
{
    // should never allow anyone to use undefined
    if (displayMode == Undefined) {
        return;
    }

    // if the mode is unchanged, nothing to do
    if (displayMode == this->mDisplayMode) {
        return;
    }

    // set the new display mode
    this->mDisplayMode = displayMode;

    // mostly for initialization, there was no widget defined prior this call
    if (CurrentWidget) {
        Layout->removeWidget(CurrentWidget);
        CurrentWidget->hide();
    }

    // set the new current widget, these have been created in the ctor
    switch (displayMode) {
    case RotationMatrix:
        CurrentWidget = MatrixWidget;
        break;
    case AxisAngle:
        CurrentWidget = AxisAngleWidget;
        break;
    case Quaternion:
        CurrentWidget = QuaternionWidget;
        break;
    case EulerZYZ:
        CurrentWidget = EulerZYZWidget;
        break;
    case EulerZYX:
        CurrentWidget = EulerZYXWidget;
        break;
    case OpenGL:
        CurrentWidget = OpenGLWidget;
        break;
    default:
        break;
    }
    Layout->addWidget(CurrentWidget);
    CurrentWidget->show();
    updateCurrentWidget();
    repaint();
    emit displayModeChanged(displayMode);
}

} // namespace EigenQt
