#ifdef _WIN32
// ============================================================================
// ФАЙЛ РЕАЛИЗАЦИИ: console_settings_keeper.cpp
// ============================================================================
#pragma hdrstop
// Этот файл содержит "внутренности" нашего класса. То, как именно он
// перехватывает обращения к Windows и переводит текст из современного формата
// (UTF-8) в старый формат консоли (DOS-866).
// ============================================================================

// Отключаем предупреждения компилятора о том, что мы используем старые,
// но рабочие функции ввода/вывода (вроде printf или strcpy)
#define _CRT_SECURE_NO_WARNINGS

// Подключаем наш заголовочный файл, чтобы знать, какие у нас есть переменные и классы
#include "console_settings_keeper.h"

// Подключаем системные библиотеки Windows
#include <windows.h>   // Главный файл Windows (окна, консоль, память)
#include <stdio.h>     // Базовый ввод/вывод (printf, fprintf)
#include <conio.h>     // Функции для работы с клавиатурой консоли (_getch)
#include <string.h>    // Работа со строками (strcpy, strncmp)
#include <tlhelp32.h>  // Функции для "подглядывания" за памятью программы (CreateToolhelp32Snapshot)
#include <ctype.h>     // Функции проверки символов (например, перевод буквы в нижний регистр)

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ СТАТИЧЕСКОЙ ПЕРЕМЕННОЙ КЛАССА
// ============================================================================
// В классе ConsoleSettingsKeeper мы объявили переменную m_isDestroyed.
// Так как она статическая (общая для всех), в C++ нужно выделить для нее
// реальную память в каком-то из cpp файлов. Мы делаем это здесь.
// Изначально ставим значение false (программа еще не завершалась).
bool ConsoleSettingsKeeper::m_isDestroyed = false;

// ============================================================================
// ТИПЫ ДАННЫХ ДЛЯ ПЕРЕХВАТА ("ШАБЛОНЫ" СИСТЕМНЫХ ФУНКЦИЙ)
// ============================================================================
// Чтобы подменить системную функцию Windows, нам нужно знать, как она выглядит:
// какие аргументы она принимает и что возвращает. Ниже создаются "шаблоны"
// (указатели на функции) для шести функций, через которые Windows выводит
// и читает текст.
typedef BOOL (WINAPI *pfnWriteFile)(HANDLE, const void*, unsigned long, unsigned long*, void*);
typedef BOOL (WINAPI *pfnWriteConsoleA)(HANDLE, const void*, unsigned long, unsigned long*, void*);
typedef BOOL (WINAPI *pfnWriteConsoleW)(HANDLE, const void*, unsigned long, unsigned long*, void*);

typedef BOOL (WINAPI *pfnReadFile)(HANDLE, void*, unsigned long, unsigned long*, void*);
typedef BOOL (WINAPI *pfnReadConsoleA)(HANDLE, void*, unsigned long, unsigned long*, void*);
typedef BOOL (WINAPI *pfnReadConsoleW)(HANDLE, void*, unsigned long, unsigned long*, void*);

// ============================================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ (ВНУТРЕННЯЯ КУХНЯ)
// ============================================================================
// Эти переменные скрыты от внешнего мира. Они нужны только для того,
// чтобы наш "переводчик" мог помнить свое состояние.

// Дескрипторы (идентификаторы) стандартных потоков: вывода (экран) и ввода (клавиатура)
static HANDLE g_hStdOut = NULL;
static HANDLE g_hStdIn  = NULL;

// В эти переменные мы сохраним настоящие адреса системных функций Windows.
// Когда мы подменим функцию на свою, нам всё равно придется иногда просить
// настоящую функцию Windows выполнить реальную работу (нарисовать букву на экране).
static pfnWriteFile       g_Real_WriteFile     = NULL;
static pfnWriteConsoleA   g_Real_WriteConsoleA = NULL;
static pfnWriteConsoleW   g_Real_WriteConsoleW = NULL;

static pfnReadFile        g_Real_ReadFile      = NULL;
static pfnReadConsoleA    g_Real_ReadConsoleA  = NULL;
static pfnReadConsoleW    g_Real_ReadConsoleW  = NULL;

// В какой кодировке сейчас работает программа (та, что задана в ApplyRussianSettings)
static SourceEncoding g_ActiveEncoding = SE_NONE;

// Вспомогательный буфер (временная память) для русского текста.
// В современном стандарте UTF-8 одна русская буква состоит из 2 байт.
// Программа может отправить эти байты по отдельности. Мы копим их здесь,
// пока не соберем целую букву, и только тогда переводим.
static char g_utf8_buffer[4096];
static int  g_buf_len = 0;

// Флаги защиты, чтобы наша система настройки не попыталась запуститься дважды
static BOOL g_isInitialized     = FALSE;
static BOOL g_isInstanceCreated = FALSE;

// ========================================================================
// СИСТЕМА ОТЛАДКИ (Логирование ошибок)
// ========================================================================
// Если что-то пойдет не так (например, не найдется нужная системная функция),
// мы запишем об этом в этот массив (блокнот).
#define MAX_DEBUG_LOG 200 // Максимум 200 записей
static char g_DebugLog[MAX_DEBUG_LOG][128]; // Массив строк по 128 символов
static int  g_DebugLogCount = 0;           // Счетчик текущего количества записей

// Функция добавления записи в наш блокнот
void DbgAdd(const char* msg)
{
    // Если блокнот еще не заполнен до конца
    if (g_DebugLogCount < MAX_DEBUG_LOG)
    {
        // Копируем текст сообщения в массив
        strncpy(g_DebugLog[g_DebugLogCount], msg, 127);

        // Обязательно ставим нулевой символ в самом конце, чтобы программа
        // знала, где заканчивается текст (стандарт языка Си)
        g_DebugLog[g_DebugLogCount][127] = '\0';

        // Переходим к следующей свободной строчке
        g_DebugLogCount++;
    }
}

// ============================================================================
// ФУНКЦИИ ПЕРЕВОДА ТЕКСТА (ДВИЖОК ПЕРЕВОДЧИКА)
// ============================================================================

// Главная функция перевода при ВЫВОДЕ текста на экран.
// Она вызывается каждый раз, когда программа (например, Qt) хочет что-то напечатать.
// data - текст, который программа хочет напечатать.
// len - размер этого текста в байтах.
void ConvertOutput(const char* data, DWORD len)
{
    wchar_t wstr[4096]; // Массив для текста в универсальном формате Windows (Unicode)
    char    cstr[8192]; // Массив для текста в формате старой консоли (DOS-866)
    int wlen;           // Количество символов после перевода в Unicode
    int clen;           // Количество символов после перевода в DOS-866
    DWORD i;            // Счетчик для перебора байтов
    unsigned char fb;   // Первый байт (используется для определения размера буквы)
    int expected_len;   // Сколько байт занимает текущая буква
    DWORD written = 0;  // Служебная переменная для Windows

    // Если программа написана в старой кодировке (Windows-1251) или UTF-8 с меткой
    if (g_ActiveEncoding == SE_ANSI_1251 || g_ActiveEncoding == SE_UTF8_BOM)
    {
        // ШАГ 1: Переводим текст из формата программы (1251) в универсальный формат (Unicode)
        // Сначала спрашиваем Windows: "Сколько символов получится?"
        wlen = MultiByteToWideChar(1251, 0, data, (int)len, NULL, 0);

        // Если перевод возможен и результат не превышает размер нашего буфера
        if ((wlen > 0) && (wlen < 4096))
        {
            // Выполняем реальный перевод
            MultiByteToWideChar(1251, 0, data, (int)len, wstr, wlen);

            // ШАГ 2: Переводим из универсального формата (Unicode) в язык консоли (DOS-866)
            clen = WideCharToMultiByte(866, 0, wstr, wlen, NULL, 0, NULL, NULL);

            if ((clen > 0) && (clen < 8192))
            {
                // Выполняем реальный перевод
                WideCharToMultiByte(866, 0, wstr, wlen, cstr, clen, NULL, NULL);

                // ШАГ 3: Отдаем переведенный текст НАСТОЯЩЕЙ функции Windows для вывода на экран
                written = 0;
                if (g_Real_WriteConsoleA)
                {
                    g_Real_WriteConsoleA(g_hStdOut, cstr, (DWORD)clen, &written, NULL);
                }
            }
        }
    }
    // Если программа написана в современном стандарте (UTF-8 без метки) - НАШ ОСНОВНОЙ РЕЖИМ
    else if (g_ActiveEncoding == SE_UTF8_NO_BOM)
    {
        // В UTF-8 русская буква занимает 2 байта. Но программа может передать нам
        // эти байты по одному (сначала первый байт буквы "А", потом второй).
        // Поэтому мы сначала просто скидываем все пришедшие байты в кучу (в буфер).
        for (i = 0; i < len; i++)
        {
            if (g_buf_len < (int)sizeof(g_utf8_buffer) - 1)
            {
                g_utf8_buffer[g_buf_len++] = data[i];
            }
        }

        // Теперь пытаемся перевести то, что накопилось в буфере
        while (g_buf_len > 0)
        {
            // Смотрим на самый первый байт в буфере
            fb = g_utf8_buffer[0];

            // По умолчанию считаем, что это обычный английский символ (занимает 1 байт)
            expected_len = 1;

            // В формате UTF-8 байты русских букв всегда начинаются с определенных
            // комбинаций нулей и единиц. Мы проверяем эти комбинации:

            if ((fb & 0xE0) == 0xC0) // Начало обычной русской буквы (например, 'А')
            {
                expected_len = 2; // Значит буква состоит из 2-х байт
            }
            else if ((fb & 0xF0) == 0xE0) // Начало редких символов (например, 'Ё' или эмодзи)
            {
                expected_len = 3; // Значит символ состоит из 3-х байт
            }
            else if ((fb & 0xF8) == 0xF0) // Начало некоторых сложных эмодзи
            {
                expected_len = 4; // Состоит из 4-х байт
            }

            // Если в буфере еще не все части буквы собраны (например, ждем второй байт),
            // то прерываемся и ждем следующего вызова функции, когда придут новые данные
            if (g_buf_len < expected_len)
            {
                break;
            }

            // Если вся буква (все её байты) собрана в буфере, начинаем перевод
            wlen = MultiByteToWideChar(CP_UTF8, 0, g_utf8_buffer, expected_len, NULL, 0);

            if ((wlen > 0) && (wlen < 4096))
            {
                // Переводим из UTF-8 в Unicode
                MultiByteToWideChar(CP_UTF8, 0, g_utf8_buffer, expected_len, wstr, wlen);

                // Переводим из Unicode в язык консоли DOS-866
                clen = WideCharToMultiByte(866, 0, wstr, wlen, NULL, 0, NULL, NULL);

                if ((clen > 0) && (clen < 8192))
                {
                    WideCharToMultiByte(866, 0, wstr, wlen, cstr, clen, NULL, NULL);

                    // Выводим переведенную букву на экран через настоящую функцию Windows
                    written = 0;
                    if (g_Real_WriteConsoleA)
                    {
                        g_Real_WriteConsoleA(g_hStdOut, cstr, (DWORD)clen, &written, NULL);
                    }
                }
            }

            // Удаляем переведенную букву из буфера (сдвигаем оставшуюся память в начало)
            g_buf_len -= expected_len;
            memmove(g_utf8_buffer, g_utf8_buffer + expected_len, g_buf_len);
        }
    }
}

// Функция перевода текста при ВВОДЕ с клавиатуры (для старых режимов кодировки)
void ConvertInputAnsiOverwrite(char* buffer, DWORD bytesRead)
{
    wchar_t wstr[4096];
    int wlen;

    if (bytesRead == 0)
    {
        return; // Если ничего не ввели, выходим
    }

    // Консоль выдает нам текст в своем формате (DOS-866).
    // Переводим его в универсальный формат (Unicode)
    wlen = MultiByteToWideChar(866, 0, buffer, (int)bytesRead, wstr, 4096);

    if (wlen > 0)
    {
        // Переводим из универсального формата в формат, который ожидает программа (Windows-1251)
        // И затираем исходный буфер новым текстом
        WideCharToMultiByte(1251, 0, wstr, wlen, buffer, (int)bytesRead, NULL, NULL);
    }
}

// Специальная функция для правильного ввода русского текста в режиме UTF-8
BOOL ReadAndConvertToUtf8(HANDLE hConsoleInput, void* inBuffer, unsigned long inBufferSize, unsigned long* outCharsRead)
{
    char* charBuffer = (char*)inBuffer;
    unsigned long maxBytes = inBufferSize;
    unsigned long safeLimit;
    int wTotalLen = 0;
    int utf8Len;
    unsigned long wRead = 0;
    unsigned char c;
    wchar_t wstr[2];
    wchar_t wFullStr[8192];

    // Проверка на наличие настоящей функции чтения Windows
    if (!g_Real_ReadConsoleW || !outCharsRead)
    {
        DbgAdd("!! FATAL RUNTIME: ReadAndConvertToUtf8 called, but ReadConsoleW is NULL !!");
        return FALSE;
    }

    if (maxBytes < 2)
    {
        return FALSE;
    }

    safeLimit = maxBytes - 1;

    // Читаем символы по одному, пока пользователь не нажмет Enter
    while (wTotalLen < 8190)
    {
        wRead = 0;

        // Просим настоящую функцию Windows прочитать ровно 1 символ
        // Читаем сразу в универсальном формате (Unicode), чтобы избежать искажений
        if (!g_Real_ReadConsoleW(hConsoleInput, wstr, 1, &wRead, NULL) || wRead == 0)
        {
            break; // Произошла ошибка или консоль закрыли
        }

        // Игнорируем служебные невидимые клавиши (возврат каретки и перевод строки)
        if (wstr[0] == L'\r')
        {
            g_Real_ReadConsoleW(hConsoleInput, wstr, 1, &wRead, NULL);
            break; // Это нажатие Enter, выходим из цикла чтения
        }

        if (wstr[0] == L'\n')
        {
            break;
        }

        // Сохраняем введенный символ в массив
        wFullStr[wTotalLen++] = wstr[0];
    }

    if (wTotalLen == 0)
    {
        *outCharsRead = 0;
        return TRUE;
    }

    // Теперь переводим все собранные символы из универсального формата в UTF-8
    utf8Len = WideCharToMultiByte(CP_UTF8, 0, wFullStr, wTotalLen, NULL, 0, NULL, NULL);

    if (utf8Len <= 0)
    {
        utf8Len = 0;
    }

    // Защита от переполнения памяти буфера
    if ((unsigned long)utf8Len > safeLimit)
    {
        utf8Len = (int)safeLimit;
    }

    if (utf8Len > 0)
    {
        WideCharToMultiByte(CP_UTF8, 0, wFullStr, wTotalLen, charBuffer, utf8Len, NULL, NULL);

        // Очищаем хвост от возможных "разорванных" остатков многобайтовых символов
        while (utf8Len > 0)
        {
            c = (unsigned char)charBuffer[utf8Len];
            if (c >= 0x80 && c <= 0xBF)
            {
                utf8Len--;
            }
            else
            {
                break;
            }
        }
    }

    // Добавляем в самый конец символ новой строки (как делает обычный ввод с клавиатуры)
    charBuffer[utf8Len] = '\n';

    // Сообщаем программе, сколько байт мы ей вернули
    *outCharsRead = (unsigned long)(utf8Len + 1);

    return TRUE;
}

// ============================================================================
// НАШИ ПОДМЕНЕННЫЕ ФУНКЦИИ (ХУКИ / ПЕРЕХВАТЧИКИ)
// ============================================================================
// Когда программа хочет вывести текст, она обращается к системной функции Windows.
// Но благодаря "магии" (которая будет ниже), адрес системной функции заменен
// на адрес наших функций ниже. Управление приходит сюда СНАЧАЛА.
// Мы переводим текст, а затем передаем его настоящей функции Windows.

// Подмена функции записи в файл (в Windows консоль тоже считается файлом)
BOOL WINAPI Hooked_WriteFile(HANDLE hFile, const void* lpBuffer, unsigned long nNumberOfBytesToWrite, unsigned long* lpNumberOfBytesWritten, void* lpOverlapped)
{
    // Проверяем: это запись именно в нашу консоль и переводчик включен?
    if (hFile == g_hStdOut && g_ActiveEncoding != SE_NONE)
    {
        // Вызываем наш движок перевода
        ConvertOutput((const char*)lpBuffer, nNumberOfBytesToWrite);

        // Обманываем программу: говорим ей, что всё прошло успешно
        if (lpNumberOfBytesWritten)
        {
            *lpNumberOfBytesWritten = nNumberOfBytesToWrite;
        }
        return TRUE;
    }

    // Если это запись куда-то в другой файл (не в консоль), мы не вмешиваемся.
    // Просто передаем данные настоящей функции Windows.
    if (g_Real_WriteFile)
    {
        return g_Real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
    }

    return FALSE;
}

// Подмена функции вывода текста в консоль (ANSI - однобайтовая версия)
BOOL WINAPI Hooked_WriteConsoleA(HANDLE hConsoleOutput, const void* lpBuffer, unsigned long nNumberOfCharsToWrite, unsigned long* lpNumberOfCharsWritten, void* lpReserved)
{
    if (hConsoleOutput == g_hStdOut && g_ActiveEncoding != SE_NONE)
    {
        ConvertOutput((const char*)lpBuffer, nNumberOfCharsToWrite);

        if (lpNumberOfCharsWritten)
        {
            *lpNumberOfCharsWritten = nNumberOfCharsToWrite;
        }
        return TRUE;
    }

    if (g_Real_WriteConsoleA)
    {
        return g_Real_WriteConsoleA(hConsoleOutput, lpBuffer, nNumberOfCharsToWrite, lpNumberOfCharsWritten, lpReserved);
    }

    return FALSE;
}

// Подмена функции вывода текста в консоль (Unicode - двухбайтовая версия)
BOOL WINAPI Hooked_WriteConsoleW(HANDLE hConsoleOutput, const void* lpBuffer, unsigned long nNumberOfCharsToWrite, unsigned long* lpNumberOfCharsWritten, void* lpReserved)
{
    int clen;
    char cstr[8192];
    DWORD written = 0;

    if (hConsoleOutput == g_hStdOut && g_ActiveEncoding != SE_NONE)
    {
        // Здесь текст уже приходит в универсальном формате.
        // Нам нужно перевести его только в формат консоли (DOS-866).
        clen = WideCharToMultiByte(866, 0, (const wchar_t*)lpBuffer, nNumberOfCharsToWrite, NULL, 0, NULL, NULL);

        if (clen > 0 && clen < 8192)
        {
            WideCharToMultiByte(866, 0, (const wchar_t*)lpBuffer, nNumberOfCharsToWrite, cstr, clen, NULL, NULL);

            // Отдаем переведенный текст оригинальной функции
            if (g_Real_WriteConsoleA)
            {
                g_Real_WriteConsoleA(g_hStdOut, cstr, (DWORD)clen, &written, NULL);
            }
            else if (g_Real_WriteFile)
            {
                g_Real_WriteFile(g_hStdOut, cstr, (DWORD)clen, &written, NULL);
            }
        }

        if (lpNumberOfCharsWritten)
        {
            *lpNumberOfCharsWritten = nNumberOfCharsToWrite;
        }
        return TRUE;
    }

    if (g_Real_WriteConsoleW)
    {
        return g_Real_WriteConsoleW(hConsoleOutput, lpBuffer, nNumberOfCharsToWrite, lpNumberOfCharsWritten, lpReserved);
    }

    return FALSE;
}

// Подмена функции чтения из файла (ввод с клавиатуры)
BOOL WINAPI Hooked_ReadFile(HANDLE hFile, void* lpBuffer, unsigned long nNumberOfBytesToRead, unsigned long* lpNumberOfBytesRead, void* lpOverlapped)
{
    BOOL result;

    // Если читают именно с клавиатуры и переводчик работает
    if (hFile == g_hStdIn && g_ActiveEncoding != SE_NONE)
    {
        // Если включен современный режим UTF-8
        if (g_ActiveEncoding == SE_UTF8_NO_BOM)
        {
            return ReadAndConvertToUtf8(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead);
        }

        // Если старый режим, читаем как обычно через настоящую функцию
        result = g_Real_ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

        // Если что-то прочитали, переводим это
        if (result && lpNumberOfBytesRead && *lpNumberOfBytesRead != 0)
        {
            ConvertInputAnsiOverwrite((char*)lpBuffer, *lpNumberOfBytesRead);
        }
        return result;
    }

    // Если читают не с клавиатуры (например, из файла на диске), не мешаем
    if (g_Real_ReadFile)
    {
        return g_Real_ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    }

    return FALSE;
}

// Подмена функции чтения с консоли (ANSI)
BOOL WINAPI Hooked_ReadConsoleA(HANDLE hConsoleInput, void* lpBuffer, unsigned long nNumberOfCharsToRead, unsigned long* lpNumberOfCharsRead, void* lpReserved)
{
    BOOL result;

    if (hConsoleInput == g_hStdIn && g_ActiveEncoding != SE_NONE)
    {
        if (g_ActiveEncoding == SE_UTF8_NO_BOM)
        {
            return ReadAndConvertToUtf8(hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead);
        }

        result = g_Real_ReadConsoleA(hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, lpReserved);

        if (result && lpNumberOfCharsRead && *lpNumberOfCharsRead != 0)
        {
            ConvertInputAnsiOverwrite((char*)lpBuffer, *lpNumberOfCharsRead);
        }
        return result;
    }

    if (g_Real_ReadConsoleA)
    {
        return g_Real_ReadConsoleA(hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, lpReserved);
    }

    return FALSE;
}

// Подмена функции чтения с консоли (Unicode)
BOOL WINAPI Hooked_ReadConsoleW(HANDLE hConsoleInput, void* lpBuffer, unsigned long nNumberOfCharsToRead, unsigned long* lpNumberOfCharsRead, void* pInputControl)
{
    BOOL result;
    wchar_t* wstr;
    char cstr866[8192];
    int len866;
    int newWlen;

    if (hConsoleInput == g_hStdIn && g_ActiveEncoding != SE_NONE)
    {
        if (g_ActiveEncoding == SE_UTF8_NO_BOM)
        {
            return ReadAndConvertToUtf8(hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead);
        }

        // Читаем в универсальном формате
        result = g_Real_ReadConsoleW(hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, pInputControl);

        if (result && lpNumberOfCharsRead && *lpNumberOfCharsRead != 0)
        {
            wstr = (wchar_t*)lpBuffer;
            // Переводим из формата консоли в универсальный
            len866 = WideCharToMultiByte(866, 0, wstr, (int)*lpNumberOfCharsRead, cstr866, sizeof(cstr866), NULL, NULL);

            if (len866 > 0)
            {
                // И обратно в формат программы, затирая буфер
                newWlen = MultiByteToWideChar(1251, 0, cstr866, len866, wstr, (int)*lpNumberOfCharsRead);
                if (newWlen > 0)
                {
                    *lpNumberOfCharsRead = newWlen;
                }
            }
        }
        return result;
    }

    if (g_Real_ReadConsoleW)
    {
        return g_Real_ReadConsoleW(hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, pInputControl);
    }

    return FALSE;
}

// ============================================================================
// ФУНКЦИИ ПОИСКА И ПОДМЕНЫ АДРЕСОВ В ПАМЯТИ (IAT HOOKING)
// ============================================================================
// Это самая сложная, но самая важная часть. Как наша функция Hooked_WriteFile
// перехватывает управление? Программа при запуске имеет "телефонную книгу"
// (таблицу импорта - IAT), где записано: "если мне понадобится вывести текст,
// звони по номеру функции WriteFile". Мы находим эту книгу в памяти программы,
// стираем старый номер карандашом и вписываем туда номер нашей функции.

// Вспомогательная функция: проверяет, содержится ли одно слово в другом
// (без учета больших/маленьких букв)
int StrContainsI(const char* str, const char* target)
{
    if (!str || !target)
    {
        return 0;
    }

    while (*str)
    {
        const char *s = str;
        const char *t = target;

        // Посимвольно сравниваем слова
        while (*s && *t && tolower((unsigned char)*s) == tolower((unsigned char)*t))
        {
            s++;
            t++;
        }

        // Если дошли до конца искомого слова, значит совпадение найдено
        if (*t == '\0')
        {
            return 1;
        }
        str++;
    }
    return 0;
}

// Вспомогательная функция: переводит имя файла из формата Windows в простой текст
void SafeCopyModNameW(const WCHAR* src, char* dst, unsigned int dstSize)
{
    if (WideCharToMultiByte(CP_ACP, 0, src, -1, dst, dstSize, NULL, NULL) == 0)
    {
        dst[0] = '\0';
    }
}

// ГЛАВНАЯ ФУНКЦИЯ ПОДМЕНЫ. Она ищет "телефонную книгу" в каждой библиотеке
// программы (например, в Qt5Core.dll) и подменяет там номера.
void HookAllModulesIAT(const char* targetFuncName, void* pOurHook, void** ppRealFunc)
{
    HANDLE hSnap;
    MODULEENTRY32W me;
    int i;
    char modNameA[256];
    char logMsg[128];
    BYTE* base;
    IMAGE_DOS_HEADER* dos;
    IMAGE_NT_HEADERS* nt;
    IMAGE_IMPORT_DESCRIPTOR* impDesc;

    IMAGE_THUNK_DATA* pINT;
    IMAGE_THUNK_DATA* pIAT;
    IMAGE_IMPORT_BY_NAME* pName;
    DWORD oldProtect = 0;

    // Делаем "слепок" (список) всех загруженных в программу библиотек
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());

    if (hSnap == INVALID_HANDLE_VALUE)
    {
        return;
    }

    me.dwSize = sizeof(me);

    // Начинаем перебирать все библиотеки одну за другой
    if (Module32FirstW(hSnap, &me))
    {
        do
        {
            SafeCopyModNameW(me.szModule, modNameA, sizeof(modNameA));

            // КРИТИЧЕСКИ ВАЖНОЕ ПРАВИЛО БЕЗОПАСНОСТИ:
            // Системные библиотеки Windows (kernel32, ntdll и др.) трогать КАТЕГОРИЧЕСКИ НЕЛЬЗЯ!
            // Если мы сломаем их "телефонную книгу", Windows тут же "убьет" нашу программу
            // с фатальной ошибкой. Поэтому мы их просто пропускаем.
            if (StrContainsI(modNameA, "kernel32")) continue;
            if (StrContainsI(modNameA, "kernelbase")) continue;
            if (StrContainsI(modNameA, "ntdll")) continue;
            if (StrContainsI(modNameA, "user32")) continue;
            if (StrContainsI(modNameA, "gdi32")) continue;
            if (StrContainsI(modNameA, "advapi32")) continue;
            if (StrContainsI(modNameA, "ole32")) continue;
            if (StrContainsI(modNameA, "shell32")) continue;
            if (StrContainsI(modNameA, "ws2_32")) continue;

            base = (BYTE*)me.hModule; // Базовый адрес библиотеки в оперативной памяти

            // Читаем заголовки файла (у каждого exe и dll файла есть спец. структура)
            dos = (IMAGE_DOS_HEADER*)base;
            if (dos->e_magic != IMAGE_DOS_SIGNATURE)
            {
                continue; // Это не программа, пропускаем
            }

            nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
            if (nt->Signature != IMAGE_NT_SIGNATURE)
            {
                continue; // Заголовок поврежден, пропускаем
            }

            // Проверяем, есть ли у этой библиотеки "телефонная книга" (таблица импорта)
            if (nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0)
            {
                continue; // Нет книги - нечего подменять
            }

            // Находим саму книгу в памяти
            impDesc = (IMAGE_IMPORT_DESCRIPTOR*)(base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

            // Перебираем все записи в книге (каждая запись - это функция, которую библиотека хочет использовать)
            for (i = 0; impDesc[i].Characteristics != 0; i++)
            {
                pINT = (IMAGE_THUNK_DATA*)(base + impDesc[i].OriginalFirstThunk);
                pIAT = (IMAGE_THUNK_DATA*)(base + impDesc[i].FirstThunk);

                if (!pINT || !pIAT)
                {
                    continue;
                }

                // Перебираем сами функции
                while (pINT->u1.AddressOfData != 0)
                {
                    // Убеждаемся, что функция ищется по имени (а не по секретному номеру)
                    if (!(pINT->u1.Ordinal & IMAGE_ORDINAL_FLAG))
                    {
                        pName = (IMAGE_IMPORT_BY_NAME*)(base + pINT->u1.AddressOfData);

                        // Если мы нашли в книге нужную нам функцию (например, WriteFile)
                        if (strcmp((const char*)pName->Name, targetFuncName) == 0)
                        {
                            if (*ppRealFunc == NULL)
                            {
                                // СОХРАНЯЕМ НАСТОЯЩИЙ НОМЕР (адрес), чтобы потом самим звонить в Windows
                                *ppRealFunc = (void*)pIAT->u1.Function;
                                sprintf(logMsg, "HOOK OK: %s in %s", targetFuncName, modNameA);
                                DbgAdd(logMsg);
                            }

                            // МЕНЯЕМ ЗАПИСЬ В КНИГЕ
                            // Память защищена от записи, поэтому просим Windows временно снять защиту
                            if (VirtualProtect(&pIAT->u1.Function, sizeof(void*), PAGE_READWRITE, &oldProtect))
                            {
                                // "Зачеркиваем старый номер и пишем адрес нашей функции"
                                pIAT->u1.Function = (ULONG_PTR)pOurHook;

                                // Возвращаем защиту памяти на место
                                VirtualProtect(&pIAT->u1.Function, sizeof(void*), oldProtect, &oldProtect);
                            }
                            break; // Нашли и подменили, выходим из цикла
                        }
                    }
                    pINT++;
                    pIAT++;
                }
            }
        }
        while (Module32NextW(hSnap, &me)); // Переходим к следующей библиотеке
    }

    CloseHandle(hSnap); // Закрываем "слепок"

    // Если мы так и не нашли функцию в библиотеках (редкая ситуация)
    if (*ppRealFunc == NULL)
    {
        sprintf(logMsg, "HOOK SKIP: %s not in modules (will use GetProcAddress)", targetFuncName);
        DbgAdd(logMsg);
    }
}

// Запасной способ найти адрес функции, если первый способ не сработал.
// Мы просто спрашиваем у Windows напрямую: "Где находится эта функция?"
void ResolveViaGetProcAddress(const char* funcName, HMODULE hDll, const char* dllName, void** ppRealFunc)
{
    char logMsg[128];
    void* pFunc;

    if (*ppRealFunc != NULL)
    {
        return; // Если адрес уже найден ранее, ничего не делаем
    }

    pFunc = (void*)GetProcAddress(hDll, funcName);

    if (pFunc != NULL)
    {
        *ppRealFunc = pFunc;
        sprintf(logMsg, "PLAN B OK: %s resolved via GetProcAddress(%s)", funcName, dllName);
        DbgAdd(logMsg);
    }
    else
    {
        sprintf(logMsg, "PLAN B FAIL: %s missing completely!", funcName);
        DbgAdd(logMsg);
    }
}

// Структура для сохранения оригинальных настроек консоли Windows
static struct
{
    HANDLE hConsole;
    UINT   originalOutputCP; // Оригинальная кодировка вывода
    UINT   originalInputCP;  // Оригинальная кодировка ввода
} g_Impl;

// ============================================================================
// ВНУТРЕННИЕ ФУНКЦИИ ИНИЦИАЛИЗАЦИИ И ПЕРЕКЛЮЧЕНИЯ РЕЖИМОВ
// ============================================================================

// Функция, которая делает всю черновую работу при запуске программы
static void internal_Init(void)
{
    HMODULE hK32;
    HMODULE hKB;

    // Защита от дурака: нельзя запускать эту систему дважды
    if (g_isInstanceCreated)
    {
        fprintf(stderr, "ConsoleSettingsKeeper: Нельзя создавать объект более одного раза!\n");
        return;
    }
    g_isInstanceCreated = TRUE;

    // Запоминаем, куда Windows выводит текст по умолчанию
    g_Impl.hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    g_hStdOut = g_Impl.hConsole;
    g_hStdIn = GetStdHandle(STD_INPUT_HANDLE);

    // Сохраняем текущие кодировки, чтобы вернуть их при выходе из программы
    g_Impl.originalOutputCP = GetConsoleOutputCP();
    g_Impl.originalInputCP = GetConsoleCP();

    if (g_isInitialized)
    {
        return;
    }
    g_isInitialized = TRUE;

    DbgAdd("Init: Universal IAT scan started...");

    // Запускаем процесс "взлома телефонных книг" для всех функций, связанных с текстом
    HookAllModulesIAT("WriteFile",     (void*)Hooked_WriteFile,     (void**)&g_Real_WriteFile);
    HookAllModulesIAT("WriteConsoleA", (void*)Hooked_WriteConsoleA, (void**)&g_Real_WriteConsoleA);
    HookAllModulesIAT("WriteConsoleW", (void*)Hooked_WriteConsoleW, (void**)&g_Real_WriteConsoleW);
    HookAllModulesIAT("ReadFile",      (void*)Hooked_ReadFile,      (void**)&g_Real_ReadFile);
    HookAllModulesIAT("ReadConsoleA",  (void*)Hooked_ReadConsoleA,  (void**)&g_Real_ReadConsoleA);
    HookAllModulesIAT("ReadConsoleW",  (void*)Hooked_ReadConsoleW,  (void**)&g_Real_ReadConsoleW);

    // Если в "книгах" не оказалось нужных функций (бывает крайне редко),
    // используем запасной метод - прямой запрос к системным библиотекам
    hK32 = GetModuleHandleA("kernel32.dll");
    hKB  = GetModuleHandleA("kernelbase.dll");

    if (hK32)
    {
        ResolveViaGetProcAddress("WriteFile",     hK32, "kernel32", (void**)&g_Real_WriteFile);
        ResolveViaGetProcAddress("WriteConsoleA", hK32, "kernel32", (void**)&g_Real_WriteConsoleA);
        ResolveViaGetProcAddress("WriteConsoleW", hK32, "kernel32", (void**)&g_Real_WriteConsoleW);
        ResolveViaGetProcAddress("ReadFile",      hK32, "kernel32", (void**)&g_Real_ReadFile);
        ResolveViaGetProcAddress("ReadConsoleA",  hK32, "kernel32", (void**)&g_Real_ReadConsoleA);
        ResolveViaGetProcAddress("ReadConsoleW",  hK32, "kernel32", (void**)&g_Real_ReadConsoleW);
    }

    if (hKB)
    {
        ResolveViaGetProcAddress("WriteFile", hKB, "kernelbase", (void**)&g_Real_WriteFile);
        ResolveViaGetProcAddress("ReadFile",  hKB, "kernelbase", (void**)&g_Real_ReadFile);
    }

    DbgAdd("Init: Scan finished.");
}

// Вспомогательная функция для переключения режимов перевода
static void internal_ApplyRussianSettings(SourceEncoding encoding)
{
    // Запоминаем выбранный режим
    g_ActiveEncoding = encoding;

    // В зависимости от режима, просим Windows переключить консоль
    if (encoding == SE_NONE)
    {
        // Режим "никакой" - вернуть всё как было
        SetConsoleOutputCP(g_Impl.originalOutputCP);
        SetConsoleCP(g_Impl.originalInputCP);
    }
    else if (encoding == SE_ANSI_1251 || encoding == SE_UTF8_BOM)
    {
        // Для старых режимов принудительно ставим кодировку консоли 866
        SetConsoleOutputCP(866);
        SetConsoleCP(866);
    }
    else if (encoding == SE_UTF8_NO_BOM)
    {
        // Для современного UTF-8: вывод принудительно переводим в 866,
        // а ввод оставляем как есть (наша функция ReadAndConvertToUtf8 сама все переведет)
        SetConsoleOutputCP(866);
        SetConsoleCP(g_Impl.originalInputCP);
    }
}

// ============================================================================
// РЕАЛИЗАЦИЯ МЕТОДОВ КЛАССА ConsoleSettingsKeeper
// ============================================================================
// Это то, что видит пользователь класса снаружи.

// Конструктор. Вызывается САМ автоматически, когда создается глобальный объект.
ConsoleSettingsKeeper::ConsoleSettingsKeeper()
{
    // Просто делегируем работу внутренней функции
    internal_Init();
}

// Деструктор. Вызывается САМ автоматически, когда программа закрывается.
ConsoleSettingsKeeper::~ConsoleSettingsKeeper()
{
    // Защита от повторного вызова деструктора (в C++ такое редко, но бывает)
    if (m_isDestroyed)
    {
        return;
    }
    m_isDestroyed = true;

    // Очищаем буфер ввода (на случай, если пользователь во время работы программы
    // случайно нажимал кнопки на клавиатуре, чтобы они не вывелися мусором)
    FlushConsoleInputBuffer(g_hStdIn);

    // Включаем правильный режим ввода перед тем, как просить нажать клавишу
    internal_ApplyRussianSettings(SE_ANSI_1251);

    // Выводим сообщение пользователю
    printf("\nНажмите любую клавишу для выхода из программы ...\n");

    // Программа встает паузой и ждет, пока пользователь нажмет любую кнопку
    _getch();

    // ВОССТАНОВЛЕНИЕ: Возвращаем консоли её оригинальные настройки, которые
    // были до того, как наша программа запустилась. (Чистим за собой!)
    SetConsoleOutputCP(g_Impl.originalOutputCP);
    SetConsoleCP(g_Impl.originalInputCP);
}

// Публичный метод для включения русского (вызывается из main.cpp)
void ConsoleSettingsKeeper::ApplyRussianSettings(SourceEncoding encoding)
{
    internal_ApplyRussianSettings(encoding);
}

// Публичный метод для вывода логов отладки
void ConsoleSettingsKeeper::PrintDebug(void)
{
    int i;
    // Просто выводим все сохраненные записи из нашего массива (блокнота)
    for (i = 0; i < g_DebugLogCount; i++)
    {
        printf("[LOG] %s\n", g_DebugLog[i]);
    }
}

// ============================================================================
// СОЗДАНИЕ ГЛОБАЛЬНОГО ОБЪЕКТА
// ============================================================================
// Эта строчка магическим образом создает объект в памяти ДО того, как начнет
// выполняться функция main(). Благодаря этому перехват функций настраивается
// на самых ранних этапах работы программы.
ConsoleSettingsKeeper consoleKeeper;

#endif // _WIN32
