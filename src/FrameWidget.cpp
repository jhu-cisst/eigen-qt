// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet
  Created on: 2013-04-20

  (C) Copyright 2013-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#include <eigenqt/FrameWidget.h>
#include <QVBoxLayout>
#include <eigenqt/detail/Constants.h>
#include <cassert>
#include <cmath>

namespace EigenQt {

FrameWidget::FrameWidget(const DisplayMode displayMode, QWidget *parent) : QWidget(parent)
{
    mRotationWidget = new RotationWidget(displayMode, this);
    connect(mRotationWidget, &RotationWidget::displayModeChanged, this,
            &FrameWidget::displayModeChanged);
    TranslationWidget = new VectorView(this);
    TranslationWidget->setPrecision(3);
    Layout = new QVBoxLayout;
    this->setLayout(Layout);
    this->setWindowTitle(tr("Frame"));
    Layout->setSpacing(1);
    Layout->setContentsMargins(0, 0, 0, 0);
    Layout->addWidget(mRotationWidget);
    Layout->addWidget(TranslationWidget);
    // by default, display in millimeters
    mPrismaticFactor = 1.0 / 0.001;
    TranslationWidget->setValue(mFrame.translation() * mPrismaticFactor);
}

} // namespace EigenQt
