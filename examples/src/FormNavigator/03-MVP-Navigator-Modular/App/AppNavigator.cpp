// ============================================================================
// AppNavigator.cpp -- Навигатор приложения (create-on-demand)
// ============================================================================

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <vcl.h>
#include "AppNavigator.h"
#include "HomeForm.h"
#include "DataForm.h"
#include "ResultForm.h"
#include "HomePresenter.h"
#include "DataPresenter.h"
#include "ResultPresenter.h"

// ============================================================================
// Таблица форм -- устраняет switch-case
//
// Каждая строка: фабрика формы + фабрика презентера.
// Индекс в массиве = значение enum MainFormType.
//
// Для добавления новой формы:
//   1. Добавить значение в enum MainFormType (в .h)
//   2. Написать две статические функции-фабрики ниже
//   3. Добавить строку в s_formTable
// ============================================================================

typedef IView*      (*FormCreateFunc)(bool isFirst);
typedef IPresenter* (*PresenterCreateFunc)(IView* view, INavigator* nav,
                                          const SharedData& data);

struct FormEntry {
    FormCreateFunc      createForm;
    PresenterCreateFunc createPresenter;
};

// ---- Фабрики форм ----

static IView* createHomeForm(bool isFirst)
{
    THomeForm* form;
    if (isFirst) { form = 0; Application->CreateForm(__classid(THomeForm), &form); }
    else         { form = new THomeForm(Application); }
    return static_cast<IHomeView*>(form);
}

static IView* createDataForm(bool isFirst)
{
    TDataForm* form;
    if (isFirst) { form = 0; Application->CreateForm(__classid(TDataForm), &form); }
    else         { form = new TDataForm(Application); }
    return static_cast<IDataView*>(form);
}

static IView* createResultForm(bool isFirst)
{
    TResultForm* form;
    if (isFirst) { form = 0; Application->CreateForm(__classid(TResultForm), &form); }
    else         { form = new TResultForm(Application); }
    return static_cast<IResultView*>(form);
}

// ---- Фабрики презентеров ----

static IPresenter* createHomePresenter(IView* view, INavigator* nav,
                                       const SharedData& data)
{
    return new HomePresenter(static_cast<IHomeView*>(view), nav, data);
}

static IPresenter* createDataPresenter(IView* view, INavigator* nav,
                                       const SharedData& data)
{
    return new DataPresenter(static_cast<IDataView*>(view), nav, data);
}

static IPresenter* createResultPresenter(IView* view, INavigator* nav,
                                         const SharedData& data)
{
    return new ResultPresenter(static_cast<IResultView*>(view), nav, data);
}

// ---- Таблица (индекс = MainFormType) ----

static const FormEntry s_formTable[] = {
    { createHomeForm,   createHomePresenter   },  // mftHome   = 0
    { createDataForm,   createDataPresenter   },  // mftData   = 1
    { createResultForm, createResultPresenter }    // mftResult = 2
};

// ============================================================================
// Конструктор
// ============================================================================

AppNavigator::AppNavigator()
    : m_mainFormType(mftNone)
{
    for (int i = 0; i < FORM_COUNT; ++i)
        m_formCache[i] = 0;
}

// ============================================================================
// Деструктор
// ============================================================================

AppNavigator::~AppNavigator()
{
    if (m_current.presenter) {
        m_current.presenter->destroy();
        delete m_current.presenter;
        m_current.presenter = 0;
    }
    m_current.view = 0;
    // Формы принадлежат Application -- VCL удалит их сама
}

// ============================================================================
// navigateTo -- универсальная навигация (без switch-case)
//
// Проверяет кэш (только MainForm кэшируется).
// Если формы нет -- создаёт через фабрику из таблицы.
// Если это первая форма -- назначает её MainForm и кэширует.
// Создаёт презентер через фабрику из таблицы и активирует триаду.
// ============================================================================

void AppNavigator::navigateTo(MainFormType type, const SharedData& data)
{
    IView* view = m_formCache[type];

    if (!view) {
        bool isFirst = (m_mainFormType == mftNone);
        view = s_formTable[type].createForm(isFirst);

        if (isFirst) {
            m_mainFormType = type;
            m_formCache[type] = view;      // Кэшируем MainForm
        }
    }

    IPresenter* presenter = s_formTable[type].createPresenter(view, this, data);
    activate(view, presenter);
}

// ============================================================================
// Навигация «домой» -- на MainForm
// ============================================================================

void AppNavigator::navigateToHome(const SharedData& data)
{
    MainFormType target = (m_mainFormType == mftNone) ? mftHome : m_mainFormType;
    navigateTo(target, data);
}

void AppNavigator::navigateToData(const SharedData& data)
{
    navigateTo(mftData, data);
}

void AppNavigator::navigateToResult(const SharedData& data)
{
    navigateTo(mftResult, data);
}

// ============================================================================
// activate -- создать новую триаду, инициализировать, показать
//
// Порядок: сначала показать новую форму, потом скрыть/освободить старую --
// это предотвращает мерцание.
//
// MainForm: скрывается (hideView), живёт вечно.
// Остальные формы: освобождаются (releaseView) -- отложенное удаление.
// ============================================================================

void AppNavigator::activate(IView* view, IPresenter* presenter)
{
    if (m_current.view == view) {
        return; // Переход к себе самой - ничего не делать
    }
    // Сохранить старую триаду для отложенной деактивации
    IView*      oldView      = m_current.view;
    IPresenter* oldPresenter = m_current.presenter;

    // Активировать новую триаду
    m_current.view      = view;
    m_current.presenter = presenter;

    presenter->initialize();    // Подписка на сигналы, отображение данных
    view->showView();           // Показать форму

    // Деактивировать старую триаду
    if (oldPresenter) {
        oldPresenter->destroy();    // Отписка от сигналов
        delete oldPresenter;        // Освобождение памяти
    }
    if (oldView) {
        if (m_mainFormType != mftNone && oldView == m_formCache[m_mainFormType]) {
            // Главная форма -- только скрыть (она MainForm, живёт вечно)
            oldView->hideView();
        } else {
            // Остальные формы -- отложенное удаление
            oldView->releaseView();
        }
    }
}

// ============================================================================
// closeCurrentView -- обработка закрытия текущего вида
//
// Если закрывается MainForm -- приложение завершается.
// Иначе -- переход на MainForm (которая может быть любой).
// ============================================================================

void AppNavigator::closeCurrentView(const SharedData& data)
{
    if (m_mainFormType != mftNone &&
        m_current.view == m_formCache[m_mainFormType])
    {
        // Закрыта главная форма -- завершить приложение
        if (m_current.presenter) {
            m_current.presenter->destroy();
            delete m_current.presenter;
            m_current.presenter = 0;
        }
        if (m_current.view) {
            m_current.view->hideView();
            m_current.view = 0;
        }
        Application->Terminate();
    }
    else
    {
        // Переход на главную форму (MainForm = «дом»)
        navigateToHome(data);
    }
}
