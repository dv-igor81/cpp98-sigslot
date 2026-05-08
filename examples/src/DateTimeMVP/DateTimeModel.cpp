// ============================================================================
// DateTimeModel.cpp — Реализация DateTimeModel
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// МОДЕЛЬ И КОНСОЛЬ:
//
//   В модели НИ ОДНОГО printf. Модель выполняет конвертацию
//   и излучает сигнал onResultReady — презентер решает,
//   как показать результат через вид.
//
// ПОТОК ДАННЫХ В МОДЕЛИ:
//
//   1. Презентер вызывает setTimezoneOffset(hours)
//      -> модель сохраняет часовой пояс
//
//   2. Презентер вызывает setConversionMode(mode)
//      -> модель сохраняет направление
//
//   3. Презентер вызывает setEpochValue() или setDateTime()
//      -> модель сохраняет входные данные
//
//   4. Презентер вызывает convert()
//      -> модель выполняет конвертацию
//      -> модель формирует текстовый результат
//      -> модель излучает onResultReady
//
//   5. Презентер забирает результат через getResult()
//
// ЧАСОВОЙ ПОЯС:
//
//   Оригинальные функции epoch2datetime и datetime2epoch имели
//   жёстко заданное смещение GMT+03:00 (10800 секунд).
//   В данной реализации смещение передаётся как параметр
//   tz_offset_sec, что позволяет задавать любой часовой пояс.
//
//   Беззнаковая арифметика с tz_offset_sec корректно работает
//   благодаря двоичному дополнению (two's complement):
//     • Отрицательное смещение (e.g. -5h = -18000) при приведении
//       к unsigned int даёт правильный результат вычитания
//     • Результат может быть некорректен для epoch < |смещение|
//       (т.е. для дат ранее 1970 года при больших отрицательных поясах)
//
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <stdio.h>     /* snprintf */
#include <string.h>    /* strlen */

#include "DateTimeModel.h"

using namespace signals;

// ============================================================================
// DateTimeModel — реализация
// ============================================================================

DateTimeModel::DateTimeModel()
    : m_tzOffsetHours(0), m_mode(0), m_epochInput(0),
      m_resultEpoch(0)
{
    // Инициализация структур нулями
    m_dateTimeInput.TimeSec    = 0;
    m_dateTimeInput.TimeMin    = 0;
    m_dateTimeInput.TimeHor    = 0;
    m_dateTimeInput.TimeWeek   = 0;
    m_dateTimeInput.TimeDMonth = 1;
    m_dateTimeInput.TimeMonth  = 1;
    m_dateTimeInput.TimeYear   = 0;

    m_resultDateTime = m_dateTimeInput;
    m_resultText[0] = '\0';
}

DateTimeModel::~DateTimeModel() {}

// --- Установить часовой пояс ---

void DateTimeModel::setTimezoneOffset(int offset_hours)
{
    m_tzOffsetHours = offset_hours;
}

// --- Установить направление преобразования ---

void DateTimeModel::setConversionMode(int mode)
{
    m_mode = mode;
}

// --- Установить входное Unix Time ---

void DateTimeModel::setEpochValue(unsigned int epoch)
{
    m_epochInput = epoch;
}

// --- Установить входную дату/время ---

void DateTimeModel::setDateTime(const TDateTime& dt)
{
    m_dateTimeInput = dt;
}

// --- Выполнить преобразование ---

void DateTimeModel::convert()
{
    long tz_offset_sec = (long)m_tzOffsetHours * SECS_PER_HOUR;

    if (m_mode == MODE_EPOCH_TO_DATETIME)
    {
        // Конвертация Unix Time -> Дата/Время
        epoch2datetime(m_epochInput, &m_resultDateTime, tz_offset_sec);

        // Формируем результат: "07 Мая 2026 (Четверг)\nВремя: 14:30:00"
        const char* wd = getWeekdayName(m_resultDateTime.TimeWeek);
        const char* mn = getMonthName(m_resultDateTime.TimeMonth);
        int fullYear = (int)m_resultDateTime.TimeYear + 2000;

        snprintf(m_resultText, sizeof(m_resultText),
                 "Дата: %02d %s %d (%s)\nВремя: %02d:%02d:%02d",
                 (int)m_resultDateTime.TimeDMonth, mn, fullYear, wd,
                 (int)m_resultDateTime.TimeHor,
                 (int)m_resultDateTime.TimeMin,
                 (int)m_resultDateTime.TimeSec);
    }
    else if (m_mode == MODE_DATETIME_TO_EPOCH)
    {
        // Конвертация Дата/Время -> Unix Time
        m_resultEpoch = datetime2epoch(&m_dateTimeInput, tz_offset_sec);

        // Формируем результат: "Unix Time: 1775719800"
        snprintf(m_resultText, sizeof(m_resultText),
                 "Unix Time: %u",
                 (unsigned int)m_resultEpoch);
    }
    else
    {
        onError.emit_("Неизвестный режим преобразования.");
        return;
    }

    onResultReady.emit_();
}

// --- Геттер ---

const char* DateTimeModel::getResult() const
{
    return m_resultText;
}

// ============================================================================
// Функции конвертации
// ============================================================================

/* 1) Конвертация из Unix Time в TDateTime */
void DateTimeModel::epoch2datetime(unsigned int epoch, TDateTime *dt, long tz_offset_sec)
{
    unsigned int t;
    unsigned short d; // 16 бит для дней (до 2099 года хватает с запасом)
    unsigned short y;
    unsigned char m, leap, md;

    // Приводим отрицательное смещение к unsigned int — двоичное дополнение
    // даёт корректный результат при сложении/вычитании
    unsigned int tz = (unsigned int)tz_offset_sec;
    t = epoch + tz;

    dt->TimeSec = (unsigned char)(t % 60U);
    t /= 60U;
    dt->TimeMin = (unsigned char)(t % 60U);
    t /= 60U;
    dt->TimeHor = (unsigned char)(t % 24U);
    t /= 24U;

    d = (unsigned short)t;

    dt->TimeWeek = (unsigned char)(((d + 3U) % 7U) + 1U);

    /* Расчет года (16-битная математика) */
    y = 1970U + (unsigned short)((d / 1461U) * 4U);
    d = d % 1461U;

    if (d >= 1096U) { y += 3U; d -= 1096U; }
    else if (d >= 730U) { y += 2U; d -= 730U; }
    else if (d >= 365U) { y += 1U; d -= 365U; }

    dt->TimeYear = (unsigned char)(y % 100U);

    /* Расчет месяца и дня БЕЗ массива */
    leap = (unsigned char)((y % 4U == 0U) ? 1U : 0U);

    m = 1U;
    while (m <= 12U) {
        if (m == 2U) {           // Февраль
            md = 28U + leap;
        } else if (m == 4U || m == 6U || m == 9U || m == 11U) { // 30 дней
            md = 30U;
        } else {                 // 31 день
            md = 31U;
        }

        if (d < md) {
            break;
        }

        d -= md;
        m++;
    }

    dt->TimeDMonth = (unsigned char)(d + 1U);
    dt->TimeMonth = m;
}

/* 2) Конвертация из TDateTime в Unix Time */
unsigned int DateTimeModel::datetime2epoch(const TDateTime *dt, long tz_offset_sec)
{
    unsigned short y;
    unsigned int days;
    unsigned char m, md;

    y = (dt->TimeYear >= 70U) ? (1900U + dt->TimeYear) : (2000U + dt->TimeYear);

    // 492 - количество високосных лет до 1970
    days = (unsigned int)(y - 1970U) * 365U + (unsigned int)((y - 1U) / 4U - 492U);

    /* Добавляем дни прошедших месяцев текущего года БЕЗ массива */
    m = dt->TimeMonth - 1U;
    while (m > 0U) {
        m--;
        if (m == 1U) {           // Февраль
            md = 28U + ((y % 4U == 0U) ? 1U : 0U);
        } else if (m == 3U || m == 5U || m == 8U || m == 10U) { // Апрель, Июнь, Сентябрь, Ноябрь
            md = 30U;
        } else {                 // Остальные
            md = 31U;
        }
        days += md;
    }

    days += dt->TimeDMonth - 1U;

    // Приводим отрицательное смещение к unsigned int — двоичное дополнение
    unsigned int tz = (unsigned int)tz_offset_sec;

    /* Вычитаем смещение часового пояса */
    return days * 86400U + (unsigned int)dt->TimeHor * 3600U
         + (unsigned int)dt->TimeMin * 60U + (unsigned int)dt->TimeSec - tz;
}

// ============================================================================
// Вспомогательные функции форматирования
// ============================================================================

const char* DateTimeModel::getWeekdayName(unsigned char weekday) const
{
    switch (weekday) {
        case 1:  return "Понедельник";
        case 2:  return "Вторник";
        case 3:  return "Среда";
        case 4:  return "Четверг";
        case 5:  return "Пятница";
        case 6:  return "Суббота";
        case 7:  return "Воскресенье";
        default: return "Неизвестно";
    }
}

const char* DateTimeModel::getMonthName(unsigned char month) const
{
    switch (month) {
        case 1:  return "Января";
        case 2:  return "Февраля";
        case 3:  return "Марта";
        case 4:  return "Апреля";
        case 5:  return "Мая";
        case 6:  return "Июня";
        case 7:  return "Июля";
        case 8:  return "Августа";
        case 9:  return "Сентября";
        case 10: return "Октября";
        case 11: return "Ноября";
        case 12: return "Декабря";
        default: return "Неизвестно";
    }
}


// ============================================================================
// Фабричная функция
// ============================================================================

IModel* createDateTimeModel() { return new DateTimeModel(); }
