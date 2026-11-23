// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet, Dorothy Hu
  Created on: 2017-01-20

  (C) Copyright 2017-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#ifndef EIGENQT_FORCETORQUE3DQTWIDGET_H
#define EIGENQT_FORCETORQUE3DQTWIDGET_H

#include <eigenqt/ForwardDeclarations.h>
#include <eigenqt/Vector3DWidget.h>
#include <Eigen/Dense>
#include <QWidget>

class QLabel;

#include <eigenqt/geometry_export.h>

#include <utility>

namespace EigenQt {

class EIGENQT_GEOMETRY_EXPORT Wrench3DWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Wrench3DWidget(QWidget *parent = nullptr);
    ~Wrench3DWidget() {}

private:
    void setupUi();
    Vector3DWidget *QVector;

    // force vs. torque
    int PlotIndex;
    Eigen::Vector3d mForce = Eigen::Vector3d::Zero();
    Eigen::Vector3d mTorque = Eigen::Vector3d::Zero();
    bool mHasValue = false;

public:
    void setValue(const Eigen::Vector3d &force, const Eigen::Vector3d &torque);

private slots:
    void slotPlotIndex(int newAxis);
};

} // namespace EigenQt

#endif // EIGENQT_FORCETORQUE3DQTWIDGET_H
