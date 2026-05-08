// ============================================================================
// GreetingModel.h Ч –еализаци€ интерфейса IModel дл€ формировани€ приветстви€
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// GreetingModel хранит UserData и формирует приветственную строку.
// ‘амили€ необ€зательна Ч если она пуста, приветствие используетс€
// только по имени.
//
// ============================================================================

#ifndef GREETING_MODEL_H
#define GREETING_MODEL_H

#include "IModel.h"

const int MAX_GREETING_LEN = 512;

class GreetingModel : public IModel {
public:
    GreetingModel();
    ~GreetingModel();

    // --- IModel: управление ---

    void setName(const char* name);
    void setSurname(const char* surname);
    void buildGreeting();

    // --- IModel: геттеры ---

    const char* getGreeting() const;

private:
    UserData m_data;
    char m_greeting[MAX_GREETING_LEN];
};

#endif // GREETING_MODEL_H
