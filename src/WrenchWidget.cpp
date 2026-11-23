// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet, Dorothy Hu
  Created on: 2017-01-20

  (C) Copyright 2017-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <eigenqt/WrenchWidget.h>

#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QVBoxLayout>

// all these widgets should be replaced to use static vectors/matrices
#include <eigenqt/VectorWidget.h>
#include <eigenqt/detail/Constants.h>
#include <cassert>
#include <cmath>

namespace EigenQt {

WrenchWidget::WrenchWidget(const DisplayMode displayMode, QWidget *parent)
    : QWidget(parent), mDisplayMode(Undefined), CurrentWidget(0)
{
    qRegisterMetaType<EigenQt::WrenchWidget::DisplayMode>("EigenQt::WrenchWidget::DisplayMode");
    mTime = 0.0;
    mForce.fill(0.0);
    mTorque.fill(0.0);

    QVBoxLayout *forceTorqueLayout = new QVBoxLayout;
    forceTorqueLayout->setContentsMargins(0, 0, 0, 0);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    forceTorqueLayout->addLayout(gridLayout);
    forceTorqueLayout->addStretch();

    gridLayout->addWidget(new QLabel("Force"), 0, 0);
    ForceWidget = new VectorView(this);
    ForceWidget->setPrecision(4);
    gridLayout->addWidget(ForceWidget, 0, 1);

    gridLayout->addWidget(new QLabel("Torque"), 1, 0);
    TorqueWidget = new VectorView(this);
    TorqueWidget->setPrecision(4);
    gridLayout->addWidget(TorqueWidget, 1, 1);

    gridLayout->addWidget(new QLabel("Norm"), 2, 0);
    NormWidget = new VectorView(this);
    NormWidget->setPrecision(4);
    gridLayout->addWidget(NormWidget, 2, 1);

    ForceTorqueWidget = new QWidget(this);
    ForceTorqueWidget->setLayout(forceTorqueLayout);

    Plot2DWidget = new WrenchPlotWidget(this);
    Plot3DWidget = new Wrench3DWidget(this);

    ForceTorqueWidget->hide();
    Plot2DWidget->hide();
    Plot3DWidget->hide();

    Layout = new QVBoxLayout;
    Layout->setSpacing(0);
    Layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(Layout);
    this->setWindowTitle("WrenchWidget");

    // Set display mode
    setDisplayMode(displayMode);

    // Context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &WrenchWidget::showContextMenu);
}

void WrenchWidget::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = this->mapToGlobal(pos);
    QMenu menu;
    QAction *text = new QAction("Text", &menu);
    QAction *plot2D = new QAction("2D", &menu);
    QAction *plot3D = new QAction("3D", &menu);

    if (mDisplayMode != Text) {
        menu.addAction(text);
    }
    if (mDisplayMode != Plot2D) {
        menu.addAction(plot2D);
    }
    if (mDisplayMode != Plot3D) {
        menu.addAction(plot3D);
    }

    QAction *selectedItem = menu.exec(globalPos);

    if (selectedItem) {
        if (selectedItem == text) {
            setDisplayMode(Text);
        } else if (selectedItem == plot2D) {
            setDisplayMode(Plot2D);
        } else if (selectedItem == plot3D) {
            setDisplayMode(Plot3D);
        }
    }
}

void WrenchWidget::updateCurrentWidget()
{
    // compute the value based on the internal values
    switch (mDisplayMode) {
    case Text:
        ForceWidget->setValue(mForce);
        TorqueWidget->setValue(mTorque);
        NormWidget->setValue(Eigen::Vector<double, 1>(mForce.norm()));
        break;
    case Plot2D:
        Plot2DWidget->setValue(mTime, mForce, mTorque);
        break;
    case Plot3D:
        Plot3DWidget->setValue(mForce, mTorque);
        break;
    default:
        break;
    }
}

void WrenchWidget::setDisplayMode(const DisplayMode displayMode)
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
    case Text:
        CurrentWidget = ForceTorqueWidget;
        break;
    case Plot2D:
        CurrentWidget = Plot2DWidget;
        break;
    case Plot3D:
        CurrentWidget = Plot3DWidget;
        break;
    default:
        assert(false);
        break;
    }
    Layout->addWidget(CurrentWidget);
    CurrentWidget->show();
    updateCurrentWidget();
    repaint();
    emit displayModeChanged(displayMode);
}

} // namespace EigenQt
