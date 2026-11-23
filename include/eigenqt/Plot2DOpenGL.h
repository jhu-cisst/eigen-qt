// SPDX-License-Identifier: MIT

/*

  Author(s):  Anton Deguet
  Created on: 2010-05-05

  (C) Copyright 2010-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#ifndef EIGENQT_PLOT2DOPENGL_H
#define EIGENQT_PLOT2DOPENGL_H

#include <eigenqt/Plot2D.h>

#include <eigenqt/plot_export.h>

namespace EigenQt {

/*! 2D plot class using OpenGL for rendering */
class EIGENQT_PLOT_EXPORT Plot2DOpenGL : public Plot2D
{
public:
    Plot2DOpenGL();
    virtual ~Plot2DOpenGL(){};

protected:
    /*! Define rendering function for OpenGL */
    //@{
    virtual void renderInitialize();
    virtual void renderResize(double width, double height);
    virtual void render(); // todo, move render to base class, not virtual

    // called by render()
    virtual void render(const VerticalLine *line);
    virtual void render(const Scale *scale);
    virtual void render(const Signal *signal);
    //@}
};

} // namespace EigenQt

#endif // EIGENQT_PLOT2DOPENGL_H
