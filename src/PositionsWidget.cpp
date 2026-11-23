// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet
  Created on: 2020-09-04

  (C) Copyright 2020-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#include <eigenqt/PositionsWidget.h>
#include <QTableWidget>
#include <QHeaderView>
#include <QKeyEvent>
#include <QPainter>
#include <eigenqt/detail/Constants.h>
#include <algorithm>
#include <cassert>
#include <cmath>

#include <eigenqt/OpenGLBaseWidget.h>

namespace EigenQt {

namespace {
constexpr int maxTableRows = 10;
}

PositionsWidget::PositionsWidget(QWidget *parent) : QWidget(parent)
{
    this->setFocusPolicy(Qt::StrongFocus);
    this->setToolTip(tr("Positions projected onto XY, XZ and YZ planes"));
    setContentsMargins(0, 0, 0, 0);
    mBB.setEmpty();

    // set grid layout to display multiple views
    mLayout = new QGridLayout();
    setLayout(mLayout);

    // table of values
    mTable = new QTableWidget();
    mTable->setContentsMargins(0, 0, 0, 0);
    mTable->verticalHeader()->hide();
    mTable->horizontalHeader()->hide();
    mTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    mTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    QSizePolicy policy;
    policy.setHorizontalPolicy(QSizePolicy::Minimum);
    policy.setVerticalPolicy(QSizePolicy::Minimum);
    mTable->setSizePolicy(policy);
    mTable->setRowCount(maxTableRows);
    mTable->setColumnCount(3);
    for (int row = 0; row < maxTableRows; ++row) {
        for (int column = 0; column < 3; ++column) {
            QTableWidgetItem *tableItem = mTable->item(row, column);
            if (tableItem == nullptr) {
                tableItem = new QTableWidgetItem();
                tableItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                tableItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                tableItem->setText("");
                mTable->setItem(row, column, tableItem);
            }
        }
    }
    mTable->resize(mTable->sizeHint());
    mPrismaticFactor = 1000.0;
    mTableRow = 0;
    mLayout->addWidget(mTable, 0, 0);

    // create 3 views
    PositionsWidgetView *view;
    view = new PositionsWidgetView(this, &mPoses, &mBB);
    view->setPrismaticRevoluteFactors(mPrismaticFactor, 1.0);
    view->setDimensions(0, 1); // plot XY
    mLayout->addWidget(view, 0, 1);
    mViews.push_back(view);

    view = new PositionsWidgetView(this, &mPoses, &mBB);
    view->setPrismaticRevoluteFactors(mPrismaticFactor, 1.0);
    view->setDimensions(0, 2); // plot XZ
    mLayout->addWidget(view, 1, 0);
    mViews.push_back(view);

    view = new PositionsWidgetView(this, &mPoses, &mBB);
    view->setPrismaticRevoluteFactors(mPrismaticFactor, 1.0);
    view->setDimensions(1, 2); // plot YZ
    mLayout->addWidget(view, 1, 1);
    mViews.push_back(view);
}

void PositionsWidget::setPrismaticRevoluteFactors(const double &prismatic, const double &revolute)
{
    mPrismaticFactor = prismatic;
    const int filledRows = std::min(mTableRow, maxTableRows);
    for (int row = 0; row < filledRows; ++row) {
        for (int column = 0; column < 3; ++column) {
            mTable->item(row, column)
                ->setText(QString::number(
                    mPoses[static_cast<size_t>(row)](column) * mPrismaticFactor, 'f', 2));
        }
    }
    for (auto *view : mViews) {
        view->setPrismaticRevoluteFactors(prismatic, revolute);
        view->update();
    }
}

void PositionsWidget::setValue(const Eigen::Vector3d &value)
{
    // update table if there's room
    if (mTableRow < maxTableRows) {
        for (int column = 0; column < 3; ++column) {
            mTable->item(mTableRow, column)
                ->setText(QString::number(value(column) * mPrismaticFactor, 'f', 2));
        }
        mTableRow++;
    }

    // update bounding box
    mBB.extend(value);

    // save all values for display
    mPoses.push_back(value);

    for (auto *view : mViews) {
        view->update();
    }
}

void PositionsWidget::clear()
{
    mTableRow = 0;
    for (int row = 0; row < maxTableRows; ++row) {
        for (int column = 0; column < 3; ++column) {
            mTable->item(row, column)->setText("");
        }
    }
    mPoses.clear();
    mBB.setEmpty();
    for (auto *view : mViews) {
        view->update();
    }
}

void PositionsWidget::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

PositionsWidgetView::PositionsWidgetView(QWidget *parent, PosesType *poses, Eigen::AlignedBox3d *bb)
    : OpenGLBaseWidget(parent), mPrismaticFactor(1000.0), mPoses(poses), mBB(bb), mAutoResize(true),
      mX(0), mY(1), mViewportTranslation(Eigen::Vector2d::Zero()),
      mViewport(Eigen::Vector2d::Ones()), mViewportScale(1.0)
{
    this->setFocusPolicy(Qt::StrongFocus);
    this->setToolTip(QString("'a' to turn on/off autoresize\n'r' to reset autoresize if already in "
                             "autoresize mode\n'1' to plot XY\n'2' to plot XZ\n'3' to plot YZ"));
    setContentsMargins(0, 0, 0, 0);
    setAutoResize(true);
}

void PositionsWidgetView::setPrismaticRevoluteFactors(const double &prismatic,
                                                      const double & /* revolute */)
{
    mPrismaticFactor = prismatic;
}

void PositionsWidgetView::setAutoResize(const bool autoResize)
{
    mAutoResize = autoResize;
    if (mAutoResize) {
        resetSize();
    }
}

void PositionsWidgetView::resetSize()
{
    mBB->setEmpty();
    for (const auto &pose : *mPoses) {
        mBB->extend(pose);
    }
}

void PositionsWidgetView::setDimensions(const size_t x, const size_t y)
{
    if ((x != y) && (x < 3) && (y < 3)) {
        mX = x;
        mY = y;
    } else {
        mX = 0;
        mY = 1;
    }
}

void PositionsWidgetView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_1:
        mX = 0;
        mY = 1;
        update();
        break;
    case Qt::Key_2:
        mX = 0;
        mY = 2;
        update();
        break;
    case Qt::Key_3:
        mX = 1;
        mY = 2;
        update();
        break;
    case Qt::Key_A:
        setAutoResize(!mAutoResize);
        update();
        break;
    case Qt::Key_R:
        resetSize();
        update();
        break;
    default:
        break;
    }
}

void PositionsWidgetView::initializeGL()
{
    glMatrixMode(GL_MODELVIEW); // set the model view matrix
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST); // disable depth test
    glDisable(GL_LIGHTING);   // disable lighting
    glShadeModel(GL_SMOOTH);  // smooth render
    const QColor bgColor = palette().color(QPalette::Base);
    glClearColor(bgColor.redF(), bgColor.greenF(), bgColor.blueF(), 1.0);
}

void PositionsWidgetView::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, mViewport.x(), 0.0, mViewport.y(), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // update translation and scale based on viewport and bounding box
    if (mAutoResize && !mBB->isEmpty()) {
        // scale to fit viewport without changing ratio
        Eigen::Vector3d sizeBB = mBB->diagonal();
        if ((sizeBB[mX] > 0.0) && (sizeBB[mY] > 0.0)) {
            const double ratioX = mViewport.x() / sizeBB[mX];
            const double ratioY = mViewport.y() / sizeBB[mY];
            mViewportScale = (ratioY > ratioX ? ratioX : ratioY) * 0.98; // add some padding
        } else {
            mViewportScale = 1.0;
        }
        // take center of BB for display and scale to viewport
        const Eigen::Vector3d centerBB = mBB->center() * mViewportScale;
        const Eigen::Vector2d centerVP = mViewport / 2.0;
        mViewportTranslation = centerVP - Eigen::Vector2d(centerBB[mX], centerBB[mY]);
    }

    glTranslated(mViewportTranslation.x(), mViewportTranslation.y(), 0.0);
    glScaled(mViewportScale, mViewportScale, 1.0);
    const QColor txtColor = palette().color(QPalette::Text);
    glColor3f(txtColor.redF(), txtColor.greenF(), txtColor.blueF());
    glPointSize(3.0);
    glBegin(GL_POINTS);
    for (const auto &pose : *mPoses) {
        glVertex2d(pose.coeff(mX), pose.coeff(mY));
    }
    glEnd();

    if (mBB->isEmpty()) {
        return;
    }

    // bounding box
    glColor3d(0.7, 0.7, 0.7);
    glBegin(GL_LINE_STRIP);
    {
        glVertex2d(mBB->min()[mX], mBB->min()[mY]);
        glVertex2d(mBB->max()[mX], mBB->min()[mY]);
        glVertex2d(mBB->max()[mX], mBB->max()[mY]);
        glVertex2d(mBB->min()[mX], mBB->max()[mY]);
        glVertex2d(mBB->min()[mX], mBB->min()[mY]);
    }
    glEnd();
    // zero values
    if (mBB->min()[mY] * mBB->max()[mY] < 0.0) {
        glBegin(GL_LINE_STRIP);
        {
            glVertex2d(mBB->min()[mX], 0.0);
            glVertex2d(mBB->max()[mX], 0.0);
        }
        glEnd();
    }
    if (mBB->min()[mX] * mBB->max()[mX] < 0.0) {
        glBegin(GL_LINE_STRIP);
        {
            glVertex2d(0.0, mBB->min()[mY]);
            glVertex2d(0.0, mBB->max()[mY]);
        }
        glEnd();
    }

    const char legend[3] = {'x', 'y', 'z'};
    const int font_size = 8;
    QPainter painter(this);
    painter.setPen(txtColor);
    painter.setFont(QFont("Helvetica", font_size));
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.drawText(1, 1 + font_size,
                     QString::asprintf("%c: %0.2f", legend[mY], mBB->max()[mY] * mPrismaticFactor));
    painter.drawText(1, mViewport.y() - font_size,
                     QString::asprintf("%0.2f/%0.2f", mBB->min()[mY] * mPrismaticFactor,
                                       mBB->min()[mX] * mPrismaticFactor));
    painter.drawText(mViewport.x() - font_size * 10, mViewport.y() - font_size,
                     QString::asprintf("%c: %0.2f", legend[mX], mBB->max()[mX] * mPrismaticFactor));
    painter.end();
}

void PositionsWidgetView::resizeGL(int width, int height)
{
    const int w = std::max(width, 1);
    const int h = std::max(height, 1);
    mViewport = Eigen::Vector2d(w, h);
    const qreal ratio = devicePixelRatioF();
    glViewport(0, 0, qRound(w * ratio), qRound(h * ratio));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w, 0.0, h, -1.0, 1.0);
}

} // namespace EigenQt
