// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet
  Created on: 2020-09-04

  (C) Copyright 2020-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#ifndef EIGENQT_POSITIONSWIDGET_H
#define EIGENQT_POSITIONSWIDGET_H

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <QGridLayout>

#include <eigenqt/ForwardDeclarations.h>

#include <eigenqt/geometry_export.h>

#include <eigenqt/OpenGLBaseWidget.h>

class QTableWidget;
#include <utility>
#include <vector>

namespace EigenQt {

class PositionsWidgetView;

/*! Widget to visualize 3D positions by projecting along x, y or z
  axis.
*/
class EIGENQT_GEOMETRY_EXPORT PositionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PositionsWidget(QWidget *parent = nullptr);
    inline ~PositionsWidget() {}

    void setPrismaticRevoluteFactors(const double &prismatic, const double &revolute);

    /*! Add a position to plot using x, y and z coordinates. */
    void setValue(const Eigen::Vector3d &value);

    /*! Add a position using a frame. The rotation part is ignored. */
    inline void setValue(const Eigen::Isometry3d &value) { setValue(value.translation()); }

    template <class Scalar, int Mode, int Options>
    void setValue(const Eigen::Transform<Scalar, 3, Mode, Options> &value)
    {
        setValue(Eigen::Vector3d(value.translation().template cast<double>()));
    }

    void clear();

    typedef std::vector<Eigen::Vector3d> PosesType;
    typedef std::vector<PositionsWidgetView *> ViewsType;

protected:
    QGridLayout *mLayout;
    void keyPressEvent(QKeyEvent *event) override;

    QTableWidget *mTable;
    int mTableRow;
    ViewsType mViews;
    PosesType mPoses;
    Eigen::AlignedBox3d mBB;

    double mPrismaticFactor;
};

class EIGENQT_GEOMETRY_EXPORT PositionsWidgetView : public OpenGLBaseWidget
{
    Q_OBJECT
public:
    typedef PositionsWidget::PosesType PosesType;

    PositionsWidgetView(QWidget *parent, PosesType *poses, Eigen::AlignedBox3d *bb);
    inline ~PositionsWidgetView() {}

    void setPrismaticRevoluteFactors(const double &prismatic, const double &revolute);
    void setAutoResize(const bool autoResize);
    void resetSize();
    void setDimensions(const size_t x, const size_t y);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    double mPrismaticFactor;

    // pointers to shared data
    PosesType *mPoses;
    Eigen::AlignedBox3d *mBB;
    bool mAutoResize;

    // dimensions to plot
    size_t mX, mY;

    // viewport
    Eigen::Vector2d mViewportTranslation, mViewport;
    double mViewportScale;
};

} // namespace EigenQt

#endif // EIGENQT_POSITIONSWIDGET_H
