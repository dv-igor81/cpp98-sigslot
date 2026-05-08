// ============================================================================
// QuadEqModel.cpp — Реализация QuadEqModel
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// МОДЕЛЬ И КОНСОЛЬ:
//
//   В модели НИ ОДНОГО printf. Все результаты — через геттеры и сигналы.
//   Модель не знает ни о какой консоли.
//
// АЛГОРИТМ РЕШЕНИЯ:
//
//   1. Если a = 0:
//      • b != 0 -> линейное уравнение: x = -c/b
//      • b = 0, c = 0 -> бесконечно много решений (0 = 0)
//      • b = 0, c != 0 -> нет решений
//
//   2. Если a != 0 (полное квадратное уравнение):
//      • D = b^2 - 4ac
//      • D > 0 -> два корня: x = (-b ± sqrtD) / (2a)
//      • D = 0 -> один корень: x = -b / (2a)
//      • D < 0 -> нет действительных корней
//
//   ПРОВЕРКА (верификация):
//     Для каждого корня x вычисляется a·x^2 + b·x + c.
//     Результат должен быть ~= 0 (с учётом погрешности float).
//
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "QuadEqModel.h"

#include <math.h>

using namespace signals;

// ============================================================================
// QuadEqModel — реализация
// ============================================================================

QuadEqModel::QuadEqModel()
    : m_a(0), m_b(0), m_c(0),
      m_result(SOLVE_NO_SOLUTION),
      m_x1(0.0), m_x2(0.0),
      m_v1(0.0), m_v2(0.0) {}

QuadEqModel::~QuadEqModel() {}

void QuadEqModel::setCoefficients(type a, type b, type c)
{
    m_a = a;
    m_b = b;
    m_c = c;
}

void QuadEqModel::solve()
{
    // ---------------------------------------------------------------
    // СЛУЧАЙ А: Уравнение вырождается в линейное (a = 0)
    // ---------------------------------------------------------------
    if (m_a == 0)
    {
        if (m_b != 0)
        {
            // Линейное уравнение: b·x + c = 0  =>  x = -c / b
            m_x1 = -(double)m_c / m_b;
            m_v1 = (double)m_b * m_x1 + m_c;
            m_result = SOLVE_LINEAR;
        }
        else
        {
            // a = 0, b = 0
            if (m_c == 0)
                m_result = SOLVE_INFINITE;  // 0 = 0
            else
                m_result = SOLVE_NO_SOLUTION; // c != 0
        }

        onResultReady.emit_();
        return;
    }

    // ---------------------------------------------------------------
    // СЛУЧАЙ Б: Полное квадратное уравнение (a != 0)
    // ---------------------------------------------------------------
    double discriminant = (double)m_b * m_b - 4.0 * (double)m_a * m_c;

    if (discriminant > 0)
    {
        // Два различных корня
        m_x1 = (-(double)m_b + sqrt(discriminant)) / (2.0 * m_a);
        m_x2 = (-(double)m_b - sqrt(discriminant)) / (2.0 * m_a);
        m_v1 = (double)m_a * m_x1 * m_x1 + (double)m_b * m_x1 + m_c;
        m_v2 = (double)m_a * m_x2 * m_x2 + (double)m_b * m_x2 + m_c;
        m_result = SOLVE_TWO_ROOTS;
    }
    else if (discriminant == 0)
    {
        // Один корень (два совпадающих)
        m_x1 = -(double)m_b / (2.0 * m_a);
        m_v1 = (double)m_a * m_x1 * m_x1 + (double)m_b * m_x1 + m_c;
        m_result = SOLVE_ONE_ROOT;
    }
    else
    {
        // Нет действительных корней (D < 0)
        m_result = SOLVE_NO_REAL_ROOTS;
    }

    onResultReady.emit_();
}

// --- Геттеры ---

SolveResult QuadEqModel::getSolveResult() const { return m_result; }
double QuadEqModel::getRoot1() const { return m_x1; }
double QuadEqModel::getRoot2() const { return m_x2; }
double QuadEqModel::getVerification1() const { return m_v1; }
double QuadEqModel::getVerification2() const { return m_v2; }


// ============================================================================
// Фабричная функция
// ============================================================================

IModel* createQuadEqModel() { return new QuadEqModel(); }
