// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet
  Created on: 2017-11-30

  (C) Copyright 2017-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#include <eigenqt/Vector3DWidget.h>
#include <QKeyEvent>
#include <QPainter>
#include <QtGlobal>
#include <eigenqt/detail/Constants.h>
#include <cassert>
#include <cmath>

#include <eigenqt/OpenGLBaseWidget.h>

namespace EigenQt {

const double vctVector3DQtWidgetDefaultMax = 0.0001;

Vector3DWidget::Vector3DWidget(QWidget *parent)
    : OpenGLBaseWidget(parent), mVector(0.0, 0.0, 0.0), mVectorNorm(0.0)
{
    this->setFocusPolicy(Qt::StrongFocus);
    this->setToolTip(QString("'a' to turn on/off autoresize\n'r' to reset autoresize if already in "
                             "autoresize mode\n'z' to reset orientation"));

    setContentsMargins(0, 0, 0, 0);
    setAutoResize(true);
    mAxisLength = 0.5;
    resetOrientation();
}

void Vector3DWidget::resetOrientation()
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

void Vector3DWidget::setAutoResize(const bool autoResize)
{
    if (autoResize) {
        // re-initialize to default, i.e. fairly small force
        mMaxNorm = vctVector3DQtWidgetDefaultMax;
        mScale = 1.0f / mMaxNorm;
    }
    mAutoResize = autoResize;
}

void Vector3DWidget::setValue(const Eigen::Vector3d &value)
{
    mVector = value;
    mVectorNorm = mVector.norm();
    if (mAutoResize && (mVectorNorm > mMaxNorm)) {
        mMaxNorm = mVectorNorm;
        mScale = 1.0f / mMaxNorm;
    }
    // update GL display
    update();
}

void Vector3DWidget::mousePressEvent(QMouseEvent *event)
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

void Vector3DWidget::mouseMoveEvent(QMouseEvent *event)
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

void Vector3DWidget::mouseReleaseEvent(QMouseEvent *)
{
    mStartMousePosition.fill(0);
    mDragging = false;
    // save current rotation
    mCurrentOrientation = mDeltaOrientation * mCurrentOrientation;
    mDeltaOrientation = Eigen::Quaterniond::Identity();
}

void Vector3DWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_A:
        // toggle autoresize
        setAutoResize(!mAutoResize);
        update();
        break;
    case Qt::Key_R:
        // reset autoresize if already in autoresize mode
        if (mAutoResize) {
            setAutoResize(true);
            update();
        }
        break;
    case Qt::Key_Z:
        resetOrientation();
        update();
        break;
    default:
        break;
    }
}

void Vector3DWidget::initializeGL()
{
    const QColor bgColor = palette().color(QPalette::Base);
    glClearColor(bgColor.redF(), bgColor.greenF(), bgColor.blueF(), 1.0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
}

void Vector3DWidget::paintGL()
{
    resizeGL(width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // disable lighting
    glDisable(GL_LIGHTING);

    // set line width
    glLineWidth(2.0);

    // gl transformation here
    // x+:left  y+:up   z+: point out screen
    glTranslatef(0.0f, 0.0f, -10.0f);
    Eigen::AngleAxisd rot(mDeltaOrientation * mCurrentOrientation);
    glRotated(rot.angle() * detail::degreesPerRadian, rot.axis().x(), rot.axis().y(),
              rot.axis().z());

    // draw X/Y/Z axis, using half
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(mAxisLength, 0.0f, 0.0f);
    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, mAxisLength, 0.0f);
    glEnd();

    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, mAxisLength);
    glEnd();

    glPushMatrix();

    if (mAutoResize) {
        glScalef(mScale, mScale, mScale);
    }

    // draw line for force
    const QColor textColor = palette().color(QPalette::Text);
    glColor3f(textColor.redF(), textColor.greenF(), textColor.blueF());
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(mVector.x(), mVector.y(), mVector.z());
    glEnd();

    // draw X/Y/Z projections
    glEnable(GL_LINE_STIPPLE);

    glColor3f(1.0f, 0.0f, 0.0f);
    if (mVector.x() > 0.0) {
        glLineStipple(1, 0x00FF); // dashed
    } else {
        glLineStipple(1, 0x0101); // dotted
    }
    glBegin(GL_LINE_STRIP);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(mVector.x(), 0.0f, 0.0f);
    glVertex3f(mVector.x(), mVector.y(), 0.0f);
    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);
    if (mVector.y() > 0.0) {
        glLineStipple(1, 0x00FF); // dashed
    } else {
        glLineStipple(1, 0x0101); // dotted
    }

    glBegin(GL_LINE_STRIP);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, mVector.y(), 0.0f);
    glVertex3f(mVector.x(), mVector.y(), 0.0f);
    glEnd();

    glColor3f(0.0f, 0.0f, 1.0f);
    if (mVector.z() > 0.0) {
        glLineStipple(1, 0x00FF); // dashed
    } else {
        glLineStipple(1, 0x0101); // dotted
    }
    glBegin(GL_LINE_LOOP);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, mVector.z());
    glVertex3f(mVector.x(), mVector.y(), mVector.z());
    glVertex3f(mVector.x(), mVector.y(), 0.0f);
    glEnd();

    glDisable(GL_LINE_STIPPLE);

    glPopMatrix();

    const int font_size = 8;
    QPainter painter(this);
    const QColor txtColor = palette().color(QPalette::Text);
    painter.setPen(txtColor);
    painter.setFont(QFont("Helvetica", font_size));
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.drawText(1, 1 + font_size, QString::asprintf("%0.2f/%0.2f", mVectorNorm, mMaxNorm));
    painter.end();

    glFlush();
}

void Vector3DWidget::resizeGL(int width, int height)
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

} // namespace EigenQt
