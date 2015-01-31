#include <iostream>
#include <vector>
#include <cstdint>

namespace Kelly
{
    template<typename T>
    class Matrix
    {
        size_t _rowCount;
        size_t _columnCount;
        size_t _rowIndexMultiplier;
        size_t _columnIndexMultiplier;
        std::vector<T> _values;

        Matrix(
            size_t rowCount,
            size_t columnCount,
            size_t rowIndexMultiplier,
            size_t columnIndexMultiplier)
            : _rowCount(rowCount)
            , _columnCount(columnCount)
            , _rowIndexMultiplier(rowIndexMultiplier)
            , _columnIndexMultiplier(columnIndexMultiplier)
        {
            _values.resize(rowCount * columnCount);
        }

        Matrix(const Matrix&) = default;

    public:
        static Matrix RowMajor(size_t rowCount, size_t columnCount)
        {
            return Matrix(rowCount, columnCount, columnCount, 1);
        }

        static Matrix ColumnMajor(size_t rowCount, size_t columnCount)
        {
            return Matrix(rowCount, columnCount, 1, rowCount);
        }

        Matrix()
            : _rowCount(0)
            , _columnCount(0)
            , _rowIndexMultiplier(0)
            , _columnIndexMultiplier(0)
        {
        }

        Matrix(Matrix&& other)
            : _rowCount(other._rowCount)
            , _columnCount(other._columnCount)
            , _rowIndexMultiplier(other._rowIndexMultiplier)
            , _columnIndexMultiplier(other._columnIndexMultiplier)
            , _values(std::move(other._values))
        {
            other._rowCount = 0;
            other._columnCount = 0;
            other._rowIndexMultiplier = 0;
            other._columnIndexMultiplier = 0;
        }

        ~Matrix() = default;

        Matrix& operator=(const Matrix&) = delete;

        Matrix& operator=(Matrix&& other)
        {
            if (this != &other)
            {
                this->~Matrix();
                new (this) Matrix(std::move(other));
            }

            return *this;
        }

        size_t IndexOf(size_t row, size_t column) const
        {
            return row * _rowIndexMultiplier + column * _columnIndexMultiplier;
        }

        T& operator()(size_t row, size_t column)
        {
            return _values[IndexOf(row, column)];
        }

        const T& operator()(size_t row, size_t column) const
        {
            return _values[IndexOf(row, column)];
        }

        T& operator[](size_t index)
        {
            return _values[index];
        }

        const T& operator[](size_t index) const
        {
            return _values[index];
        }

        size_t RowCount() const { return _rowCount; }
        size_t ColumnCount() const { return _columnCount; }
        size_t CellCount() const { return _values.size(); }
        bool IsRowMajor() const { return _columnIndexMultiplier == 1; }
        bool IsColumnMajor() const { return _rowIndexMultiplier == 1; }

        Matrix Copy() const { return Matrix(*this); }
    };

    template<typename T>
    Matrix<T> operator*(const Matrix<T>& a, const Matrix<T>& b)
    {
        Matrix<T> result;

        if (a.CellCount() > 0 &&
            b.CellCount() > 0 &&
            a.ColumnCount() == b.RowCount())
        {
            result = Matrix<T>::RowMajor(a.RowCount(), b.ColumnCount());

            for (size_t i = 0; i < result.RowCount(); ++i)
            {
                for (size_t j = 0; j < result.ColumnCount(); ++j)
                {
                    T total = 0;

                    for (size_t k = 0; k < a.ColumnCount(); ++k)
                    {
                        total += a(i, k) * b(k, j);
                    }

                    result(i, j) = total;
                }
            }
        }

        return result;
    }

    template<typename T>
    bool operator==(const Matrix<T>& a, const Matrix<T>& b)
    {
        if (a.RowCount() == b.RowCount() && a.ColumnCount() == b.ColumnCount())
        {
            for (size_t i = 0; i < a.RowCount(); ++i)
            {
                for (size_t j = 0; j < a.ColumnCount(); ++j)
                {
                    if (a(i, j) != b(i, j)) return false;
                }
            }

            return true;
        }

        return false;
    }

    template<typename T>
    bool operator!=(const Matrix<T>& a, const Matrix<T>& b)
    {
        return !(a == b);
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& stream, const Matrix<T>& matrix)
    {
        if (matrix.CellCount() > 0)
        {
            for (size_t i = 0; i < matrix.RowCount(); ++i)
            {
                stream << matrix(i, 0);

                for (size_t j = 1; j < matrix.ColumnCount(); ++j)
                    stream << ", " << matrix(i, j);

                stream << '\n';
            }
        }
        else
        {
            stream << "(empty)\n";
        }

        return stream;
    }
}
