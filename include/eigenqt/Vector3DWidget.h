// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet
  Created on: 2017-11-30

  (C) Copyright 2017-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#ifndef EIGENQT_VECTOR3DQTWIDGET_H
#define EIGENQT_VECTOR3DQTWIDGET_H

#include <eigenqt/ForwardDeclarations.h>

#include <Eigen/Dense>

#include <eigenqt/geometry_export.h>

#include <eigenqt/OpenGLBaseWidget.h>

#include <utility>

namespace EigenQt {

/*!
  Widget to visualize a vector using 3 axes in OpenGL
*/
class EIGENQT_GEOMETRY_EXPORT Vector3DWidget : public OpenGLBaseWidget
{
    Q_OBJECT

public:
    Vector3DWidget(QWidget *parent = 0);
    inline ~Vector3DWidget(){};

    void setValue(const Eigen::Vector3d &value);
    void setAutoResize(const bool autoResize);
    inline const double &maximumNorm() const { return mMaxNorm; }
    inline float &axisLength() { return mAxisLength; }

protected:
    void resetOrientation();
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    Eigen::Vector3d mVector;
    Eigen::Quaterniond mCurrentOrientation, mDeltaOrientation;
    Eigen::Vector2i mStartMousePosition;
    bool mDragging = false;
    bool mAutoResize;
    double mVectorNorm, mMaxNorm;
    float mScale; // = 1 / mMaxNorm
    float mAxisLength;
};

} // namespace EigenQt

#endif // EIGENQT_VECTOR3DQTWIDGET_H
