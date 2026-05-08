// ============================================================================
// MatrixModel.cpp — Реализация MatrixModel
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// МОДЕЛЬ И КОНСОЛЬ:
//
//   В модели НИ ОДНОГО printf. Если происходит ошибка (например,
//   нехватка памяти), модель излучает сигнал onError — и пусть
//   презентер решает, как показать это пользователю через вид.
//   Модель не знает ни о какой консоли.
//
// ПОТОК ДАННЫХ В МОДЕЛИ:
//
//   1. Презентер вызывает setDimensions(rA, cA, rB, cB)
//      -> модель выделяет память под матрицы
//
//   2. Презентер вызывает loadMatrixA(data, rows, cols)
//      -> модель копирует данные во внутренний буфер
//
//   3. Презентер вызывает loadMatrixB(data, rows, cols)
//      -> модель копирует данные во внутренний буфер
//
//   4. Презентер вызывает multiply()
//      -> модель выполняет умножение с проверкой переполнения
//      -> модель излучает onResultReady
//      -> если было переполнение, модель излучает onOverflowDetected
//
//   5. Презентер забирает результат через геттеры
//
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// ============================================================================
// ПОДКЛЮЧЕНИЕ БИБЛИОТЕК
// ============================================================================
#include <stdlib.h>
#include <string.h>

#include "MatrixModel.h"
#include "MatrixTypes.h"

using namespace signals;

// ============================================================================
// MatrixModel — реализация
// ============================================================================

MatrixModel::MatrixModel()
    : m_rA(0), m_cA(0), m_rB(0), m_cB(0),
      m_matA(NULL), m_matB(NULL), m_result(NULL),
      m_resultRows(0), m_resultCols(0), m_overflow(false) {}

MatrixModel::~MatrixModel()
{
    freeData();
}

// --- Установить размерности и выделить память ---

void MatrixModel::setDimensions(int rA, int cA, int rB, int cB)
{
    freeData();
    m_rA = rA; m_cA = cA;
    m_rB = rB; m_cB = cB;
    m_matA = (type*)malloc((size_t)rA * cA * sizeof(type));
    m_matB = (type*)malloc((size_t)rB * cB * sizeof(type));
    m_resultRows = rA;
    m_resultCols = cB;
    m_result = (typeres*)malloc((size_t)rA * cB * sizeof(typeres));
    if (!m_matA || !m_matB || !m_result) {
        // Не печатаем на консоль! Уведомляем через сигнал.
        onError.emit_("Ошибка: не хватило памяти!");
        freeData();
        return;
    }
}

// --- Загрузить данные матрицы ---

void MatrixModel::loadMatrixA(const type* data, int rows, int cols)
{
    (void)rows; (void)cols;
    if (m_matA && data)
        memcpy(m_matA, data, (size_t)m_rA * m_cA * sizeof(type));
}

void MatrixModel::loadMatrixB(const type* data, int rows, int cols)
{
    (void)rows; (void)cols;
    if (m_matB && data)
        memcpy(m_matB, data, (size_t)m_rB * m_cB * sizeof(type));
}

// --- Умножение матриц с проверкой переполнения ---

void MatrixModel::multiply()
{
    int i, j, k;
    m_overflow = false;
    computeTypeLimits();

    for (i = 0; i < m_rA; i++) {
        for (j = 0; j < m_cB; j++) {
            typeres product;
            int product_is_neg;

            m_result[i * m_cB + j] = 0;

            for (k = 0; k < m_cA; k++) {
                product = (typeres)m_matA[i * m_cA + k] * m_matB[k * m_cB + j];

                product_is_neg = ((product < g_TYPERES_MAX) &&
                                 ((product & ~g_TYPERES_MAX) != 0));

                if (product != 0 && !product_is_neg) {
                    if (m_result[i * m_cB + j] > g_TYPERES_MAX - product)
                        m_overflow = true;
                } else if (product_is_neg) {
                    if (m_result[i * m_cB + j] < g_TYPERES_MIN - product)
                        m_overflow = true;
                }

                m_result[i * m_cB + j] += product;
            }
        }
    }

    // Уведомляем: результат готов
    onResultReady.emit_();
    // Если было переполнение — дополнительное уведомление
    if (m_overflow) onOverflowDetected.emit_();
}

// --- Геттеры ---

const typeres* MatrixModel::getResultData() const { return m_result; }
int MatrixModel::getResultRows() const { return m_resultRows; }
int MatrixModel::getResultCols() const { return m_resultCols; }
bool MatrixModel::hasOverflow() const { return m_overflow; }

// --- Освобождение памяти ---

void MatrixModel::freeData()
{
    if (m_matA)   { free(m_matA);   m_matA   = NULL; }
    if (m_matB)   { free(m_matB);   m_matB   = NULL; }
    if (m_result) { free(m_result);  m_result = NULL; }
}


// ============================================================================
// Фабричная функция
// ============================================================================

IModel* createMatrixModel() { return new MatrixModel(); }
