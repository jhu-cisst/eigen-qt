// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2012-2026 Johns Hopkins University
#include <eigenqt/MatrixWidget.h>
namespace EigenQt {
MatrixView::MatrixView(QWidget *parent) : MatrixView(false, Text, parent) {}
MatrixView::MatrixView(bool editable, DisplayMode mode, QWidget *parent)
    : NumericTableWidget(editable, mode, parent)
{
}
MatrixEditor::MatrixEditor(DisplayMode mode, QWidget *parent) : MatrixView(true, mode, parent) {}
MatrixEditor::MatrixEditor(QWidget *parent) : MatrixEditor(SpinBox, parent) {}
} // namespace EigenQt
