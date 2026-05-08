// MatrixTypes.h Ч ќбщие типы, константы и утилиты дл€ проекта MatrixMVP
// Borland C++ Builder 6.0 / C++98

#ifndef MATRIX_TYPES_H
#define MATRIX_TYPES_H

// ============================================================================
// Ќј—“–ќ… » “»ѕќ¬
// ============================================================================
#ifndef ELEM_TYPE
#define ELEM_TYPE short
#endif

#ifndef RESULT_TYPE
#define RESULT_TYPE int
#endif

typedef ELEM_TYPE  type;
typedef RESULT_TYPE typeres;

#define MAX_DIM 100

// ѕроверки на этапе компил€ции
#define COMPILE_TIME_ASSERT(cond, msg) \
    typedef char compile_time_assert_##msg[(cond) ? 1 : -1]

COMPILE_TIME_ASSERT(sizeof(typeres) == 2 * sizeof(type),
                    typeres_must_be_twice_type_size);
COMPILE_TIME_ASSERT(((type)-1 < 0) == ((typeres)-1 < 0),
                    types_must_have_same_signedness);

// ============================================================================
// √лобальные пределы типов (вычисл€ютс€ один раз при первом обращении)
// ============================================================================

extern typeres g_TYPERES_MAX_SIGNED;
extern type    g_TYPE_MAX_SIGNED;
extern typeres g_TYPERES_MAX;
extern typeres g_TYPERES_MIN;

// ¬ычисл€ет пределы при первом вызове, последующие вызовы Ч no-op.
void computeTypeLimits();

#endif // MATRIX_TYPES_H
