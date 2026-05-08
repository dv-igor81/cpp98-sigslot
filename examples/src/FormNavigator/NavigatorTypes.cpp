// ============================================================================
// NavigatorTypes.cpp Ч –еализаци€ SharedData
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "NavigatorTypes.h"
#include <string.h>

SharedData::SharedData() : count(1)
{
    text[0] = '\0';
}

void SharedData::clear()
{
    text[0] = '\0';
    count = 1;
}
