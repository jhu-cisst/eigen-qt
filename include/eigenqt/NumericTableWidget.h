// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 EigenQt contributors
#pragma once

#include <eigenqt/detail/Number.h>
#include <eigenqt/widgets_export.h>
#include <QTableWidget>
#include <optional>
#include <stdexcept>

namespace EigenQt {
// Common Qt implementation. Container/element types live in the adapters,
// rather than defining separate QObject classes for every numeric type.
class EIGENQT_WIDGETS_EXPORT NumericTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    enum DisplayMode
    {
        Text,
        SpinBox,
        Slider
    };
    Q_ENUM(DisplayMode)
    enum NumberType
    {
        Boolean,
        SignedInteger,
        UnsignedInteger,
        FloatingPoint
    };
    Q_ENUM(NumberType)
    Q_PROPERTY(DisplayMode displayMode READ displayMode CONSTANT)
    Q_PROPERTY(NumberType numberType READ numberType)
    Q_PROPERTY(int precision READ precision WRITE setPrecision)
    Q_PROPERTY(int base READ base WRITE setBase)

    DisplayMode displayMode() const { return mMode; }
    NumberType numberType() const { return mType; }
    int precision() const { return mPrecision; }
    int base() const { return mBase; }
    void setPrecision(int precision);
    void setFormat(char format);
    void setBase(int base);

    template <class Min, class Max,
              std::enable_if_t<std::is_arithmetic_v<Min> && std::is_arithmetic_v<Max>, int> = 0>
    void setRange(Min minimum, Max maximum)
    {
        setRanges({detail::number(minimum)}, {detail::number(maximum)}, true);
    }
    template <class Min, class Max, std::enable_if_t<!std::is_arithmetic_v<Min>, int> = 0>
    void setRange(const Min &minimums, const Max &maximums)
    {
        // Allow container-specific begin/end overloads through argument-dependent lookup.
        using std::begin;
        using std::end;
        std::vector<detail::Number> low, high;
        if (!detail::collect(begin(minimums), end(minimums), low) ||
            !detail::collect(begin(maximums), end(maximums), high)) {
            throw std::length_error("Too many numeric ranges");
        }
        setRanges(std::move(low), std::move(high), false);
    }
    template <class T> void setStep(T step) { setStepValue(detail::number(step)); }

signals:
    void valueChanged();

protected:
    explicit NumericTableWidget(bool editable, DisplayMode mode, QWidget *parent);
    bool setNumbers(int rows, int columns, std::size_t type, std::vector<detail::Number> values,
                    bool blockSignals);
    bool readNumbers(std::vector<detail::Number> &values) const;
    static bool validShape(std::size_t rows, std::size_t columns);

private:
    using Number = detail::Number;
    bool mEditable;
    DisplayMode mMode;
    NumberType mType = FloatingPoint;
    int mPrecision = 2;
    char mFormat = 'f';
    int mBase = 10;
    Number mStep = 1;
    std::vector<Number> mValues;
    std::optional<std::pair<Number, Number>> mRange;
    std::vector<Number> mMinimums, mMaximums;
    bool mUpdating = false;
    std::pair<Number, Number> bounds(std::size_t index, NumberType type) const;
    void setRanges(std::vector<Number> minimums, std::vector<Number> maximums, bool uniform);
    void setStepValue(Number step);
    void refresh();
    void edited();
};
} // namespace EigenQt
