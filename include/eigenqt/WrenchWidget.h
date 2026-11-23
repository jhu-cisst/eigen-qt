// SPDX-License-Identifier: MIT

/*
  Author(s): Anton Deguet, Dorothy Hu
  Created on: 2017-01-20

  (C) Copyright 2017-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#ifndef EIGENQT_FORCETORQUEQTWIDGET_H
#define EIGENQT_FORCETORQUEQTWIDGET_H

#include <Eigen/Dense>
#include <eigenqt/ForwardDeclarations.h>
#include <eigenqt/WrenchPlotWidget.h>
#include <eigenqt/Wrench3DWidget.h>

#include <QWidget>

class QVBoxLayout;

#include <eigenqt/geometry_export.h>

#include <utility>

namespace EigenQt {

/*! Qt Widget to display force/torque using Eigen. */
class EIGENQT_GEOMETRY_EXPORT WrenchWidget : public QWidget
{
    Q_OBJECT

public:
    /*! Possible display modes.  See setDisplayMode method.  Please
      note that Undefined should never be used. */
    typedef enum
    {
        Undefined,
        Text,   // text display
        Plot2D, // 2D plot
        Plot3D  // 3D display
    } DisplayMode;
    Q_ENUM(DisplayMode)
    Q_PROPERTY(
        DisplayMode displayMode READ displayMode WRITE setDisplayMode NOTIFY displayModeChanged)
    DisplayMode displayMode() const { return mDisplayMode; }

    /*! Constructor.  Default display mode is 2D plot.  See
      also setDisplayMode. */
    explicit WrenchWidget(const DisplayMode displayMode = Plot3D, QWidget *parent = nullptr);

    inline ~WrenchWidget(){};

    /*! Set the force and torque. For 2D plot, time is needed as
      well. */
    void setValue(const Eigen::Vector3d &force, const Eigen::Vector3d &torque,
                  const double &time = 0.0)
    {
        mForce = force;
        mTorque = torque;
        mTime = time;
        this->updateCurrentWidget();
    }

    /*! Set the display mode, i.e. the widget used to represent the
      wrench.  Options are Text (just numbers), Plot2D
      (x, y, z plot over time), Plot3D (3D lines representing
      force/torque direction and amplitude).  Please note that the
      display mode Undefined will be silently ignored. */
    void setDisplayMode(const DisplayMode displayMode);
    Eigen::Vector3d force() const { return mForce; }
    Eigen::Vector3d torque() const { return mTorque; }
    double timestamp() const { return mTime; }

signals:
    void displayModeChanged(EigenQt::WrenchWidget::DisplayMode mode);

protected slots:
    /*! Contextual menu showed when the user right clicks on the widget */
    void showContextMenu(const QPoint &position);

protected:
    /*! Current display mode, somewhat redundant with the
      CurrentWidget pointer.  The current widget should always be
      set by changing the mDisplayMode. */
    DisplayMode mDisplayMode;

    /*! Update the content of the current widget.  This method is
      called when the user provides a new wrench with setValue or
      when the widget used to display the wrench is changed using
      setDisplayMode. */
    void updateCurrentWidget();

    /*! Internal representation for the wrench/time to display. */
    Eigen::Vector3d mForce, mTorque;
    double mTime;

    // Text widgets
    VectorView *ForceWidget;
    VectorView *TorqueWidget;
    VectorView *NormWidget;
    QWidget *ForceTorqueWidget;

    // 2D/3D widgets
    WrenchPlotWidget *Plot2DWidget;
    Wrench3DWidget *Plot3DWidget;

    // current widget
    QWidget *CurrentWidget;
    QVBoxLayout *Layout;
};

} // namespace EigenQt

#endif // EIGENQT_FORCETORQUEQTWIDGET_H
