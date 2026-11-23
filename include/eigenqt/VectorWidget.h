// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2011-2026 Johns Hopkins University
#pragma once
#include <eigenqt/NumericTableWidget.h>
#include <eigenqt/ForwardDeclarations.h>

namespace EigenQt {
class EIGENQT_WIDGETS_EXPORT VectorView : public NumericTableWidget
{
    Q_OBJECT
public:
    explicit VectorView(QWidget *parent = nullptr);
    template <class Iterator>
    bool setValues(Iterator first, Iterator last, bool blockSignals = true)
    {
        using T = typename std::iterator_traits<Iterator>::value_type;
        std::vector<detail::Number> snapshot;
        if (!detail::collect(first, last, snapshot)) {
            return false;
        }
        const int count = static_cast<int>(snapshot.size());
        return setNumbers(1, count, detail::numberIndex<T>(), std::move(snapshot), blockSignals);
    }
    template <class Container> bool setValue(const Container &values, bool blockSignals = true)
    {
        // Allow container-specific begin/end overloads through argument-dependent lookup.
        using std::begin;
        using std::end;
        return setValues(begin(values), end(values), blockSignals);
    }

protected:
    VectorView(bool editable, DisplayMode mode, QWidget *parent);
};

class EIGENQT_WIDGETS_EXPORT VectorEditor : public VectorView
{
    Q_OBJECT
public:
    explicit VectorEditor(DisplayMode displayMode = Text, QWidget *parent = nullptr);
    template <class Iterator> bool readValues(Iterator first, Iterator last) const
    {
        std::vector<detail::Number> snapshot;
        return readNumbers(snapshot) && detail::copyChecked(snapshot, first, last);
    }
    template <class Container> bool readValues(Container &values) const
    {
        // Allow container-specific begin/end overloads through argument-dependent lookup.
        using std::begin;
        using std::end;
        return readValues(begin(values), end(values));
    }
    template <class T> std::optional<std::vector<T>> values() const
    {
        std::vector<T> result(columnCount());
        if (!readValues(result)) {
            return std::nullopt;
        }
        return result;
    }
};
} // namespace EigenQt
