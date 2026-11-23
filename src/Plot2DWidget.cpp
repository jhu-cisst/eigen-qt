// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet
  Created on: 2010-05-05

  (C) Copyright 2010-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#include <eigenqt/Plot2DWidget.h>

#include <QMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QColor>
#include <eigenqt/detail/Constants.h>
#include <cassert>
#include <cmath>

#include <eigenqt/OpenGLBaseWidget.h>

namespace EigenQt {

Plot2DWidget::Plot2DWidget(QWidget *parent) : OpenGLBaseWidget(parent), Plot2DOpenGL()
{
    this->setFocusPolicy(Qt::StrongFocus);
    this->setToolTip(QString("'space' to pause/restart, 'r' to reset\nright click for menu"));
    // Context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &Plot2DWidget::showContextMenu);
}

void Plot2DWidget::setDisplayYRangeScale(Plot2D::Scale *scale)
{
    if (scale) {
        DisplayYRangeScaleName = scale->name();
    } else {
        DisplayYRangeScaleName.reset();
    }
    update();
}

Plot2D::Scale *Plot2DWidget::displayYRangeScale() const
{
    if (!DisplayYRangeScaleName) {
        return nullptr;
    }
    const auto found = Scales.find(*DisplayYRangeScaleName);
    return found == Scales.end() ? nullptr : found->second.get();
}

void Plot2DWidget::initializeGL()
{
    Plot2DOpenGL::renderInitialize();
}

void Plot2DWidget::resizeGL(int width, int height)
{
    Plot2DOpenGL::renderResize(width, height);
    const qreal ratio = devicePixelRatioF();
    glViewport(0, 0, qRound(Viewport.x() * ratio), qRound(Viewport.y() * ratio));
}

void Plot2DWidget::paintGL()
{
    Plot2DOpenGL::render();
    if (const auto *scale = displayYRangeScale()) {
        const int font_size = 8;
        QPainter painter(this);
        const QColor txtColor = palette().color(QPalette::Text);
        painter.setPen(txtColor);
        painter.setFont(QFont("Helvetica", font_size));
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        Eigen::Vector2d range = scale->viewingRangeY();
        painter.drawText(1, 1 + font_size, QString().asprintf("%0.2f", range.y()));
        painter.drawText(1, this->Viewport.y() - font_size, QString().asprintf("%0.2f", range.x()));
        painter.end();
    }
}

void Plot2DWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Space:
        this->setFrozen(!this->isFrozen());
        this->update();
        break;
    case Qt::Key_R:
        this->setContinuousExpandY(false);
        this->setContinuousExpandY(true);
        this->update();
        break;
    default:
        break;
    }
}

void Plot2DWidget::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = this->mapToGlobal(pos);
    QMenu menu;

    QAction *freeze = new QAction(tr("Freeze"), &menu);
    freeze->setCheckable(true);
    freeze->setChecked(this->isFrozen());
    menu.addAction(freeze);
    connect(freeze, &QAction::toggled, this, &Plot2DWidget::freezeSlot);

    menu.addSeparator();

    QAction *fitXnow = new QAction("Fit X now", &menu);
    QAction *fitXalways = new QAction("Fit X always", &menu);
    fitXalways->setCheckable(true);
    fitXalways->setChecked(this->continuousFitX());
    menu.addAction(fitXnow);
    menu.addAction(fitXalways);
    connect(fitXnow, &QAction::triggered, this, &Plot2DWidget::fitXSlot);
    connect(fitXalways, &QAction::toggled, this, &Plot2DWidget::setContinuousFitXSlot);

    menu.addSeparator();

    QAction *fitYnow = new QAction("Fit Y now", &menu);
    QAction *fitYalways = new QAction("Fit Y always", &menu);
    QAction *expandYalways = new QAction("Expand Y always", &menu);
    QAction *expandYreset = new QAction("Expand Y reset", &menu);
    fitYalways->setCheckable(true);
    fitYalways->setChecked(this->continuousFitY());
    expandYalways->setCheckable(true);
    expandYalways->setChecked(this->continuousExpandY());
    menu.addAction(fitYnow);
    menu.addAction(fitYalways);
    menu.addAction(expandYalways);
    menu.addAction(expandYreset);
    connect(fitYnow, &QAction::triggered, this, &Plot2DWidget::fitYSlot);
    connect(fitYalways, &QAction::toggled, this, &Plot2DWidget::setContinuousFitYSlot);
    connect(expandYalways, &QAction::toggled, this, &Plot2DWidget::setContinuousExpandYSlot);
    connect(expandYreset, &QAction::triggered, this, &Plot2DWidget::setContinuousExpandYResetSlot);

    menu.exec(globalPos);
}

void Plot2DWidget::freezeSlot(bool checked)
{
    this->setFrozen(checked);
    this->update();
}

void Plot2DWidget::fitXSlot()
{
    this->setContinuousFitX(false);
    this->autoFitX();
    this->update();
}

void Plot2DWidget::fitYSlot()
{
    this->setContinuousFitY(false);
    this->autoFitY();
    this->update();
}

void Plot2DWidget::setContinuousFitXSlot(bool checked)
{
    this->setContinuousFitX(checked);
    this->update();
}

void Plot2DWidget::setContinuousFitYSlot(bool checked)
{
    this->setContinuousFitY(checked);
    this->update();
}

void Plot2DWidget::setContinuousExpandYSlot(bool checked)
{
    this->setContinuousExpandY(checked);
    this->update();
}

void Plot2DWidget::setContinuousExpandYResetSlot()
{
    this->setContinuousExpandY(false);
    this->setContinuousExpandY(true);
    this->update();
}

} // namespace EigenQt
