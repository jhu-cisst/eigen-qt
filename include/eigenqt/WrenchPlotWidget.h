// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet, Dorothy Hu
  Created on: 2017-01-20

  (C) Copyright 2017-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#ifndef EIGENQT_FORCETORQUE2DQTWIDGET_H
#define EIGENQT_FORCETORQUE2DQTWIDGET_H

#include <eigenqt/ForwardDeclarations.h>
#include <eigenqt/Plot2D.h>
#include <Eigen/Dense>

#include <QWidget>

class QLabel;
class QCheckBox;

#include <eigenqt/geometry_export.h>

#include <utility>

namespace EigenQt {

class EIGENQT_GEOMETRY_EXPORT WrenchPlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WrenchPlotWidget(QWidget *parent = nullptr);
    ~WrenchPlotWidget() {}

    void setValue(const double &time, const Eigen::Vector3d &force, const Eigen::Vector3d &torque);

private:
    void setupUi();

    Plot2DWidget *QFTPlot;
    // signals 0, 1, 2 are for x, y, z.  3 is for norm and 4 for zero
    Plot2D::Signal *mSignals[2][5];
    bool mVisibleSignals[2][5];

    // scale 0 is for forces, 1 is for torques
    Plot2D::Scale *mScales[2];

    int mScaleIndex; // 0 for forces, 1 for torques

    QCheckBox *mCheckBoxes[5];

private slots:
    void slotScaleIndex(int newAxis);
    void slotVisibleSignal(int);
};

} // namespace EigenQt

#endif // EIGENQT_FORCETORQUE2DQTWIDGET_H
