// ============================================================================
// GreetingTypes.cpp Ч –еализаци€ UserData
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "GreetingTypes.h"
#include <string.h>

UserData::UserData()
{
    name[0] = '\0';
    surname[0] = '\0';
}

void UserData::clear()
{
    name[0] = '\0';
    surname[0] = '\0';
}
