// SPDX-License-Identifier: MIT

/*
  Author(s):  Zihan Chen, Anton Deguet
  Created on: 2013-03-20

  (C) Copyright 2013-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#ifndef EIGENQT_QTWIDGETROTATION_H
#define EIGENQT_QTWIDGETROTATION_H

#include <eigenqt/ForwardDeclarations.h>

#include <Eigen/Geometry>

class QVBoxLayout;

#include <eigenqt/geometry_export.h>

#include <eigenqt/OpenGLBaseWidget.h>

#include <stdexcept>
#include <utility>

namespace EigenQt {

namespace detail {
inline Eigen::Matrix3d toRotationMatrix(const Eigen::Matrix3d &rotation)
{
    return rotation;
}

template <class Derived>
Eigen::Matrix3d toRotationMatrix(const Eigen::RotationBase<Derived, 3> &rotation)
{
    return rotation.toRotationMatrix().template cast<double>();
}

template <class Derived>
Eigen::Matrix3d toRotationMatrix(const Eigen::MatrixBase<Derived> &rotation)
{
    static_assert(
        (Derived::RowsAtCompileTime == 3 || Derived::RowsAtCompileTime == Eigen::Dynamic) &&
            (Derived::ColsAtCompileTime == 3 || Derived::ColsAtCompileTime == Eigen::Dynamic),
        "Rotation matrix must be 3x3");
    if (rotation.rows() != 3 || rotation.cols() != 3) {
        throw std::invalid_argument("Rotation matrix must be 3x3");
    }
    return rotation.template cast<double>();
}

template <class Derived> Eigen::Vector3d toVector3d(const Eigen::MatrixBase<Derived> &vector)
{
    static_assert((Derived::RowsAtCompileTime == 3 && Derived::ColsAtCompileTime == 1) ||
                      (Derived::RowsAtCompileTime == 1 && Derived::ColsAtCompileTime == 3) ||
                      Derived::RowsAtCompileTime == Eigen::Dynamic ||
                      Derived::ColsAtCompileTime == Eigen::Dynamic,
                  "Vector must have 3 elements");
    if (vector.size() != 3 || (vector.rows() != 1 && vector.cols() != 1)) {
        throw std::invalid_argument("Vector must have 3 elements");
    }
    const auto eval = vector.template cast<double>().eval();
    return Eigen::Vector3d(eval(0), eval(1), eval(2));
}
} // namespace detail

/*!
  Widget to visualize rotation using 3 axes in OpenGL
*/
class EIGENQT_GEOMETRY_EXPORT RotationView : public OpenGLBaseWidget
{
    Q_OBJECT

public:
    explicit RotationView(QWidget *parent = nullptr);
    inline ~RotationView(){};

    void setValue(const Eigen::Matrix3d &rotation);

    template <class Derived> void setValue(const Eigen::RotationBase<Derived, 3> &rotation)
    {
        setValue(detail::toRotationMatrix(rotation));
    }

    template <class Derived> void setValue(const Eigen::MatrixBase<Derived> &rotation)
    {
        setValue(detail::toRotationMatrix(rotation));
    }

protected:
    void resetOrientation();
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    Eigen::Matrix3d mRotation;
    Eigen::Quaterniond mCurrentOrientation, mDeltaOrientation;
    Eigen::Vector2i mStartMousePosition;
    bool mDragging = false;
};

/*! Qt Widget to display a rotation using Eigen. */
class EIGENQT_GEOMETRY_EXPORT RotationWidget : public QWidget
{
    Q_OBJECT

public:
    /*! Possible display modes.  See setDisplayMode method.  Please
      note that Undefined should never be used. */
    typedef enum
    {
        Undefined,
        RotationMatrix,
        AxisAngle,
        Quaternion,
        EulerZYZ,
        EulerZYX,
        OpenGL
    } DisplayMode;
    Q_ENUM(DisplayMode)
    Q_PROPERTY(
        DisplayMode displayMode READ displayMode WRITE setDisplayMode NOTIFY displayModeChanged)
    DisplayMode displayMode() const { return mDisplayMode; }

    /*! Constructor.  Default display mode is rotation matrix.  See
      also setDisplayMode. */
    explicit RotationWidget(const DisplayMode displayMode = RotationMatrix,
                            QWidget *parent = nullptr);

    inline ~RotationWidget(){};

    /*! Set the rotation value to be displayed.  This method assumes
      the rotation is valid, i.e. normalized and will not perform any
      check or normalization.  Accepts Eigen 3x3 rotation matrices,
      quaternions, axis-angle, and Euler angles. */
    void setValue(const Eigen::Matrix3d &rotation)
    {
        Rotation = rotation;
        updateCurrentWidget();
    }

    template <class Derived> void setValue(const Eigen::RotationBase<Derived, 3> &rotation)
    {
        setValue(detail::toRotationMatrix(rotation));
    }

    template <class Derived> void setValue(const Eigen::MatrixBase<Derived> &rotation)
    {
        setValue(detail::toRotationMatrix(rotation));
    }

    /*! Set the display mode, i.e. the widget used to represent the
      rotation.  Options are rotation matrix, axis and angle, quaternion
      (displayed in order x, y, z, w), Euler angles (ZYZ or ZYX, in degrees),
      and 3D OpenGL based using red, green and blue axes.
      Please note that the display mode Undefined will be silently ignored. */
    void setDisplayMode(const DisplayMode displayMode);
    Eigen::Matrix3d value() const { return Rotation; }

signals:
    void displayModeChanged(EigenQt::RotationWidget::DisplayMode mode);

public:
    inline void setPrismaticRevoluteFactors(const double & /* prismatic */, const double &revolute)
    {
        mRevoluteFactor = revolute;
        updateCurrentWidget();
    }

protected slots:
    /*! Contextual menu showed when the user right clicks on the widget */
    void showContextMenu(const QPoint &position);

protected:
    /*! Current display mode, somewhat redundant with the
      CurrentWidget pointer.  The current widget should always be
      set by changing the mDisplayMode. */
    DisplayMode mDisplayMode;

    /*! Update the content of the current widget.  This method is
      called when the user provides a new rotation with setValue or
      when the widget used to display the rotation is changed using
      setDisplayMode. */
    void updateCurrentWidget();

    /*! Internal representation for the rotation to display. */
    Eigen::Matrix3d Rotation;

    // Matrix
    MatrixView *MatrixWidget;

    // Axis Angle
    VectorView *AxisWidget;
    VectorView *AngleWidget;
    QWidget *AxisAngleWidget;

    // Quaternion
    VectorView *QuaternionWidget;

    // Euler angles (in degrees)
    VectorView *EulerZYZWidget;
    VectorView *EulerZYXWidget;

    // Visualization
    RotationView *OpenGLWidget;

    // current widget
    QWidget *CurrentWidget;
    QVBoxLayout *Layout;

    // conversion factor
    double mRevoluteFactor;
};

} // namespace EigenQt

#endif // EIGENQT_QTWIDGETROTATION_H
