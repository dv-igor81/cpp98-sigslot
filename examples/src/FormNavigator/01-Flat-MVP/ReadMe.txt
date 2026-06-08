============================================================
FormNavigator — Демонстрация MVP + Сигналы/Слоты + VCL GUI
Borland C++ Builder 6.0 / C++98
============================================================

НАЗНАЧЕНИЕ
----------
Приложение демонстрирует навигацию между тремя VCL-формами
с использованием паттерна MVP (Model-View-Presenter) и
библиотеки сигналов/слотов (signal_impl.h/cpp).

Три формы:
  1. HomeForm   (Главная)       — показывает текущие данные
  2. DataForm   (Редактор)      — позволяет редактировать данные
  3. ResultForm (Результаты)    — показывает результат обработки

В каждый момент только одна форма видима, две другие скрыты.
Навигация между формами осуществляется через кнопки на каждой форме.

Общие данные (SharedData):
  • text   — текстовая строка (до 255 символов)
  • count  — числовой счётчик

Поток данных:
  DataForm > «Применить» > onDataSubmitted > Presenter > Model >
  onDataChanged > Presenter > updateData() для всех форм


ГРАФ ЗАВИСИМОСТЕЙ
-----------------

                   FormNavigator.cpp
                    /     |       \
                   v      v        v
              IView.h   IModel.h   Presenter.h
                |          |           |
                v          v           v
          NavigatorTypes.h     +--- HomeForm.h
                               +--- DataForm.h
                               +--- ResultForm.h
                               +--- AppModel.h

  IView.h ------> signal_impl.h
  IModel.h -----> signal_impl.h
  Presenter.h --> IView.h, IModel.h
  HomeForm.h ---> IView.h
  DataForm.h ---> IView.h
  ResultForm.h -> IView.h
  AppModel.h ---> IModel.h


ФАЙЛЫ ПРОЕКТА
-------------
signal_impl.h/cpp   — Библиотека сигналов/слотов (C++98, реентерабельная)
NavigatorTypes.h/cpp — Общие типы (SharedData)
IView.h             — Абстрактные интерфейсы видов (IView, IHomeView, IDataView, IResultView)
IModel.h            — Абстрактный интерфейс модели + фабрика
HomeForm.h/cpp      — Форма 1: Главная (TForm + IView)
DataForm.h/cpp      — Форма 2: Редактор данных (TForm + IView)
ResultForm.h/cpp    — Форма 3: Результаты (TForm + IView)
AppModel.h/cpp      — Реализация модели
Presenter.h/cpp     — Презентер (связывает виды с моделью)
MainSource.cpp      — Точка входа (WinMain)
FormNavigator.bpr   — Файл проекта BCB6
FormNavigator.bpf   — Файл пакета BCB6


СИГНАЛЬНО-СЛОТОВЫЕ СОЕДИНЕНИЯ
------------------------------

Форма > Презентер:
  HomeForm.onGoData      > Presenter::handleGoData
  HomeForm.onGoResult    > Presenter::handleGoResult
  HomeForm.onExit        > Presenter::handleExit
  DataForm.onGoHome      > Presenter::handleGoHome
  DataForm.onGoResult    > Presenter::handleGoResult
  DataForm.onDataSubmitted > Presenter::handleDataSubmitted
  ResultForm.onGoHome    > Presenter::handleGoHome
  ResultForm.onGoData    > Presenter::handleGoData

Модель > Презентер:
  Model.onDataChanged    > Presenter::handleDataChanged

Презентер > Вид (прямые вызовы, не сигналы):
  view->showForm()       — показать форму
  view->hideForm()       — скрыть форму
  view->updateData()     — обновить данные на форме


ПРАВИЛА MVP В ЭТОМ ПРОЕКТЕ
---------------------------
1. Формы НЕ обращаются к модели напрямую.
2. Формы НЕ знают друг о друге.
3. Модель НЕ знает о формах.
4. Презентер — единственный, кто связывает виды с моделью.
5. Навигация — полностью в ведении презентера.
6. Кнопка X на DataForm/ResultForm > скрыть + перейти на HomeForm.
7. Кнопка X на HomeForm > закрыть приложение.


КАК СОБРАТЬ
-----------
Откройте FormNavigator.bpr в Borland C++ Builder 6.0 IDE
и нажмите F9 (Run).

Примечание: компоненты на формах создаются программно (без .dfm),
поэтому визуальный дизайнер форм в IDE будет пустым.
Все элементы управления инициализируются в конструкторах форм.
