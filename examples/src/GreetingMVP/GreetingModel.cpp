// ============================================================================
// GreetingModel.cpp — Реализация GreetingModel
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// МОДЕЛЬ И КОНСОЛЬ:
//
//   В модели НИ ОДНОГО printf. Модель формирует приветствие
//   и излучает сигнал onGreetingReady — презентер решает,
//   как показать результат через вид.
//
// ПОТОК ДАННЫХ В МОДЕЛИ:
//
//   1. Презентер вызывает setName(name)
//      -> модель сохраняет имя
//
//   2. Презентер вызывает setSurname(surname)
//      -> модель сохраняет фамилию (может быть "")
//
//   3. Презентер вызывает buildGreeting()
//      -> модель формирует приветственную строку
//      -> модель излучает onGreetingReady
//
//   4. Презентер забирает результат через getGreeting()
//
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <stdio.h>     /* snprintf */
#include <string.h>    /* strncpy, strlen */

#include "GreetingModel.h"

using namespace signals;

// ============================================================================
// GreetingModel — реализация
// ============================================================================

GreetingModel::GreetingModel()
{
    m_data.clear();
    m_greeting[0] = '\0';
}

GreetingModel::~GreetingModel() {}

// --- Установить имя ---

void GreetingModel::setName(const char* name)
{
    if (!name) {
        m_data.name[0] = '\0';
        return;
    }
    strncpy(m_data.name, name, MAX_NAME_LEN - 1);
    m_data.name[MAX_NAME_LEN - 1] = '\0';
}

// --- Установить фамилию ---

void GreetingModel::setSurname(const char* surname)
{
    if (!surname) {
        m_data.surname[0] = '\0';
        return;
    }
    strncpy(m_data.surname, surname, MAX_NAME_LEN - 1);
    m_data.surname[MAX_NAME_LEN - 1] = '\0';
}

// --- Сформировать приветствие ---

void GreetingModel::buildGreeting()
{
    // Проверяем, что имя задано
    if (m_data.name[0] == '\0') {
        onError.emit_("Имя не задано — не могу сформировать приветствие.");
        return;
    }

    // Формируем приветствие в зависимости от наличия фамилии
    if (m_data.surname[0] != '\0') {
        // Имя и фамилия заданы
        snprintf(m_greeting, sizeof(m_greeting),
                 "Здравствуйте, %s %s! Рад вас видеть!",
                 m_data.name, m_data.surname);
    } else {
        // Только имя
        snprintf(m_greeting, sizeof(m_greeting),
                 "Здравствуйте, %s! Рад вас видеть!",
                 m_data.name);
    }

    // Уведомляем: приветствие готово
    onGreetingReady.emit_();
}

// --- Геттер ---

const char* GreetingModel::getGreeting() const
{
    return m_greeting;
}


// ============================================================================
// Фабричная функция
// ============================================================================

IModel* createGreetingModel() { return new GreetingModel(); }
