============================================================
FormNavigator — Демонстрация MVP + Навигатор + Сигналы/Слоты + VCL GUI
Borland C++ Builder 6.0 / C++98
============================================================

НАЗНАЧЕНИЕ
----------
Приложение демонстрирует навигацию между тремя VCL-формами
с использованием паттерна MVP (Model-View-Presenter) и
библиотеки сигналов/слотов (signal_impl.h/cpp).

Архитектура основана на паттерне «Навигатор»:
  - Каждый экран имеет свой собственный Presenter
  - Навигатор управляет сборкой MVP-триад и жизненным циклом
  - Подписка/отписка на события через Initialize()/Destroy()
  - Корректное завершение при закрытии последней формы

Три формы:
  1. HomeForm   (Главная)       — показывает текущие данные
  2. DataForm   (Редактор)      — позволяет редактировать данные
  3. ResultForm (Результаты)    — показывает результат обработки

В каждый момент только одна форма видима, две другие скрыты.
Навигация между формами осуществляется через кнопки на каждой форме.

Общие данные (SharedData):
  • text   — текстовая строка (до 255 символов)
  • count  — числовой счётчик

ГРАФ ЗАВИСИМОСТЕЙ
-----------------

                    FormNavigator.cpp
                    /     |       \
                   v      v        v
              IView.h   IPresenter.h   INavigator.h
                |          |              |
                v          v              v
          SharedDatah      +--- HomeForm.h
                           +--- DataForm.h
                           +--- ResultForm.h
                           +--- HomePresenter.h
                           +--- DataPresenter.h
                           +--- ResultPresenter.h
                           +--- AppNavigator.h

  IView.h ------> signal_impl.h
  IPresenter.h -> IView.h, INavigator.h, NavigatorTypes.h
  INavigator.h -> NavigatorTypes.h
  HomeForm.h ---> IView.h
  DataForm.h ---> IView.h
  ResultForm.h -> IView.h
  HomePresenter.h  --> IView.h, IPresenter.h, INavigator.h
  DataPresenter.h  --> IView.h, IPresenter.h, INavigator.h
  ResultPresenter.h --> IView.h, IPresenter.h, INavigator.h
  AppNavigator.h --> INavigator.h, IView.h, IPresenter.h


ФАЙЛЫ ПРОЕКТА
-------------
signal_impl.h/cpp        — Библиотека сигналов/слотов (C++98, реентерабельная)
SharedData.h/cpp         — Общие типы (SharedData)
IPresenter.h             — Базовый интерфейс презентера (initialize/destroy)
INavigator.h             — Интерфейс навигатора
IView.h                  — Абстрактные интерфейсы видов + viewClosedSignal
HomeForm.h/cpp/.dfm      — Форма 1: Главная (TForm + IHomeView)
DataForm.h/cpp/.dfm      — Форма 2: Редактор данных (TForm + IDataView)
ResultForm.h/cpp/.dfm    — Форма 3: Результаты (TForm + IResultView)
HomePresenter.h/cpp      — Презентер главной формы
DataPresenter.h/cpp      — Презентер формы редактирования
ResultPresenter.h/cpp    — Презентер формы результатов
AppNavigator.h/cpp       — Навигатор (управление MVP-триадами)
FormNavigator.cpp        — Точка входа (WinMain)
FormNavigator.bpr        — Файл проекта BCB6


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


ЖИЗНЕННЫЙ ЦИКЛ MVP-ТРИАДЫ
--------------------------
1. Навигатор создаёт новый Presenter для целевого вида
2. presenter->initialize() — подписка на сигналы вида, displayData()
3. view->showView() — показать форму
4. ...пользователь взаимодействует с формой...
5. presenter->destroy() — отписка от сигналов вида
6. delete presenter — освобождение памяти
7. view->hideView() — скрыть форму

ВАЖНО: после вызова методов навигатора из обработчика сигнала,
презентер может быть удалён. Поэтому в обработчиках НЕ должно быть
кода после вызова навигатора. Это безопасно для void-методов,
которые просто возвращаются.


КАК СОБРАТЬ
-----------
Откройте FormNavigator.bpr в Borland C++ Builder 6.0 IDE
и нажмите F9 (Run).

Примечание: компоненты на формах описаны в .dfm-файлах и загружаются
стандартным механизмом VCL (#pragma resource "*.dfm").
Визуальный дизайнер форм в IDE отображает корректный макет.
