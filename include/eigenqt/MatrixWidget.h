// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2012-2026 Johns Hopkins University
#pragma once
#include <eigenqt/NumericTableWidget.h>
#include <eigenqt/Matrix.h>
#include <eigenqt/ForwardDeclarations.h>

namespace EigenQt {
class EIGENQT_WIDGETS_EXPORT MatrixView : public NumericTableWidget
{
    Q_OBJECT
public:
    explicit MatrixView(QWidget *parent = nullptr);
    template <class Accessor>
    bool setValues(std::size_t rows, std::size_t columns, Accessor coefficient,
                   bool blockSignals = true)
    {
        using T = std::decay_t<decltype(coefficient(0, 0))>;
        if (!validShape(rows, columns)) {
            return false;
        }
        std::vector<detail::Number> snapshot;
        snapshot.reserve(rows * columns);
        for (std::size_t row = 0; row < rows; ++row) {
            for (std::size_t column = 0; column < columns; ++column) {
                snapshot.push_back(detail::number(static_cast<T>(coefficient(row, column))));
            }
        }
        return setNumbers(static_cast<int>(rows), static_cast<int>(columns),
                          detail::numberIndex<T>(), std::move(snapshot), blockSignals);
    }
    template <class MatrixLike> bool setValue(const MatrixLike &values, bool blockSignals = true)
    {
        using T = typename detail::MatrixScalar<MatrixLike>::type;
        return setValues(
            values.rows(), values.cols(),
            [&](std::size_t row, std::size_t column) {
                return static_cast<T>(values(row, column));
            },
            blockSignals);
    }

protected:
    MatrixView(bool editable, DisplayMode mode, QWidget *parent);
};

class EIGENQT_WIDGETS_EXPORT MatrixEditor : public MatrixView
{
    Q_OBJECT
public:
    explicit MatrixEditor(DisplayMode displayMode = SpinBox, QWidget *parent = nullptr);
    explicit MatrixEditor(QWidget *parent);
    template <class T, class Setter>
    bool readValues(std::size_t rows, std::size_t columns, Setter coefficient) const
    {
        if (rows != static_cast<std::size_t>(rowCount()) ||
            columns != static_cast<std::size_t>(columnCount())) {
            return false;
        }
        std::vector<detail::Number> snapshot;
        if (!readNumbers(snapshot)) {
            return false;
        }
        std::vector<T> converted(rows * columns);
        if (!detail::copyChecked(snapshot, converted.begin(), converted.end())) {
            return false;
        }
        for (std::size_t row = 0; row < rows; ++row) {
            for (std::size_t column = 0; column < columns; ++column) {
                coefficient(row, column, static_cast<T>(converted[row * columns + column]));
            }
        }
        return true;
    }
    template <class MatrixLike> bool readValues(MatrixLike &values) const
    {
        using T = typename detail::MatrixScalar<MatrixLike>::type;
        return readValues<T>(
            values.rows(), values.cols(),
            [&](std::size_t row, std::size_t column, T value) { values(row, column) = value; });
    }
};
} // namespace EigenQt
