// ============================================================================
// MatrixModel.h — Реализация интерфейса IModel для умножения матриц
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// ЧТО ДЕЛАЕТ МОДЕЛЬ?
//
//   MatrixModel — это «вычислительный движок». Она:
//     • Хранит матрицы A, B и результат их умножения
//     • Выделяет/освобождает память под матрицы
//     • Умножает матрицы с проверкой переполнения
//     • Сообщает о результате через сигналы
//
//   Модель НЕ общается с пользователем — никакого printf.
//   Ошибки (например, нехватка памяти) сообщаются через сигнал onError,
//   а не через прямой вывод на консоль.
//
// ============================================================================

#ifndef MATRIX_MODEL_H
#define MATRIX_MODEL_H

#include "IModel.h"

class MatrixModel : public IModel {
public:
    MatrixModel();
    ~MatrixModel();

    // --- IModel: управление ---

    void setDimensions(int rA, int cA, int rB, int cB);
    void loadMatrixA(const type* data, int rows, int cols);
    void loadMatrixB(const type* data, int rows, int cols);
    void multiply();

    // --- IModel: геттеры ---

    const typeres* getResultData() const;
    int getResultRows() const;
    int getResultCols() const;
    bool hasOverflow() const;

private:
    int m_rA, m_cA, m_rB, m_cB;
    type*    m_matA;
    type*    m_matB;
    typeres* m_result;
    int m_resultRows, m_resultCols;
    bool m_overflow;

    void freeData();
};

#endif // MATRIX_MODEL_H
