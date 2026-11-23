// SPDX-License-Identifier: MIT

/*
  Author(s):  Anton Deguet
  Created on: 2010-05-05

  (C) Copyright 2010-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#ifndef EIGENQT_PLOT2DOPENGLQTWIDGET_H
#define EIGENQT_PLOT2DOPENGLQTWIDGET_H

#include <eigenqt/ForwardDeclarations.h>
#include <eigenqt/Plot2DOpenGL.h>

#include <eigenqt/plot_export.h>

#include <eigenqt/OpenGLBaseWidget.h>

#include <utility>
#include <optional>

namespace EigenQt {

class EIGENQT_PLOT_EXPORT Plot2DWidget : public OpenGLBaseWidget, public Plot2DOpenGL
{
    Q_OBJECT

public:
    Plot2DWidget(QWidget *parent = 0);
    ~Plot2DWidget(){};

    void setDisplayYRangeScale(Plot2D::Scale *scale);
    Plot2D::Scale *displayYRangeScale() const;

protected:
    /*! Methods required for Qt */
    //@{
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();
    virtual void keyPressEvent(QKeyEvent *event);
    //@}

public slots:
    void showContextMenu(const QPoint &pos);
    void freezeSlot(bool);
    void fitXSlot();
    void fitYSlot();
    void setContinuousFitXSlot(bool);
    void setContinuousFitYSlot(bool);
    void setContinuousExpandYSlot(bool);
    void setContinuousExpandYResetSlot();

protected:
    // Resolve the selection on each paint instead of retaining a borrowed pointer.
    std::optional<std::string> DisplayYRangeScaleName;
};

} // namespace EigenQt

#endif // EIGENQT_PLOT2DOPENGLQTWIDGET_H
