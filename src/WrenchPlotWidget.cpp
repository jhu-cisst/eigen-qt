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
#include <QCheckBox>
#include <QSignalBlocker>

#include <eigenqt/WrenchPlotWidget.h>
#include <eigenqt/Plot2DWidget.h>
#include <eigenqt/detail/Constants.h>
#include <cassert>
#include <cmath>

namespace EigenQt {

WrenchPlotWidget::WrenchPlotWidget(QWidget *parent) : QWidget(parent), mScaleIndex(0)
{
    setupUi();
}

void WrenchPlotWidget::setupUi()
{

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // left side, selector and legend
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->addLayout(leftLayout);

    // combo box to select the plot item
    QComboBox *QPlotSelectItem = new QComboBox;
    QPlotSelectItem->addItem("Forces");
    QPlotSelectItem->addItem("Torques");
    QPlotSelectItem->setCurrentIndex(mScaleIndex);
    leftLayout->addWidget(QPlotSelectItem);

    // constants
    const QColor textColor = palette().color(QPalette::Text);
    const QColor baseColor = palette().color(QPalette::Base);
    const Eigen::Vector3d _colors[5] = {
        Eigen::Vector3d(1.0, 0.0, 0.0), Eigen::Vector3d(0.0, 1.0, 0.0),
        Eigen::Vector3d(0.0, 0.0, 1.0),
        Eigen::Vector3d(textColor.redF(), textColor.greenF(), textColor.blueF()),
        Eigen::Vector3d(0.5, 0.5, 0.5)};
    const std::string _signals[5] = {"Axis X", "Axis Y", "Axis Z", "Norm", "Zero"};
    const std::string _scales[2] = {"F", "T"};

    // legends
    QLabel *label;
    QPalette palette;

    for (size_t signal = 0; signal < 5; ++signal) {
        QHBoxLayout *signalLayout = new QHBoxLayout();
        signalLayout->setContentsMargins(0, 0, 0, 0);
        leftLayout->addLayout(signalLayout);
        // label
        label = new QLabel(_signals[signal].c_str());
        label->setAutoFillBackground(true);
        palette.setColor(QPalette::WindowText,
                         QColor(_colors[signal].x() * 255, _colors[signal].y() * 255,
                                _colors[signal].z() * 255));
        label->setPalette(palette);
        signalLayout->addWidget(label);
        // checkbox
        mCheckBoxes[signal] = new QCheckBox("");
        mCheckBoxes[signal]->setChecked(true);
        signalLayout->addWidget(mCheckBoxes[signal]);

        const int signalIndex = static_cast<int>(signal);
        connect(mCheckBoxes[signal], &QCheckBox::toggled, this,
                [this, signalIndex] { slotVisibleSignal(signalIndex); });
    }

    leftLayout->addStretch();

    // plot area
    QFTPlot = new Plot2DWidget();
    QFTPlot->setBackgroundColor(
        Eigen::Vector3d(baseColor.redF(), baseColor.greenF(), baseColor.blueF()));
    QFTPlot->resize(QFTPlot->sizeHint());
    QFTPlot->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    mainLayout->addWidget(QFTPlot);

    // create scales and signals
    for (size_t scale = 0; scale < 2; ++scale) {
        mScales[scale] = QFTPlot->addScale(_scales[scale]);
        for (size_t signal = 0; signal < 5; ++signal) {
            mSignals[scale][signal] = mScales[scale]->addSignal(_signals[signal]);
            mSignals[scale][signal]->setColor(_colors[signal]);
            mVisibleSignals[scale][signal] = true;
        }
    }

    this->setLayout(mainLayout);

    setWindowTitle("");
    resize(sizeHint());

    // setup Qt Connection
    connect(QPlotSelectItem, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &WrenchPlotWidget::slotScaleIndex);

    slotScaleIndex(mScaleIndex);
}

void WrenchPlotWidget::setValue(const double &time, const Eigen::Vector3d &force,
                                const Eigen::Vector3d &torque)
{
    // make sure we should update the display
    if (this->isHidden()) {
        return;
    }

    // plot both force and torque scales so switching scales preserves history
    const Eigen::Vector3d samples[2] = {force, torque};
    for (size_t scale = 0; scale < 2; ++scale) {
        for (size_t i = 0; i < 3; ++i) {
            mSignals[scale][i]->appendPoint(Eigen::Vector2d(time, samples[scale][i]));
        }
        mSignals[scale][3]->appendPoint(Eigen::Vector2d(time, samples[scale].norm()));
        mSignals[scale][4]->appendPoint(Eigen::Vector2d(time, 0.0));
    }

    QFTPlot->setDisplayYRangeScale(mScales[mScaleIndex]);
    QFTPlot->update();
}

void WrenchPlotWidget::slotScaleIndex(int newAxis)
{
    mScaleIndex = newAxis;

    for (size_t i = 0; i < 5; ++i) {
        // hide other scale
        mSignals[(mScaleIndex + 1) % 2][i]->setVisible(false);
        // show current scales and user preferences
        mSignals[mScaleIndex][i]->setVisible(mVisibleSignals[mScaleIndex][i]);
        QSignalBlocker blocker(mCheckBoxes[i]);
        mCheckBoxes[i]->setChecked(mVisibleSignals[mScaleIndex][i]);
    }
    mScales[mScaleIndex]->autoFitXY();
    QFTPlot->setDisplayYRangeScale(mScales[mScaleIndex]);
    QFTPlot->setContinuousExpandYResetSlot();
}

void WrenchPlotWidget::slotVisibleSignal(int index)
{
    const bool checked = mCheckBoxes[index]->isChecked();
    mSignals[mScaleIndex][index]->setVisible(checked);
    mVisibleSignals[mScaleIndex][index] = checked;
    QFTPlot->update();
}

} // namespace EigenQt
