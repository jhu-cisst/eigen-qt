// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet
  Created on: 2013-04-20

  (C) Copyright 2013-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#ifndef EIGENQT_QTWIDGETFRAME_H
#define EIGENQT_QTWIDGETFRAME_H

#include <eigenqt/ForwardDeclarations.h>
#include <eigenqt/RotationWidget.h>
#include <eigenqt/VectorWidget.h>

#include <Eigen/Geometry>

#include <QWidget>

class QVBoxLayout;

#include <eigenqt/geometry_export.h>

#include <utility>

namespace EigenQt {

/*! Qt Widget to display a 3D frame using Eigen. */
class EIGENQT_GEOMETRY_EXPORT FrameWidget : public QWidget
{
    Q_OBJECT

public:
    /*! Define the display mode type based on the rotation display
      mode. */
    typedef RotationWidget RotationWidgetType;
    typedef RotationWidgetType::DisplayMode DisplayMode;
    Q_PROPERTY(EigenQt::RotationWidget::DisplayMode displayMode READ displayMode WRITE
                   setDisplayMode NOTIFY displayModeChanged)
    DisplayMode displayMode() const { return mRotationWidget->displayMode(); }

    /*! Constructor.  Default display mode is rotation matrix.  See
      also setDisplayMode. */
    explicit FrameWidget(const DisplayMode displayMode = RotationWidgetType::RotationMatrix,
                         QWidget *parent = nullptr);

    inline ~FrameWidget(){};

    /*! Set the frame value to be displayed.  This method assumes
      the rotation part is valid, i.e. normalized and will not
      perform any check or normalization.  Accepts Eigen 3D transforms
      (Isometry3, Affine3, AffineCompact3), 4x4 homogeneous matrices,
      or a (rotation, translation) pair. */
    inline void setValue(const Eigen::Isometry3d &frame)
    {
        mFrame = frame;
        mRotationWidget->setValue(frame.linear());
        TranslationWidget->setValue(frame.translation() * mPrismaticFactor);
    }

    template <class Scalar, int Mode, int Options>
    void setValue(const Eigen::Transform<Scalar, 3, Mode, Options> &frame)
    {
        Eigen::Isometry3d isometry = Eigen::Isometry3d::Identity();
        isometry.linear() = frame.linear().template cast<double>();
        isometry.translation() = frame.translation().template cast<double>();
        setValue(isometry);
    }

    template <class Rotation, class TranslationDeriv>
    void setValue(const Rotation &rotation, const Eigen::MatrixBase<TranslationDeriv> &translation)
    {
        Eigen::Isometry3d isometry = Eigen::Isometry3d::Identity();
        isometry.linear() = detail::toRotationMatrix(rotation);
        isometry.translation() = detail::toVector3d(translation);
        setValue(isometry);
    }

    template <class Derived> void setValue(const Eigen::MatrixBase<Derived> &homogeneous)
    {
        static_assert(
            (Derived::RowsAtCompileTime == 4 || Derived::RowsAtCompileTime == Eigen::Dynamic) &&
                (Derived::ColsAtCompileTime == 4 || Derived::ColsAtCompileTime == Eigen::Dynamic),
            "Homogeneous frame matrix must be 4x4");
        if (homogeneous.rows() != 4 || homogeneous.cols() != 4) {
            throw std::invalid_argument("Homogeneous frame matrix must be 4x4");
        }
        Eigen::Isometry3d isometry = Eigen::Isometry3d::Identity();
        isometry.linear() = homogeneous.template topLeftCorner<3, 3>().template cast<double>();
        isometry.translation() =
            homogeneous.template topRightCorner<3, 1>().template cast<double>();
        setValue(isometry);
    }

    Eigen::Isometry3d value() const { return mFrame; }

    /*! Set the display mode.  This method applies the display mode to
      the rotation widget. */
    inline void setDisplayMode(const DisplayMode displayMode)
    {
        mRotationWidget->setDisplayMode(displayMode);
    }

    inline void setPrismaticRevoluteFactors(const double &prismatic, const double &revolute)
    {
        mPrismaticFactor = prismatic;
        mRotationWidget->setPrismaticRevoluteFactors(prismatic, revolute);
        TranslationWidget->setValue(mFrame.translation() * mPrismaticFactor);
    }

signals:
    void displayModeChanged(EigenQt::RotationWidget::DisplayMode mode);

protected:
    Eigen::Isometry3d mFrame = Eigen::Isometry3d::Identity();
    // widgets
    RotationWidget *mRotationWidget;
    VectorView *TranslationWidget;
    // layout
    QVBoxLayout *Layout;
    // conversion factors
    double mPrismaticFactor;
};

} // namespace EigenQt

#endif // EIGENQT_QTWIDGETFRAME_H
