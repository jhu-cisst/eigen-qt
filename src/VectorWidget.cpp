// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2011-2026 Johns Hopkins University
#include <eigenqt/VectorWidget.h>
#include <QHeaderView>
namespace EigenQt {
VectorView::VectorView(QWidget *parent) : VectorView(false, Text, parent) {}
VectorView::VectorView(bool editable, DisplayMode mode, QWidget *parent)
    : NumericTableWidget(editable, mode, parent)
{
    setRowCount(1);
    setFixedHeight(verticalHeader()->sizeHint().height());
}
VectorEditor::VectorEditor(DisplayMode mode, QWidget *parent) : VectorView(true, mode, parent) {}
} // namespace EigenQt
