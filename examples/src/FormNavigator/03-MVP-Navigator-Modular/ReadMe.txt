============================================================
Navigator -- Демонстрация MVP + Навигатор + Сигналы/Слоты + VCL GUI
Borland C++ Builder 6.0 / C++98
Группа проектов (Shared + Business.lib + Forms.lib + App.exe)
============================================================

НАЗНАЧЕНИЕ
----------
Приложение демонстрирует навигацию между тремя VCL-формами
с использованием паттерна MVP (Model-View-Presenter) и
библиотеки сигналов/слотов (signal_impl.h — header-only).

Архитектура основана на паттерне «Навигатор»:
  - Каждый экран имеет свой собственный Presenter
  - Навигатор управляет сборкой MVP-триад и жизненным циклом
  - Подписка/отписка на события через Initialize()/Destroy()
  - Формы создаются по требованию (create-on-demand)
  - Корректное завершение при закрытии главной формы

Три формы:
  1. HomeForm   (Главная)       — показывает текущие данные
  2. DataForm   (Редактор)      — позволяет редактировать данные
  3. ResultForm (Результаты)    — показывает результат обработки

В каждый момент только одна форма видима, остальные — не существуют
или скрыты. Навигация между формами осуществляется через кнопки.

Общие данные (SharedData):
  - text   — текстовая строка (до 255 символов)
  - count  — числовой счётчик


СТРУКТУРА ПРОЕКТА
-----------------

                    App.exe
                   /       \
                  v         v
          Business.lib    Forms.lib
                  \         /
                   v       v
                 Shared (только .h)

Shared/          — интерфейсы и типы (header-only, без .cpp)
  IView.h           — IView, IHomeView, IDataView, IResultView
  IPresenter.h      — IPresenter (initialize/destroy)
  INavigator.h      — INavigator (navigateTo*, closeCurrentView)
  SharedData.h      — SharedData (inline-реализация)
  signal_impl.h     — библиотека сигналов/слотов (header-only, C++98)

Business/       — Business.lib (презентеры, зависит только от Shared)
  HomePresenter.h/cpp
  DataPresenter.h/cpp
  ResultPresenter.h/cpp

Forms/          — Forms.lib (VCL-формы, зависит только от Shared)
  HomeForm.h/cpp/.dfm
  DataForm.h/cpp/.dfm
  ResultForm.h/cpp/.dfm

App/            — App.exe (WinMain + навигатор, зависит от Business + Forms)
  AppMain.cpp       — точка входа (WinMain)
  AppNavigator.h/cpp — навигатор (create-on-demand)

Navigator.bpg  — группа проектов BCB6


ГРАФ ЗАВИСИМОСТЕЙ
-----------------

  App.exe --> Business.lib + Forms.lib + Shared
  Business.lib --> Shared (только)
  Forms.lib --> Shared (только)
  Shared --> (автономный, header-only)

Подробные зависимости заголовков:

  Shared:
    IView.h ------> signal_impl.h, SharedData.h
    IPresenter.h -> (автономный)
    INavigator.h -> SharedData.h
    SharedData.h -> (автономный)
    signal_impl.h -> (автономный)

  Business:
    HomePresenter.h  --> IPresenter.h, IView.h, INavigator.h, SharedData.h
    DataPresenter.h  --> IPresenter.h, IView.h, INavigator.h, SharedData.h
    ResultPresenter.h --> IPresenter.h, IView.h, INavigator.h, SharedData.h

  Forms:
    HomeForm.h   --> IView.h
    DataForm.h   --> IView.h
    ResultForm.h --> IView.h

  App:
    AppNavigator.h --> INavigator.h, IView.h, IPresenter.h, SharedData.h
    AppMain.cpp    --> AppNavigator.h


ЖИЗНЕННЫЙ ЦИКЛ ФОРМ (CREATE-ON-DEMAND)
--------------------------------------

Главная форма (HomeForm):
  - Создаётся при первом вызове navigateToHome() через
    Application->CreateForm(), что автоматически назначает её
    Application->MainForm
  - Живёт всё время работы приложения
  - При уходе с экрана — скрывается (hideView), не уничтожается
  - Закрытие MainForm через X или «Выход» --> Application->Terminate()

Остальные формы (DataForm, ResultForm):
  - Создаются через new при каждой навигации (navigateToData / navigateToResult)
  - При уходе с экрана — отложенно освобождаются (releaseView -> TForm::Release)
  - TForm::Release() вызывает PostMessage(CM_RELEASE), что гарантирует
    безопасное удаление формы после завершения текущей обработки сигналов


ЖИЗНЕННЫЙ ЦИКЛ MVP-ТРИАДЫ
--------------------------
1. Навигатор создаёт новую форму (new) и новый Presenter
2. presenter->initialize() — подписка на сигналы вида, displayData()
3. view->showView() — показать форму
4. ...пользователь взаимодействует с формой...
5. presenter->destroy() — отписка от сигналов вида
6. delete presenter — освобождение памяти
7. view->releaseView() или hideView() — скрыть/освободить форму

ВАЖНО: после вызова методов навигатора из обработчика сигнала,
презентер может быть удалён. Поэтому в обработчиках НЕ должно быть
кода после вызова навигатора. Это безопасно для void-методов,
которые просто возвращаются.


СИГНАЛЬНО-СЛОТОВЫЕ СОЕДИНЕНИЯ
------------------------------

Форма ---> Презентер (подписка в initialize(), отписка в destroy()):
  HomeForm.onGoData        ---> HomePresenter::handleGoData
  HomeForm.onGoResult      ---> HomePresenter::handleGoResult
  HomeForm.onExit          ---> HomePresenter::handleExit
  HomeForm.viewClosed      ---> HomePresenter::handleViewClosed

  DataForm.onGoHome        ---> DataPresenter::handleGoHome
  DataForm.onGoResult      ---> DataPresenter::handleGoResult
  DataForm.onDataSubmitted ---> DataPresenter::handleDataSubmitted
  DataForm.viewClosed      ---> DataPresenter::handleViewClosed

  ResultForm.onGoHome      ---> ResultPresenter::handleGoHome
  ResultForm.onGoData      ---> ResultPresenter::handleGoData
  ResultForm.viewClosed    ---> ResultPresenter::handleViewClosed

Презентер ---> Навигатор (прямые вызовы):
  HomePresenter::handleGoData()       ---> navigator->navigateToData(m_data)
  HomePresenter::handleGoResult()     ---> navigator->navigateToResult(m_data)
  HomePresenter::handleExit()         ---> navigator->closeCurrentView(m_data)
  HomePresenter::handleViewClosed()   ---> navigator->closeCurrentView(m_data)

  DataPresenter::handleGoHome()       ---> navigator->navigateToHome(m_data)
  DataPresenter::handleGoResult()     ---> navigator->navigateToResult(m_data)
  DataPresenter::handleViewClosed()   ---> navigator->closeCurrentView(m_data)

  ResultPresenter::handleGoHome()     ---> navigator->navigateToHome(m_data)
  ResultPresenter::handleGoData()     ---> navigator->navigateToData(m_data)
  ResultPresenter::handleViewClosed() ---> navigator->closeCurrentView(m_data)

Навигатор ---> Презентер (управление жизненным циклом):
  presenter->initialize()  — подписка на сигналы + displayData()
  presenter->destroy()     — отписка от сигналов


ПРАВИЛА MVP В ЭТОМ ПРОЕКТЕ
---------------------------
1. Формы НЕ обращаются к модели/сервисам напрямую.
2. Формы НЕ знают друг о друге.
3. Каждый презентер знает только свой вид и навигатор.
4. Навигатор — единственный, кто создаёт MVP-триады и управляет
   переключением экранов.
5. Данные передаются через параметры навигации.
6. Кнопка X на DataForm/ResultForm ---> перейти на HomeForm.
7. Кнопка X на HomeForm ---> завершить приложение.
8. Кнопка «Выход» на HomeForm ---> завершить приложение.


БЕЗОПАСНОСТЬ ПРИ DELETE THIS
-----------------------------
При навигации из обработчика сигнала:
  1. Пользователь нажимает кнопку на форме
  2. VCL вызывает обработчик -> emit_()
  3. Презентер вызывает navigator->navigateToXxx()
  4. Навигатор: create new triad -> activate()
  5. activate(): destroy() + delete oldPresenter
  6. Если oldPresenter — это текущий обработчик, то после delete
     метод просто возвращается (void, нет кода после вызова навигатора)

Для форм: вместо delete формы вызывается releaseView() -> TForm::Release(),
что откладывает удаление до завершения обработки сообщений.
Это предотвращает удаление формы во время emit её собственного сигнала.


КАК СОБРАТЬ
-----------
Откройте Navigator.bpg в Borland C++ Builder 6.0 IDE.
Группа проектов содержит три подпроекта:

  1. Business.lib  (презентеры)
  2. Forms.lib     (VCL-формы)
  3. App.exe       (исполняемый файл)

Порядок сборки: Business.lib -> Forms.lib -> App.exe

Нажмите F9 (Run) для сборки и запуска.

Примечание: компоненты на формах описаны в .dfm-файлах и загружаются
стандартным механизмом VCL (#pragma resource "*.dfm").
Визуальный дизайнер форм в IDE отображает корректный макет.


ОТЛИЧИЯ ОТ ОРИГИНАЛЬНОГО ПРОЕКТА (FormNavigator.bpr)
-----------------------------------------------------
1. Один проект (.bpr) разбит на группу проектов (.bpg):
     Shared (header-only) + Business.lib + Forms.lib + App.exe
2. Формы создаются по требованию (create-on-demand), а не живут вечно.
   DataForm и ResultForm создаются при навигации и освобождаются при уходе.
3. HomeForm создаётся через Application->CreateForm() и назначается MainForm.
4. Добавлен метод releaseView() в IView для безопасного удаления форм
   через TForm::Release() (отложенное удаление).
5. Удалены глобальные переменные форм (extern PACKAGE THomeForm *HomeForm).
6. Удалены USEFORM-макросы из WinMain.
7. signal_impl — header-only (было .h + .cpp).
8. SharedData — header-only (было .h + .cpp).
