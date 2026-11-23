// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 EigenQt contributors
#include <eigenqt/NumericTableWidget.h>
#include <QAbstractSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QSlider>
#include <functional>
#include <algorithm>

namespace EigenQt {
namespace {
using Number = detail::Number;
constexpr int resolution = 1000;

bool less(const Number &left, const Number &right)
{
    if (left.index() != right.index()) {
        return std::visit(
            [](auto a, auto b) {
                return static_cast<long double>(a) < static_cast<long double>(b);
            },
            left, right);
    }
    return std::visit([&](auto a) { return a < std::get<decltype(a)>(right); }, left);
}

QString format(const Number &value, char realFormat, int precision, int base)
{
    return std::visit(
        [&](auto scalar) -> QString {
            using T = decltype(scalar);
            if constexpr (std::is_same_v<T, double>) {
                return QString::number(scalar, realFormat, precision);
            } else if constexpr (std::is_same_v<T, bool>) {
                return scalar ? QStringLiteral("1") : QStringLiteral("0");
            } else if constexpr (std::is_signed_v<T>) {
                if (scalar < 0) {
                    return QStringLiteral("-") +
                           QString::number(static_cast<qulonglong>(-(scalar + 1)) + 1, base);
                }
                return QString::number(static_cast<qlonglong>(scalar), base);
            } else {
                return QString::number(static_cast<qulonglong>(scalar), base);
            }
        },
        value);
}

bool parse(const QString &text, NumericTableWidget::NumberType type, int base, Number &value)
{
    bool ok = false;
    if (type == NumericTableWidget::FloatingPoint) {
        const double scalar = text.toDouble(&ok);
        if (ok && std::isfinite(scalar)) {
            value = scalar;
            return true;
        }
    } else if (type == NumericTableWidget::SignedInteger) {
        const auto scalar = text.toLongLong(&ok, base);
        if (ok) {
            value = static_cast<std::int64_t>(scalar);
            return true;
        }
    } else if (type == NumericTableWidget::UnsignedInteger && !text.trimmed().startsWith('-')) {
        const auto scalar = text.toULongLong(&ok, base);
        if (ok) {
            value = static_cast<std::uint64_t>(scalar);
            return true;
        }
    }
    return false;
}

std::uint64_t ordered(const Number &value)
{
    if (value.index() == 1) {
        return static_cast<std::uint64_t>(std::get<std::int64_t>(value)) ^ (std::uint64_t(1) << 63);
    }
    return std::get<std::uint64_t>(value);
}

Number fromOrdered(std::uint64_t value, bool isSigned)
{
    if (!isSigned) {
        return value;
    }
    const auto middle = std::uint64_t(1) << 63;
    if (value < middle) {
        return std::numeric_limits<std::int64_t>::min() + static_cast<std::int64_t>(value);
    }
    return static_cast<std::int64_t>(value - middle);
}

// QSpinBox stores int. This small Qt spinbox preserves full signed/unsigned
// 64-bit values, including when stepping at the limits.
class IntegerSpinBox : public QAbstractSpinBox
{
public:
    explicit IntegerSpinBox(QWidget *parent) : QAbstractSpinBox(parent)
    {
        connect(this, &QAbstractSpinBox::editingFinished, this, [this] {
            Number parsed;
            if (parse(text(), mType, mBase, parsed) && !less(parsed, mMinimum) &&
                !less(mMaximum, parsed)) {
                change(parsed);
            } else {
                updateText();
            }
        });
    }
    void configure(Number value, Number minimum, Number maximum, std::uint64_t step, int base)
    {
        mValue = value;
        mMinimum = minimum;
        mMaximum = maximum;
        mStep = step;
        mBase = base;
        mType = value.index() == 1 ? NumericTableWidget::SignedInteger
                                   : NumericTableWidget::UnsignedInteger;
        updateText();
    }
    bool read(Number &value) const
    {
        return parse(text(), mType, mBase, value) && !less(value, mMinimum) &&
               !less(mMaximum, value);
    }
    std::function<void()> changed;
    void stepBy(int steps) override
    {
        Number current;
        if (read(current)) {
            mValue = current;
        }
        const auto position = ordered(mValue);
        const auto available =
            steps >= 0 ? ordered(mMaximum) - position : position - ordered(mMinimum);
        const auto count = steps >= 0 ? std::uint64_t(steps) : std::uint64_t(-std::int64_t(steps));
        const auto delta = count && mStep > available / count ? available : mStep * count;
        change(fromOrdered(steps >= 0 ? position + delta : position - delta,
                           mType == NumericTableWidget::SignedInteger));
    }
    QValidator::State validate(QString &input, int &) const override
    {
        if (input.isEmpty() || input == "-" || input == "+") {
            return QValidator::Intermediate;
        }
        Number value;
        if (!parse(input, mType, mBase, value)) {
            return QValidator::Invalid;
        }
        return !less(value, mMinimum) && !less(mMaximum, value) ? QValidator::Acceptable
                                                                : QValidator::Intermediate;
    }

protected:
    StepEnabled stepEnabled() const override
    {
        Number current;
        if (!read(current)) {
            current = mValue;
        }
        StepEnabled result = StepNone;
        if (less(current, mMaximum)) {
            result |= StepUpEnabled;
        }
        if (less(mMinimum, current)) {
            result |= StepDownEnabled;
        }
        return result;
    }

private:
    Number mValue = std::int64_t(0), mMinimum = std::int64_t(0), mMaximum = std::int64_t(100);
    std::uint64_t mStep = 1;
    int mBase = 10;
    NumericTableWidget::NumberType mType = NumericTableWidget::SignedInteger;
    void updateText() { lineEdit()->setText(format(mValue, 'f', 0, mBase)); }
    void change(Number value)
    {
        const bool different = value != mValue;
        mValue = value;
        updateText();
        if (different && changed) {
            changed();
        }
    }
};

int sliderPosition(const Number &value, const Number &minimum, const Number &maximum)
{
    if (!less(minimum, maximum)) {
        return 0;
    }
    long double ratio;
    if (value.index() == 3) {
        const auto low = std::get<double>(minimum) * 0.5L;
        ratio = (std::get<double>(value) * 0.5L - low) / (std::get<double>(maximum) * 0.5L - low);
    } else {
        ratio = static_cast<long double>(ordered(value) - ordered(minimum)) /
                (ordered(maximum) - ordered(minimum));
    }
    return std::clamp(static_cast<int>(std::llround(ratio * resolution)), 0, resolution);
}

Number sliderValue(int position, const Number &minimum, const Number &maximum)
{
    if (minimum.index() == 3) {
        const double ratio = double(position) / resolution;
        return (1.0 - ratio) * std::get<double>(minimum) + ratio * std::get<double>(maximum);
    }
    const auto distance = ordered(maximum) - ordered(minimum);
    const auto offset = distance / resolution * position +
                        (distance % resolution * position + resolution / 2) / resolution;
    return fromOrdered(ordered(minimum) + offset, minimum.index() == 1);
}
} // namespace

NumericTableWidget::NumericTableWidget(bool editable, DisplayMode mode, QWidget *parent)
    : QTableWidget(parent), mEditable(editable), mMode(mode)
{
    setContentsMargins(0, 0, 0, 0);
    verticalHeader()->hide();
    horizontalHeader()->hide();
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(this, &QTableWidget::itemChanged, this, [this] { edited(); });
}

bool NumericTableWidget::validShape(std::size_t rows, std::size_t columns)
{
    const auto limit = static_cast<std::size_t>(std::numeric_limits<int>::max());
    return rows <= limit && columns <= limit && (!columns || rows <= limit / columns);
}

std::pair<NumericTableWidget::Number, NumericTableWidget::Number>
NumericTableWidget::bounds(std::size_t index, NumberType type) const
{
    if (type == Boolean) {
        return {false, true};
    }
    const bool slider = mMode == Slider;
    Number low, high;
    if (type == FloatingPoint) {
        low = slider ? -100.0 : std::numeric_limits<double>::lowest();
        high = slider ? 100.0 : std::numeric_limits<double>::max();
    } else if (type == SignedInteger) {
        low = slider ? std::int64_t(-100) : std::numeric_limits<std::int64_t>::min();
        high = slider ? std::int64_t(100) : std::numeric_limits<std::int64_t>::max();
    } else {
        low = std::uint64_t(0);
        high = slider ? std::uint64_t(100) : std::numeric_limits<std::uint64_t>::max();
    }
    const auto convertBound = [&](const Number &bound, bool isUpper) -> Number {
        return std::visit(
            [&](auto prototype) -> Number {
                using Target = decltype(prototype);
                Target converted{};
                if (detail::convert(bound, converted)) {
                    return converted;
                }
                return std::visit(
                    [&](auto raw) -> Number {
                        using Raw = decltype(raw);
                        if constexpr (std::is_same_v<Target, double>) {
                            return static_cast<double>(raw);
                        } else if constexpr (std::is_integral_v<Target>) {
                            if constexpr (std::is_floating_point_v<Raw>) {
                                const double rounded = isUpper ? std::floor(raw) : std::ceil(raw);
                                const double maxVal =
                                    std::ldexp(1.0, std::numeric_limits<Target>::digits);
                                const double minVal = std::is_signed_v<Target> ? -maxVal : 0.0;
                                if (rounded < minVal) {
                                    if (isUpper) {
                                        throw std::invalid_argument(
                                            "Range does not fit numeric type");
                                    }
                                    return std::numeric_limits<Target>::lowest();
                                }
                                if (rounded >= maxVal) {
                                    if (!isUpper) {
                                        throw std::invalid_argument(
                                            "Range does not fit numeric type");
                                    }
                                    return std::numeric_limits<Target>::max();
                                }
                                return static_cast<Target>(rounded);
                            } else if constexpr (std::is_signed_v<Raw> &&
                                                 !std::is_signed_v<Target>) {
                                if (raw < 0) {
                                    if (isUpper) {
                                        throw std::invalid_argument(
                                            "Range does not fit numeric type");
                                    }
                                    return Target(0);
                                }
                                return static_cast<Target>(raw);
                            } else if constexpr (std::is_same_v<Raw, std::uint64_t> &&
                                                 std::is_same_v<Target, std::int64_t>) {
                                if (raw > static_cast<std::uint64_t>(
                                              std::numeric_limits<Target>::max())) {
                                    if (!isUpper) {
                                        throw std::invalid_argument(
                                            "Range does not fit numeric type");
                                    }
                                    return std::numeric_limits<Target>::max();
                                }
                                return static_cast<Target>(raw);
                            }
                        }
                        throw std::invalid_argument("Range does not fit numeric type");
                    },
                    bound);
            },
            low);
    };
    if (mRange) {
        auto newLow = convertBound(mRange->first, false);
        auto newHigh = convertBound(mRange->second, true);
        low = newLow;
        high = newHigh;
    }
    if (index < mMinimums.size()) {
        auto newLow = convertBound(mMinimums[index], false);
        auto newHigh = convertBound(mMaximums[index], true);
        low = newLow;
        high = newHigh;
    }
    if (less(high, low)) {
        throw std::invalid_argument("Minimum exceeds maximum");
    }
    if (mEditable && mMode == SpinBox && type == FloatingPoint) {
        // QDoubleSpinBox rounds its bounds to the configured decimal places.
        // Move rounded bounds inward so every displayed value remains in range.
        const auto round = [this](double value) {
            return QString::number(value, 'f', mPrecision).toDouble();
        };
        const double requestedLow = std::get<double>(low);
        const double requestedHigh = std::get<double>(high);
        double roundedLow = round(requestedLow), roundedHigh = round(requestedHigh);
        const double quantum = std::pow(10.0, -mPrecision);
        if (roundedLow < requestedLow) {
            const double nextLow =
                std::max(roundedLow + quantum,
                         std::nextafter(requestedLow, std::numeric_limits<double>::infinity()));
            roundedLow = round(nextLow);
        }
        if (roundedHigh > requestedHigh) {
            const double nextHigh =
                std::min(roundedHigh - quantum,
                         std::nextafter(requestedHigh, -std::numeric_limits<double>::infinity()));
            roundedHigh = round(nextHigh);
        }
        if (!std::isfinite(roundedLow) || !std::isfinite(roundedHigh) ||
            roundedLow < requestedLow || roundedHigh > requestedHigh || roundedLow > roundedHigh) {
            throw std::invalid_argument("Range contains no value at the selected precision");
        }
        low = roundedLow;
        high = roundedHigh;
    }
    return {low, high};
}

bool NumericTableWidget::setNumbers(int rows, int columns, std::size_t type,
                                    std::vector<Number> values, bool blockSignals)
{
    if (rows < 0 || columns < 0 || !validShape(rows, columns) ||
        values.size() != std::size_t(rows) * columns || type > 3) {
        return false;
    }
    const auto kind = static_cast<NumberType>(type);
    if (mEditable) {
        try {
            for (std::size_t i = 0; i < values.size(); ++i) {
                const auto range = bounds(i, kind);
                if (kind == FloatingPoint && !std::isfinite(std::get<double>(values[i]))) {
                    return false;
                }
                if (less(values[i], range.first)) {
                    values[i] = range.first;
                }
                if (less(range.second, values[i])) {
                    values[i] = range.second;
                }
            }
            if ((kind == SignedInteger || kind == UnsignedInteger) && mMode == SpinBox) {
                std::uint64_t step = 1;
                if (!detail::convert(mStep, step)) {
                    if (mStep.index() == 3 && std::get<double>(mStep) > 0.0) {
                        step = 1;
                    } else {
                        return false;
                    }
                }
                if (!step) {
                    return false;
                }
            } else if (kind == FloatingPoint && mMode == SpinBox) {
                double step;
                if (!detail::convert(mStep, step) || step <= 0.0) {
                    return false;
                }
            }
        } catch (const std::invalid_argument &) {
            return false;
        }
    }
    const bool changed =
        mType != kind || mValues != values || rows != rowCount() || columns != columnCount();
    QSignalBlocker blocker(this);
    mUpdating = true;
    if (mType != kind || rows != rowCount() || columns != columnCount()) {
        setRowCount(0);
        setColumnCount(columns);
        setRowCount(rows);
    }
    mType = kind;
    mValues = std::move(values);
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const auto index = std::size_t(row) * columns + column;
            const Number value = mValues[index];
            if (!mEditable || (mMode == Text && kind != Boolean)) {
                auto *cell = item(row, column);
                if (!cell) {
                    cell = new QTableWidgetItem;
                    cell->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    if (!mEditable) {
                        cell->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                    }
                    setItem(row, column, cell);
                }
                cell->setText(format(value, mFormat, mPrecision, mBase));
                continue;
            }
            const auto range = bounds(index, kind);
            if (kind == Boolean) {
                auto *box = qobject_cast<QCheckBox *>(cellWidget(row, column));
                if (!box) {
                    box = new QCheckBox;
                    setCellWidget(row, column, box);
                    connect(box, &QCheckBox::toggled, this, [this] { edited(); });
                }
                box->setChecked(std::get<bool>(value));
            } else if (mMode == Slider) {
                auto *slider = qobject_cast<QSlider *>(cellWidget(row, column));
                if (!slider) {
                    slider = new QSlider(Qt::Horizontal);
                    slider->setRange(0, resolution);
                    setCellWidget(row, column, slider);
                    connect(slider, &QSlider::valueChanged, this, [this, index, slider] {
                        if (mUpdating) {
                            return;
                        }
                        const auto limits = bounds(index, mType);
                        const auto value =
                            sliderValue(slider->value(), limits.first, limits.second);
                        if (value != mValues[index]) {
                            mValues[index] = value;
                            emit valueChanged();
                        }
                    });
                }
                slider->setValue(sliderPosition(value, range.first, range.second));
            } else if (kind == FloatingPoint) {
                auto *box = qobject_cast<QDoubleSpinBox *>(cellWidget(row, column));
                if (!box) {
                    box = new QDoubleSpinBox;
                    setCellWidget(row, column, box);
                    connect(box, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                            [this] { edited(); });
                }
                box->setDecimals(mPrecision);
                box->setRange(std::get<double>(range.first), std::get<double>(range.second));
                double step = 1.0;
                detail::convert(mStep, step);
                box->setSingleStep(step);
                box->setValue(std::get<double>(value));
            } else {
                auto *box = dynamic_cast<IntegerSpinBox *>(cellWidget(row, column));
                if (!box) {
                    box = new IntegerSpinBox(this);
                    setCellWidget(row, column, box);
                    box->changed = [this] { edited(); };
                }
                std::uint64_t step = 1;
                detail::convert(mStep, step);
                box->configure(value, range.first, range.second, step, mBase);
            }
        }
    }
    mUpdating = false;
    if (!blockSignals && changed) {
        blocker.unblock();
        emit valueChanged();
    }
    return true;
}

bool NumericTableWidget::readNumbers(std::vector<Number> &values) const
{
    if (mValues.size() != std::size_t(rowCount()) * columnCount()) {
        return false;
    }
    values = mValues;
    for (int row = 0; row < rowCount(); ++row) {
        for (int column = 0; column < columnCount(); ++column) {
            const auto index = std::size_t(row) * columnCount() + column;
            if (mType == Boolean) {
                const auto *box = qobject_cast<QCheckBox *>(cellWidget(row, column));
                values[index] = box ? box->isChecked() : std::get<bool>(mValues[index]);
            } else if (mMode == Text || !mEditable) {
                if (!item(row, column) ||
                    !parse(item(row, column)->text(), mType, mBase, values[index])) {
                    return false;
                }
            } else if (mMode == SpinBox) {
                if (mType == FloatingPoint) {
                    const auto *box = qobject_cast<QDoubleSpinBox *>(cellWidget(row, column));
                    if (!box) {
                        return false;
                    }
                    values[index] = box->value();
                } else {
                    const auto *box = dynamic_cast<IntegerSpinBox *>(cellWidget(row, column));
                    if (!box || !box->read(values[index])) {
                        return false;
                    }
                }
            }
            const auto range = bounds(index, mType);
            if (less(values[index], range.first) || less(range.second, values[index])) {
                return false;
            }
        }
    }
    return true;
}

void NumericTableWidget::edited()
{
    if (mUpdating) {
        return;
    }
    std::vector<Number> values;
    if (readNumbers(values) && values != mValues) {
        mValues = std::move(values);
        emit valueChanged();
    }
}

void NumericTableWidget::refresh()
{
    if (mValues.empty()) {
        return;
    }
    if (!setNumbers(rowCount(), columnCount(), mType, mValues, true)) {
        throw std::invalid_argument("Settings do not fit current numeric values");
    }
}

void NumericTableWidget::setPrecision(int precision)
{
    if (precision < 0 || precision > 323) {
        throw std::invalid_argument("Invalid precision");
    }
    const auto previous = mPrecision;
    mPrecision = precision;
    try {
        refresh();
    } catch (...) {
        mPrecision = previous;
        throw;
    }
}
void NumericTableWidget::setFormat(char format)
{
    if (QStringLiteral("eEfFgG").indexOf(format) < 0) {
        throw std::invalid_argument("Invalid numeric format");
    }
    mFormat = format;
    refresh();
}
void NumericTableWidget::setBase(int base)
{
    if (base < 2 || base > 36) {
        throw std::invalid_argument("Invalid integer base");
    }
    mBase = base;
    refresh();
}
void NumericTableWidget::setStepValue(Number step)
{
    const bool positive = std::visit(
        [](auto value) { return std::isfinite(static_cast<long double>(value)) && value > 0; },
        step);
    if (!positive) {
        throw std::invalid_argument("Step must be positive");
    }
    const auto previous = mStep;
    mStep = step;
    try {
        refresh();
    } catch (...) {
        mStep = previous;
        throw;
    }
}
void NumericTableWidget::setRanges(std::vector<Number> minimums, std::vector<Number> maximums,
                                   bool uniform)
{
    if (minimums.size() != maximums.size()) {
        throw std::invalid_argument("Mismatched range dimensions");
    }
    for (std::size_t i = 0; i < minimums.size(); ++i) {
        if ((minimums[i].index() == 3 && !std::isfinite(std::get<double>(minimums[i]))) ||
            (maximums[i].index() == 3 && !std::isfinite(std::get<double>(maximums[i])))) {
            throw std::invalid_argument("Ranges must be finite");
        }
        if (less(maximums[i], minimums[i])) {
            throw std::invalid_argument("Minimum exceeds maximum");
        }
    }
    const auto previous = mRange;
    auto previousMin = mMinimums, previousMax = mMaximums;
    if (uniform) {
        mRange = std::make_pair(minimums.front(), maximums.front());
        mMinimums.clear();
        mMaximums.clear();
    } else {
        mRange.reset();
        mMinimums = std::move(minimums);
        mMaximums = std::move(maximums);
    }
    try {
        refresh();
    } catch (...) {
        mRange = previous;
        mMinimums = std::move(previousMin);
        mMaximums = std::move(previousMax);
        throw;
    }
}
} // namespace EigenQt
