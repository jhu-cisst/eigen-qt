// SPDX-License-Identifier: MIT
#pragma once
#include <vector>
#include <type_traits>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace EigenQt {
template <class T> class Matrix
{
public:
    using value_type = T;
    Matrix(std::size_t rows, std::size_t cols)
        : rows_(rows), cols_(cols), values_(checkedSize(rows, cols))
    {
    }
    std::size_t rows() const { return rows_; }
    std::size_t cols() const { return cols_; }
    void resize(std::size_t rows, std::size_t cols)
    {
        values_.resize(checkedSize(rows, cols));
        rows_ = rows;
        cols_ = cols;
    }
    typename std::vector<T>::reference operator()(std::size_t r, std::size_t c)
    {
        return values_[r * cols_ + c];
    }
    typename std::vector<T>::const_reference operator()(std::size_t r, std::size_t c) const
    {
        return values_[r * cols_ + c];
    }

private:
    static std::size_t checkedSize(std::size_t rows, std::size_t cols)
    {
        if (rows > std::numeric_limits<int>::max() || cols > std::numeric_limits<int>::max() ||
            (cols && rows > std::numeric_limits<std::size_t>::max() / cols)) {
            throw std::length_error("Matrix dimensions exceed widget limits");
        }
        return rows * cols;
    }
    std::size_t rows_, cols_;
    std::vector<T> values_;
};

// Strides are measured in elements. The caller owns the referenced storage.
template <class T> class StridedMatrixView
{
public:
    using value_type = std::remove_const_t<T>;
    StridedMatrixView(T *data, std::size_t rows, std::size_t cols, std::ptrdiff_t rowStride,
                      std::ptrdiff_t colStride = 1)
        : data_(data), rows_(rows), cols_(cols), rowStride_(rowStride), colStride_(colStride)
    {
    }
    std::size_t rows() const { return rows_; }
    std::size_t cols() const { return cols_; }
    T &operator()(std::size_t row, std::size_t col) const
    {
        return data_[static_cast<std::ptrdiff_t>(row) * rowStride_ +
                     static_cast<std::ptrdiff_t>(col) * colStride_];
    }

private:
    T *data_;
    std::size_t rows_, cols_;
    std::ptrdiff_t rowStride_, colStride_;
};
} // namespace EigenQt
