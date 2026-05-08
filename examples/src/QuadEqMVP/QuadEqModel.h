// ============================================================================
// QuadEqModel.h — Реализация интерфейса IModel для решения уравнений
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// QuadEqModel — «вычислительный движок» для решения квадратных уравнений.
// Модель НЕ общается с пользователем — никакого printf.
// Все результаты предоставляются через геттеры после излучения onResultReady.
//
// ============================================================================

#ifndef QUAD_EQ_MODEL_H
#define QUAD_EQ_MODEL_H

#include "IModel.h"

class QuadEqModel : public IModel {
public:
    QuadEqModel();
    ~QuadEqModel();

    // --- IModel: управление ---
    void setCoefficients(type a, type b, type c);
    void solve();

    // --- IModel: геттеры ---
    SolveResult getSolveResult() const;
    double getRoot1() const;
    double getRoot2() const;
    double getVerification1() const;
    double getVerification2() const;

private:
    type m_a, m_b, m_c;
    SolveResult m_result;
    double m_x1, m_x2;
    double m_v1, m_v2;
};

// Фабричная функция
IModel* createQuadEqModel();

#endif // QUAD_EQ_MODEL_H
