// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet, Dorothy Hu
  Created on: 2017-01-20

  (C) Copyright 2017-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QComboBox>

#include <eigenqt/Wrench3DWidget.h>
#include <eigenqt/detail/Constants.h>
#include <cassert>
#include <cmath>

namespace EigenQt {

Wrench3DWidget::Wrench3DWidget(QWidget *parent) : QWidget(parent), PlotIndex(0)
{
    setupUi();
}

void Wrench3DWidget::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // left side, upper/lower limit, selector and legend
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(leftLayout);

    // combo box to select the plot item
    QComboBox *QPlotSelectItem = new QComboBox;
    QPlotSelectItem->addItem("Forces");
    QPlotSelectItem->addItem("Torques");
    QPlotSelectItem->setCurrentIndex(PlotIndex);
    leftLayout->addWidget(QPlotSelectItem);

    // legend
    QLabel *label;
    QPalette palette;
    label = new QLabel("Axis X");
    label->setAutoFillBackground(true);
    palette.setColor(QPalette::WindowText, Qt::red);
    label->setPalette(palette);
    leftLayout->addWidget(label);
    label = new QLabel("Axis Y");
    label->setAutoFillBackground(true);
    palette.setColor(QPalette::WindowText, Qt::green);
    label->setPalette(palette);
    leftLayout->addWidget(label);
    label = new QLabel("Axis Z");
    label->setAutoFillBackground(true);
    palette.setColor(QPalette::WindowText, Qt::blue);
    label->setPalette(palette);
    leftLayout->addWidget(label);
    label = new QLabel("Vector");
    label->setAutoFillBackground(true);
    palette.setColor(QPalette::WindowText, this->palette().color(QPalette::WindowText));
    label->setPalette(palette);
    leftLayout->addWidget(label);

    leftLayout->addStretch();

    // plot area
    QVector = new Vector3DWidget();
    QVector->resize(QVector->sizeHint());
    QVector->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    mainLayout->addWidget(QVector);

    this->setLayout(mainLayout);

    setWindowTitle("");
    resize(sizeHint());

    // setup Qt Connection
    connect(QPlotSelectItem, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &Wrench3DWidget::slotPlotIndex);

    slotPlotIndex(PlotIndex);
}

void Wrench3DWidget::setValue(const Eigen::Vector3d &force, const Eigen::Vector3d &torque)
{
    mForce = force;
    mTorque = torque;
    mHasValue = true;

    // make sure we should update the display
    if (this->isHidden()) {
        return;
    }

    // plot
    if (PlotIndex == 0) {
        QVector->setValue(mForce);
    } else if (PlotIndex == 1) {
        QVector->setValue(mTorque);
    }
}

void Wrench3DWidget::slotPlotIndex(int newAxis)
{
    PlotIndex = newAxis;
    QVector->setAutoResize(true);
    if (mHasValue) {
        QVector->setValue(PlotIndex == 0 ? mForce : mTorque);
    } else {
        QVector->update();
    }
}

} // namespace EigenQt
