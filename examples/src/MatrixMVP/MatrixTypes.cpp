// MatrixTypes.cpp Ч –еализаци€ общих утилит дл€ проекта MatrixMVP
// Borland C++ Builder 6.0 / C++98

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "MatrixTypes.h"

// √лобальные пределы типов
typeres g_TYPERES_MAX_SIGNED;
type    g_TYPE_MAX_SIGNED;
typeres g_TYPERES_MAX;
typeres g_TYPERES_MIN;

void computeTypeLimits()
{
    static int computed = 0;
    if (computed) return;
    computed = 1;

    int is_signed = ((typeres)-1 < 0);
    int bits_typeres = sizeof(typeres) * 8 - 1;
    int bits_type    = sizeof(type) * 8 - 1;
    int ii;

    g_TYPERES_MAX_SIGNED = 1;
    for (ii = 1; ii < bits_typeres; ii++) {
        g_TYPERES_MAX_SIGNED <<= 1;
        g_TYPERES_MAX_SIGNED += 1;
    }

    g_TYPE_MAX_SIGNED = 1;
    for (ii = 1; ii < bits_type; ii++) {
        g_TYPE_MAX_SIGNED <<= 1;
        g_TYPE_MAX_SIGNED += 1;
    }

    if (is_signed) {
        g_TYPERES_MAX = g_TYPERES_MAX_SIGNED;
    } else {
        g_TYPERES_MAX = ~(typeres)0;
    }
    g_TYPERES_MIN = ~g_TYPERES_MAX;
}
