// ============================================================================
// DateTimeModel.h — Реализация интерфейса IModel для преобразования времени
// Borland C++ Builder 6.0 / C++98
// ============================================================================
//
// DateTimeModel хранит параметры преобразования и выполняет конвертацию
// между Unix Time и TDateTime с учётом заданного часового пояса.
//
// Функции конвертации:
//   • Жёстко заданное смещение GMT+03:00 заменено на параметр
//   • Расчёт месяцев без массива (экономия памяти)
//
// ============================================================================

#ifndef DATETIME_MODEL_H
#define DATETIME_MODEL_H

#include "IModel.h"

class DateTimeModel : public IModel {
public:
    DateTimeModel();
    ~DateTimeModel();

    // --- IModel: управление ---

    void setTimezoneOffset(int offset_hours);
    void setConversionMode(int mode);
    void setEpochValue(unsigned int epoch);
    void setDateTime(const TDateTime& dt);
    void convert();

    // --- IModel: геттеры ---

    const char* getResult() const;

private:
    int      m_tzOffsetHours;    // Часовой пояс (в часах)
    int      m_mode;             // Направление преобразования
    unsigned int m_epochInput;  // Входное Unix Time
    TDateTime m_dateTimeInput;   // Входная дата/время
    TDateTime m_resultDateTime;  // Результат (epoch2datetime)
    unsigned int m_resultEpoch; // Результат (datetime2epoch)
    char      m_resultText[MAX_RESULT_LEN]; // Форматированный результат

    // ========================================================================
    // Функции конвертации
    // ========================================================================

    // Конвертация Unix Time -> TDateTime
    // tz_offset_sec — смещение часового пояса в секундах
    void epoch2datetime(unsigned int epoch, TDateTime* dt, long tz_offset_sec);

    // Конвертация TDateTime -> Unix Time
    // tz_offset_sec — смещение часового пояса в секундах
    unsigned int datetime2epoch(const TDateTime* dt, long tz_offset_sec);

    // ========================================================================
    // Вспомогательные функции форматирования
    // ========================================================================

    // Получить название дня недели по номеру (1=Пн ... 7=Вс)
    const char* getWeekdayName(unsigned char weekday) const;

    // Получить название месяца в родительном падеже (1=Января ... 12=Декабря)
    const char* getMonthName(unsigned char month) const;
};

#endif // DATETIME_MODEL_H
