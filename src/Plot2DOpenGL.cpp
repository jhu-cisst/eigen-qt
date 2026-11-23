// SPDX-License-Identifier: MIT

/*

  Author(s):  Anton Deguet
  Created on: 2010-05-05

  (C) Copyright 2010-2026 Johns Hopkins University (JHU), All Rights Reserved.

*/

#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <eigenqt/Plot2DOpenGL.h>
#include <eigenqt/detail/Constants.h>
#include <cassert>
#include <cmath>

namespace EigenQt {

Plot2DOpenGL::Plot2DOpenGL() : Plot2D() {}

void Plot2DOpenGL::renderInitialize()
{
    glMatrixMode(GL_MODELVIEW); // set the model view matrix
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST); // disable depth test
    glDisable(GL_LIGHTING);   // disable lighting
    glShadeModel(GL_SMOOTH);  // smooth render
    glClearColor(static_cast<GLclampf>(this->BackgroundColor[0]),
                 static_cast<GLclampf>(this->BackgroundColor[1]),
                 static_cast<GLclampf>(this->BackgroundColor[2]), static_cast<GLclampf>(1.0));
}

void Plot2DOpenGL::renderResize(double width, double height)
{
    const double w = width > 1.0 ? width : 1.0;
    const double h = height > 1.0 ? height : 1.0;
    Viewport = Eigen::Vector2d(w, h);

    for (auto &scale : Scales) {
        scale.second->Viewport.x() = w;
        scale.second->Viewport.y() = h;
    }

    glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w, 0.0, h, -1.0, 1.0);
}

void Plot2DOpenGL::render()
{
    // see if translation and scale need to be updated
    this->continuousUpdate();
    // clear
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, Viewport.x(), 0.0, Viewport.y(), -1.0, 1.0);
    // plot all scales
    for (auto &scale : Scales) {
        this->render(scale.second.get());
    }
}

void Plot2DOpenGL::render(const Plot2D::VerticalLine *line)
{
    if (!line->Visible) {
        return;
    }
    glColor3d(line->Color.x(), line->Color.y(), line->Color.z());
    glLineWidth(static_cast<GLfloat>(line->LineWidth));
    glBegin(GL_LINE_STRIP);
    glVertex2d(line->X, this->Viewport.y());
    glVertex2d(line->X, 0);
    glEnd();
}

void Plot2DOpenGL::render(const Plot2D::Signal *signal)
{
    if (!signal->Visible || signal->Empty) {
        return;
    }

    glColor3d(signal->Color.x(), signal->Color.y(), signal->Color.z());
    glLineWidth(static_cast<GLfloat>(signal->LineWidth));
    const double *data = signal->data.front().data();
    size_t size = signal->data.size();

    if (signal->IndexFirst > signal->IndexLast) {
        // circular buffer is full/split in two
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_DOUBLE, 0, data);
        // draw first part
        glDrawArrays(GL_LINE_STRIP, static_cast<GLint>(signal->IndexFirst),
                     static_cast<GLsizei>(size - signal->IndexFirst));
        // draw second part
        glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(signal->IndexLast + 1));
        glDisableClientState(GL_VERTEX_ARRAY);
        // draw between end of buffer and beginning
        glBegin(GL_LINE_STRIP);
        glVertex2d(signal->data[size - 1][0], signal->data[size - 1][1]);
        glVertex2d(signal->data[0][0], signal->data[0][1]);
        glEnd();
    } else {
        // simpler case, all points contiguous
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_DOUBLE, 0, data);
        glDrawArrays(GL_LINE_STRIP, static_cast<GLint>(signal->IndexFirst),
                     static_cast<GLsizei>(signal->IndexLast + 1 - signal->IndexFirst));
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}

void Plot2DOpenGL::render(const Plot2D::Scale *scale)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(scale->Translation.x(), scale->Translation.y(), 0.0);
    glScaled(scale->ScaleValue.x(), scale->ScaleValue.y(), 1.0);

    // signals
    {
        const Plot2D::Scale::SignalsType::const_iterator end = scale->Signals.end();
        Plot2D::Scale::SignalsType::const_iterator iter = scale->Signals.begin();
        for (; iter != end; ++iter) {
            render(iter->second.get());
        }
    }

    // lines
    {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslated(scale->Translation.x(), 0.0, 0.0);
        glScaled(scale->ScaleValue.x(), 1.0, 1.0);
        const Plot2D::Scale::VerticalLinesType::const_iterator end = scale->VerticalLines.end();
        Plot2D::Scale::VerticalLinesType::const_iterator iter = scale->VerticalLines.begin();
        for (; iter != end; ++iter) {
            render(iter->second.get());
        }
    }
}

} // namespace EigenQt
