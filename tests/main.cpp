// Тесты системы сигналов и слотов. BCB6 / C++98.
// Сборка: g++ -std=c++98 -O0 -o signal_test main.cpp signal_impl.cpp

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "signal_impl.h"

#include "console_settings_keeper.h"

#include <stdio.h>
#include <string.h>

using namespace signals;

// --- Инфраструктура тестирования ---

static int g_testNum = 0;
static int g_passed  = 0;
static int g_failed  = 0;

#define CHECK(name, condition) do { \
    g_testNum++; \
    if (condition) { \
        printf("  [%2d] PASS: %s\n", g_testNum, name); \
        g_passed++; \
    } else { \
        printf("  [%2d] FAIL: %s\n", g_testNum, name); \
        g_failed++; \
    } \
} while(0)

// Глобальное состояние для колбэков
static int    g_counter    = 0;
static int    g_lastArg1   = 0;
static int    g_lastArg2   = 0;
static int    g_lastArg3   = 0;
static int    g_lastArg4   = 0;
static int    g_lastArg5   = 0;
static float  g_lastFloat  = 0.0f;
static double g_lastDouble = 0.0;
static char   g_lastChar   = 0;
static void*  g_lastPtr    = NULL;
static int    g_log[200];
static int    g_logCount   = 0;

void resetState() {
    g_counter = 0; g_lastArg1 = 0; g_lastArg2 = 0; g_lastArg3 = 0;
    g_lastArg4 = 0; g_lastArg5 = 0; g_lastFloat = 0.0f;
    g_lastDouble = 0.0; g_lastChar = 0; g_lastPtr = NULL; g_logCount = 0;
}

void logId(int id) {
    if (g_logCount < 200) { g_log[g_logCount] = id; g_logCount++; }
}

// Свободные функции-колбэки (id: cbA=1, cbB=2, cbC=3, cbD=4, cbE=5)
void cbA()  { g_counter++; logId(1); }
void cbB()  { g_counter++; logId(2); }
void cbC()  { g_counter++; logId(3); }
void cbD()  { g_counter++; logId(4); }
void cbE()  { g_counter++; logId(5); }

void cbArg1(int a1) { g_counter++; g_lastArg1 = a1; }
void cbArg2(int a1, int a2) { g_counter++; g_lastArg1 = a1; g_lastArg2 = a2; }
void cbArg3(int a1, int a2, int a3) { g_counter++; g_lastArg1 = a1; g_lastArg2 = a2; g_lastArg3 = a3; }
void cbArg4(int a1, float f, double d, char c)
    { g_counter++; g_lastArg1 = a1; g_lastFloat = f; g_lastDouble = d; g_lastChar = c; }
void cbArg5(int a, int b, int c, int d, int e)
    { g_counter++; g_lastArg1 = a; g_lastArg2 = b; g_lastArg3 = c; g_lastArg4 = d; g_lastArg5 = e; }
void cbPtr(void* p) { g_counter++; g_lastPtr = p; }
void cbDoubles(double a, double b) { g_counter++; g_lastDouble = a + b; }

// --- Вспомогательные классы ---

class Counter {
public:
    mutable int count;
    int id;
    Counter() : count(0), id(0) {}
    void inc() { count++; g_counter++; if (id) logId(id); }
    void constInc() const { count++; g_counter++; if (id) logId(id); }
};

class MultiMethod {
public:
    mutable int callA, callB;
    MultiMethod() : callA(0), callB(0) {}
    void methodA() { callA++; g_counter++; logId(10); }
    void methodB() { callB++; g_counter++; logId(11); }
    void constMethod() const { callA++; g_counter++; logId(12); }
};

// Отключает сам себя во время emit
class SelfRemover {
public:
    Signal0* sig;
    int callCount;
    SelfRemover() : sig(NULL), callCount(0) {}
    void remove() { callCount++; g_counter++; logId(20); sig->disconnect(this); }
};

// Отключает другой объект во время emit
class AnotherSlotRemover {
public:
    Signal0* sig;
    Counter* target;
    int callCount;
    AnotherSlotRemover() : sig(NULL), target(NULL), callCount(0) {}
    void removeOther() { callCount++; g_counter++; logId(21); if (sig && target) sig->disconnect(target); }
};

// Отключает свободную функцию во время emit
class FuncRemover {
public:
    Signal0* sig;
    int callCount;
    FuncRemover() : sig(NULL), callCount(0) {}
    void removeFunc() { callCount++; g_counter++; logId(22); if (sig) sig->disconnect(cbA); }
};

// Рекурсивный emit
class Recursor {
public:
    Signal0* sig;
    int depth, maxDepth;
    Recursor() : sig(NULL), depth(0), maxDepth(3) {}
    void recurse() { depth++; g_counter++; logId(30 + depth); if (depth < maxDepth) (*sig)(); }
};

// clear() во время emit
class ClearDuringEmit {
public:
    Signal0* sig;
    int callCount;
    ClearDuringEmit() : sig(NULL), callCount(0) {}
    void clearIt() { callCount++; g_counter++; logId(40); if (sig) sig->clear(); }
};

// delete this во время emit
class Suicide {
public:
    Signal0* sig;
    SlotMethodImpl<Suicide> m_dieSlot;
    int callCount;
    Suicide() : sig(NULL), m_dieSlot(this, &Suicide::die), callCount(0) {}
    void die() { callCount++; g_counter++; logId(50); delete this; }
};

// Стековый слот, подключённый к двум сигналам
class DualConnect {
public:
    SlotMethodImpl<DualConnect> m_slot;
    int callCount;
    DualConnect() : m_slot(this, &DualConnect::onSignal), callCount(0) {}
    void onSignal() { callCount++; g_counter++; logId(60); }
};

class ConstReceiver {
public:
    mutable int callCount;
    ConstReceiver() : callCount(0) {}
    void onSignal() const { callCount++; g_counter++; logId(70); }
    void onArg(int v) const { callCount++; g_counter++; g_lastArg1 = v; }
};

class ArgReceiver {
public:
    mutable int lastA, lastB, lastC;
    ArgReceiver() : lastA(0), lastB(0), lastC(0) {}
    void onTwo(int a, int b) { lastA = a; lastB = b; g_counter++; }
    void onThree(int a, int b, int c) { lastA = a; lastB = b; lastC = c; g_counter++; }
    void onTwoConst(int a, int b) const { lastA = a; lastB = b; g_counter++; }
};

// Подключает свободную функцию во время emit
class ConnectDuringEmit {
public:
    Signal0* sig;
    int callCount;
    ConnectDuringEmit() : sig(NULL), callCount(0) {}
    void addSlot() { callCount++; g_counter++; logId(80); if (sig) sig->connect(cbB); }
};

// Подключает метод объекта во время emit
class ConnectMethodDuringEmit {
public:
    Signal0* sig;
    Counter* target;
    int callCount;
    ConnectMethodDuringEmit() : sig(NULL), target(NULL), callCount(0) {}
    void addMethodSlot() { callCount++; g_counter++; logId(81); if (sig && target) sig->connect(target, &Counter::inc); }
};

// === Секция 1: Базовые операции Signal0 ===
void test_basic_signal0() {
    printf("\n=== Секция 1: Базовые операции Signal0 ===\n");
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig();
        CHECK("Свободная функция: connect + emit", g_counter == 1);
    }
    resetState();

    {
        Signal0 sig;
        sig();
        CHECK("Emit без подключений: нет краша", g_counter == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig.connect(cbB);
        sig();
        CHECK("Две свободные функции: обе вызваны", g_counter == 2);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig.connect(cbB);
        sig.connect(cbC);
        sig();
        CHECK("FIFO порядок: A->B->C",
              g_logCount == 3 && g_log[0] == 1 && g_log[1] == 2 && g_log[2] == 3);
    }
    resetState();

    {
        Counter obj;
        Signal0 sig;
        sig.connect(&obj, &Counter::inc);
        sig();
        CHECK("Метод объекта: connect + emit", obj.count == 1 && g_counter == 1);
    }
    resetState();

    {
        ConstReceiver obj;
        Signal0 sig;
        sig.connect(&obj, &ConstReceiver::onSignal);
        sig();
        CHECK("Const-метод: connect + emit", obj.callCount == 1 && g_counter == 1);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig();
        sig.emit_();
        CHECK("operator() == emit_(): вызвано 2 раза", g_counter == 2);
    }
    resetState();

    {
        Signal0 sig;
        SlotBase* p1 = sig.connect(cbA);
        Counter obj;
        SlotBase* p2 = sig.connect(&obj, &Counter::inc);
        CHECK("connect() возвращает не-NULL", p1 != NULL && p2 != NULL);
    }
}

// === Секция 2: Стековые билеты ===
void test_stack_slots() {
    printf("\n=== Секция 2: Стековые билеты ===\n");
    resetState();

    {
        Counter obj;
        SlotMethodImpl<Counter> slot(&obj, &Counter::inc);
        Signal0 sig;
        sig.connect(&slot);
        sig();
        CHECK("Стековый SlotMethodImpl: вызывается", obj.count == 1);
    }
    resetState();

    {
        ConstReceiver obj;
        SlotConstMethodImpl<ConstReceiver> slot(&obj, &ConstReceiver::onSignal);
        Signal0 sig;
        sig.connect(&slot);
        sig();
        CHECK("Стековый SlotConstMethodImpl: вызывается", obj.callCount == 1);
    }
    resetState();

    {
        Signal0 sig;
        {
            Counter obj;
            SlotMethodImpl<Counter> slot(&obj, &Counter::inc);
            sig.connect(&slot);
            sig();
        }
        sig();
        CHECK("Автоотписка: после scope exit слот не вызывается", g_counter == 1);
    }
    resetState();

    {
        Signal0 sig;
        {
            Counter obj;
            SlotMethodImpl<Counter> slot(&obj, &Counter::inc);
            sig.connect(&slot);
        }
        sig();
        sig();
        CHECK("После scope exit: повторный emit безопасен", g_counter == 0);
    }
    resetState();

    {
        Counter obj;
        SlotMethodImpl<Counter> slot(&obj, &Counter::inc);
        Signal0 sig;
        sig.connect(&slot);
        sig.connect(&slot);
        sig();
        CHECK("Стековый билет x2: вызывается дважды", obj.count == 2);
    }
    resetState();

    {
        ConstReceiver obj;
        SlotConstMethodImpl<ConstReceiver> slot(&obj, &ConstReceiver::onSignal);
        Signal0 sig;
        sig.connect(&slot);
        sig();
        CHECK("Стековый билет + const-метод: вызывается", obj.callCount == 1);
    }
    resetState();

    {
        DualConnect dc;
        Signal0 sig1, sig2;
        sig1.connect(&dc.m_slot);
        sig2.connect(&dc.m_slot);
        sig1();
        sig2();
        CHECK("Один билет, два сигнала: оба вызывают", dc.callCount == 2);
    }
    resetState();

    {
        Signal0 sig1, sig2;
        {
            DualConnect dc;
            sig1.connect(&dc.m_slot);
            sig2.connect(&dc.m_slot);
            sig1();
            sig2();
        }
        sig1();
        sig2();
        CHECK("Автоотписка от двух сигналов: оба безопасны", g_counter == 2);
    }
}

// === Секция 3: Операции disconnect ===
void test_disconnect() {
    printf("\n=== Секция 3: Операции disconnect ===\n");
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig.disconnect(cbA);
        sig();
        CHECK("disconnect(своб.функция): слот не вызывается", g_counter == 0);
    }
    resetState();

    {
        MultiMethod mm;
        Signal0 sig;
        sig.connect(&mm, &MultiMethod::methodA);
        sig.connect(&mm, &MultiMethod::methodB);
        sig.disconnect(static_cast<void*>(&mm));
        sig();
        CHECK("disconnect(obj): все методы удалены", mm.callA == 0 && mm.callB == 0);
    }
    resetState();

    {
        Counter obj;
        SlotMethodImpl<Counter> slot(&obj, &Counter::inc);
        Signal0 sig;
        sig.connect(&slot);
        sig.disconnect(&slot);
        sig();
        CHECK("disconnect(SlotBase*): слот удалён", obj.count == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig.disconnect(cbA);
        CHECK("disconnect несуществующей функции: нет краша", g_counter == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig.disconnect((void*)NULL);
        sig.disconnect((SlotBase*)NULL);
        CHECK("disconnect(NULL): нет краша", g_counter == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig.disconnect(cbA);
        sig.disconnect(cbA);
        CHECK("Повторный disconnect: нет краша", g_counter == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig.connect(cbB);
        sig.connect(cbC);
        sig.clear();
        sig();
        CHECK("clear(): все слоты удалены", g_counter == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig.clear();
        CHECK("clear() на пустом сигнале: нет краша", g_counter == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig.connect(cbA);
        sig.connect(cbA);
        sig.disconnect(cbA);
        sig();
        CHECK("Disconnect одного из дублей: осталось 2", g_counter == 2);
    }
    resetState();

    {
        MultiMethod mm;
        Signal0 sig;
        sig.connect(&mm, &MultiMethod::methodA);
        sig.connect(&mm, &MultiMethod::methodB);
        sig.disconnect(static_cast<void*>(&mm));
        sig();
        CHECK("disconnect(obj): оба метода удалены (count=0)",
              mm.callA == 0 && mm.callB == 0 && g_counter == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig.connect(cbB);
        sig.disconnect(cbA);
        sig();
        CHECK("После disconnect: оставшийся слот работает",
              g_counter == 1 && g_logCount == 1 && g_log[0] == 2);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig.disconnect(cbA);
        sig.connect(cbA);
        sig();
        CHECK("connect-disconnect-connect: работает", g_counter == 1);
    }
}

// === Секция 4: Операторы += и -= ===
void test_operators() {
    printf("\n=== Секция 4: Операторы += и -= ===\n");
    resetState();

    {
        Signal0 sig;
        sig += cbA;
        sig();
        CHECK("operator+= (своб.функция): вызывается", g_counter == 1);
    }
    resetState();

    {
        Signal0 sig;
        sig += cbA;
        sig -= cbA;
        sig();
        CHECK("operator-= (своб.функция): удалена", g_counter == 0);
    }
    resetState();

    {
        Counter obj;
        SlotMethodImpl<Counter> slot(&obj, &Counter::inc);
        Signal0 sig;
        sig += &slot;
        sig();
        CHECK("operator+= (стековый слот): вызывается", obj.count == 1);
    }
    resetState();

    {
        Counter obj;
        SlotMethodImpl<Counter> slot(&obj, &Counter::inc);
        Signal0 sig;
        sig += &slot;
        sig -= &slot;
        sig();
        CHECK("operator-= (стековый слот): удалён", obj.count == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig += cbA;
        sig += cbB;
        sig += cbC;
        sig -= cbB;
        sig();
        CHECK("Цепочка +=/-=: вызваны A и C (не B)",
              g_counter == 2 && g_log[0] == 1 && g_log[1] == 3);
    }
}

// === Секция 5: Signal1<int> ===
void test_signal1() {
    printf("\n=== Секция 5: Signal1<int> ===\n");
    resetState();

    {
        Signal1<int> sig;
        sig.connect(cbArg1);
        sig(42);
        CHECK("Signal1<int>: свободная функция с аргументом", g_lastArg1 == 42);
    }
    resetState();

    {
        ConstReceiver obj;
        Signal1<int> sig;
        sig.connect(&obj, &ConstReceiver::onArg);
        sig(77);
        CHECK("Signal1<int>: const-метод с аргументом",
              obj.callCount == 1 && g_lastArg1 == 77);
    }
    resetState();

    {
        Signal1<int> sig;
        sig.connect(cbArg1);
        sig.disconnect(cbArg1);
        sig(10);
        CHECK("Signal1: disconnect свободной функции", g_counter == 0);
    }
    resetState();

    {
        ConstReceiver obj;
        Signal1<int> sig;
        sig.connect(&obj, &ConstReceiver::onArg);
        sig.disconnect(static_cast<void*>(&obj));
        sig(10);
        CHECK("Signal1: disconnect по объекту", g_counter == 0);
    }
    resetState();

    {
        Signal1<int> sig;
        sig.connect(cbArg1);
        sig(123);
        CHECK("Signal1: operator()(123)", g_lastArg1 == 123);
    }
}

// === Секция 6: Signal2<int,int> ===
void test_signal2() {
    printf("\n=== Секция 6: Signal2<int,int> ===\n");
    resetState();

    {
        Signal2<int, int> sig;
        sig.connect(cbArg2);
        sig(10, 20);
        CHECK("Signal2<int,int>: своб.функция", g_lastArg1 == 10 && g_lastArg2 == 20);
    }
    resetState();

    {
        ArgReceiver obj;
        Signal2<int, int> sig;
        sig.connect(&obj, &ArgReceiver::onTwo);
        sig(5, 7);
        CHECK("Signal2<int,int>: метод объекта",
              obj.lastA == 5 && obj.lastB == 7);
    }
    resetState();

    {
        Signal2<int, int> sig;
        sig.connect(cbArg2);
        sig(100, 200);
        CHECK("Signal2: operator()(100, 200)", g_lastArg1 == 100 && g_lastArg2 == 200);
    }
    resetState();

    {
        Signal2<int, int> sig;
        sig.connect(cbArg2);
        sig.disconnect(cbArg2);
        sig(1, 2);
        CHECK("Signal2: disconnect своб.функции", g_counter == 0);
    }
    resetState();

    {
        Signal2<int, int> sig;
        sig.connect(cbArg2);
        sig.clear();
        sig(1, 2);
        CHECK("Signal2: clear() удаляет все слоты", g_counter == 0);
    }
}

// === Секция 7: Signal3–Signal5 и разные типы ===
void test_signal_multi_args() {
    printf("\n=== Секция 7: Signal3–Signal5 и разные типы ===\n");
    resetState();

    {
        Signal3<int, int, int> sig;
        sig.connect(cbArg3);
        sig(1, 2, 3);
        CHECK("Signal3<int,int,int>: emit",
              g_lastArg1 == 1 && g_lastArg2 == 2 && g_lastArg3 == 3);
    }
    resetState();

    {
        Signal4<int, float, double, char> sig;
        sig.connect(cbArg4);
        sig(42, 3.14f, 2.718, 'X');
        bool ok = (g_lastArg1 == 42) &&
                  (g_lastFloat > 3.13f && g_lastFloat < 3.15f) &&
                  (g_lastDouble > 2.717 && g_lastDouble < 2.719) &&
                  (g_lastChar == 'X');
        CHECK("Signal4<int,float,double,char>: смешанные типы", ok);
    }
    resetState();

    {
        Signal5<int, int, int, int, int> sig;
        sig.connect(cbArg5);
        sig(1, 2, 3, 4, 5);
        CHECK("Signal5: пять аргументов",
              g_lastArg1 == 1 && g_lastArg2 == 2 && g_lastArg3 == 3 &&
              g_lastArg4 == 4 && g_lastArg5 == 5);
    }
    resetState();

    {
        Signal1<void*> sig;
        sig.connect(cbPtr);
        int dummy = 0;
        void* p = &dummy;
        sig(p);
        CHECK("Signal1<void*>: аргумент-указатель", g_lastPtr == p);
    }
    resetState();

    {
        Signal2<double, double> sig;
        sig.connect(cbDoubles);
        sig(1.5, 2.5);
        bool ok = (g_lastDouble > 3.99 && g_lastDouble < 4.01);
        CHECK("Signal2<double,double>: 1.5+2.5=4.0", ok);
    }
}

// === Секция 8: Реентерабельность ===
void test_reentrancy() {
    printf("\n=== Секция 8: Реентерабельность ===\n");
    resetState();

    {
        Signal0 sig;
        SelfRemover sr;
        sr.sig = &sig;
        sig.connect(&sr, &SelfRemover::remove);
        sig();
        CHECK("Самоотключение во время emit: нет краша", sr.callCount == 1);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        SelfRemover sr;
        sr.sig = &sig;
        sig.connect(&sr, &SelfRemover::remove);
        sig.connect(cbC);
        sig();
        CHECK("Самоотключение одного из многих: остальные работают",
              g_counter == 3);
    }
    resetState();

    {
        Signal0 sig;
        Recursor rec;
        rec.sig = &sig;
        rec.maxDepth = 3;
        sig.connect(&rec, &Recursor::recurse);
        sig();
        CHECK("Рекурсивный emit (глубина 3): 3 вызова", g_counter == 3);
    }
    resetState();

    {
        Signal0 sig;
        Counter target;
        target.id = 100;
        AnotherSlotRemover remover;
        remover.sig = &sig;
        remover.target = &target;

        sig.connect(&target, &Counter::inc);
        sig.connect(&remover, &AnotherSlotRemover::removeOther);
        sig.connect(cbC);

        sig();
        CHECK("Отключение чужого слота во время emit: нет краша",
              target.count == 1 && remover.callCount == 1);
    }
    resetState();

    {
        Signal0 sig;
        Suicide* s = new Suicide();
        s->sig = &sig;
        sig.connect(&s->m_dieSlot);
        sig();
        sig();
        CHECK("Самоубийство (delete this): нет краша", g_counter == 1);
    }
    resetState();

    {
        Signal0 sig;
        Suicide* s = new Suicide();
        s->sig = &sig;
        sig.connect(&s->m_dieSlot);
        sig.connect(&s->m_dieSlot);
        sig();
        sig();
        CHECK("Самоубийство с двойным подключением: нет краша", g_counter == 1);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);

        ClearDuringEmit clearer;
        clearer.sig = &sig;
        sig.connect(&clearer, &ClearDuringEmit::clearIt);
        sig.connect(cbC);

        sig();
        CHECK("clear() во время emit: cbA+clearer вызваны, cbC нет",
              g_counter == 2);
    }
}

// === Секция 9: Сложные сценарии ===
void test_complex() {
    printf("\n=== Секция 9: Сложные сценарии ===\n");
    resetState();

    {
        Signal0 sig;
        Counter obj;
        for (int i = 0; i < 50; i++) {
            sig.connect(&obj, &Counter::inc);
        }
        sig();
        CHECK("Стресс: 50 подключений -> 50 вызовов", obj.count == 50);
    }
    resetState();

    {
        Signal0 sig;
        for (int i = 0; i < 10; i++) {
            sig.connect(cbA);
            sig();
            sig.disconnect(cbA);
        }
        CHECK("Цикл connect-emit-disconnect x10: 10 вызовов", g_counter == 10);
    }
    resetState();

    {
        Counter obj;
        SlotMethodImpl<Counter> slot(&obj, &Counter::inc);
        {
            Signal0 sig;
            sig.connect(&slot);
            sig();
        }
        CHECK("Уничтожение сигнала: стековый слот жив", obj.count == 1);
    }
    resetState();

    {
        Counter a, b;
        a.id = 1; b.id = 2;
        Signal0 sig;
        sig.connect(&a, &Counter::inc);
        sig.connect(&b, &Counter::inc);
        sig();
        CHECK("Два объекта на одном сигнале: оба вызваны",
              a.count == 1 && b.count == 1 && g_counter == 2);
    }
    resetState();

    {
        MultiMethod mm;
        Signal0 sig;
        sig.connect(&mm, &MultiMethod::methodA);
        sig.connect(&mm, &MultiMethod::methodB);
        sig();
        CHECK("Разные методы одного объекта: оба вызваны",
              mm.callA == 1 && mm.callB == 1);
    }
    resetState();

    {
        ArgReceiver obj;
        Signal2<int, int> sig;
        sig.connect(&obj, &ArgReceiver::onTwoConst);
        sig(10, 20);
        CHECK("Signal2: const-метод с двумя аргументами",
              obj.lastA == 10 && obj.lastB == 20);
    }
    resetState();

    {
        DualConnect dc;
        Signal0 sig1, sig2;
        sig1.connect(&dc.m_slot);
        sig2.connect(&dc.m_slot);

        sig1.disconnect(&dc.m_slot);
        sig1();
        sig2();
        CHECK("Disconnect из sig1 не влияет на sig2",
              dc.callCount == 1 && g_counter == 1);
    }
    resetState();

    {
        DualConnect dc;
        Signal0 sig1, sig2, sig3;
        sig1.connect(&dc.m_slot);
        sig2.connect(&dc.m_slot);
        sig3.connect(&dc.m_slot);
        sig1(); sig2(); sig3();
        CHECK("Один слот, три сигнала: вызван 3 раза", dc.callCount == 3);
    }
    resetState();

    {
        Counter a, b;
        Signal0 sig1, sig2;
        sig1.connect(&a, &Counter::inc);
        sig2.connect(&b, &Counter::inc);
        sig1();
        CHECK("Emit sig1 не вызывает sig2", a.count == 1 && b.count == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig();
        sig.disconnect(cbA);
        sig.connect(cbB);
        sig();
        CHECK("connect-disconnect-reconnect: cbA=1, cbB=1",
              g_counter == 2 && g_log[0] == 1 && g_log[1] == 2);
    }
}

// === Секция 10: Корректность refCount и память ===
void test_refcount_and_memory() {
    printf("\n=== Секция 10: Корректность refCount и память ===\n");
    resetState();

    {
        Signal0 sig;
        {
            Counter obj;
            SlotMethodImpl<Counter> slot(&obj, &Counter::inc);
            sig.connect(&slot);
            sig.connect(&slot);
            sig();
        }
        sig();
        CHECK("refCount x2: после scope exit оба узла очищены", g_counter == 2);
    }
    resetState();

    {
        Signal0 sig1, sig2;
        {
            DualConnect dc;
            sig1.connect(&dc.m_slot);
            sig2.connect(&dc.m_slot);
        }
        sig1();
        sig2();
        CHECK("Два сигнала + автоотписка: оба безопасны", g_counter == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig.disconnect(cbA);
        sig();
        CHECK("Heap-слот: connect-disconnect-cleaned", g_counter == 0);
    }
    resetState();

    {
        Counter obj;
        for (int i = 0; i < 100; i++) {
            Signal0 sig;
            sig.connect(&obj, &Counter::inc);
            sig();
        }
        CHECK("100 сигналов: нет краша", obj.count == 100);
    }
    resetState();

    {
        Signal0 sig;
        SelfRemover sr1, sr2;
        sr1.sig = &sig;
        sr2.sig = &sig;
        sig.connect(&sr1, &SelfRemover::remove);
        sig.connect(&sr2, &SelfRemover::remove);
        sig();
        sig();
        CHECK("Два SelfRemover: оба отключились",
              sr1.callCount == 1 && sr2.callCount == 1 && g_counter == 2);
    }
}

// === Секция 11: Краевые случаи ===
void test_edge_cases() {
    printf("\n=== Секция 11: Краевые случаи ===\n");
    resetState();

    {
        Signal6<int,int,int,int,int,int> sig6;
        Signal7<int,int,int,int,int,int,int> sig7;
        Signal8<int,int,int,int,int,int,int,int> sig8;
        g_testNum++; g_passed++;
        printf("  [%2d] PASS: %s\n", g_testNum, "Signal6/7/8: макросы генерируют классы");
    }
    resetState();

    {
        Signal0 sig;
        sig();
        sig.clear();
        sig();
        CHECK("Пустой сигнал: emit-clear-emit", g_counter == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig.connect(cbB);
        sig.connect(cbC);
        sig.disconnect(cbC);
        sig.disconnect(cbB);
        sig.disconnect(cbA);
        sig();
        CHECK("Disconnect в обратном порядке: все удалены", g_counter == 0);
    }
    resetState();

    {
        MultiMethod mm;
        Signal0 sig;
        sig.connect(&mm, &MultiMethod::methodA);
        sig.connect(&mm, &MultiMethod::methodB);
        sig.connect(&mm, &MultiMethod::constMethod);
        sig.disconnect(static_cast<void*>(&mm));
        sig();
        CHECK("3 метода + disconnect(obj): все удалены",
              mm.callA == 0 && mm.callB == 0 && g_counter == 0);
    }
    resetState();

    {
        ArgReceiver obj;
        Signal2<int, int> sig;
        sig.connect(&obj, &ArgReceiver::onTwoConst);
        sig(33, 44);
        CHECK("Signal2: const-метод вызван с правильными аргументами",
              obj.lastA == 33 && obj.lastB == 44);
    }
}

// === Секция 12: Сложная реентерабельность ===
void test_advanced_reentrancy() {
    printf("\n=== Секция 12: Сложная реентерабельность ===\n");
    resetState();

    {
        Signal0 sig;
        Recursor rec;
        rec.sig = &sig;
        rec.maxDepth = 5;
        sig.connect(&rec, &Recursor::recurse);
        sig();
        CHECK("Рекурсия глубины 5: 5 вызовов", g_counter == 5);
    }
    resetState();

    {
        Signal0 sig;
        Suicide* s = new Suicide();
        s->sig = &sig;
        sig.connect(cbA);
        sig.connect(&s->m_dieSlot);
        sig();
        sig();
        CHECK("Самоубийца + другой слот: другой слот жив", g_counter == 3);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        FuncRemover fr;
        fr.sig = &sig;
        sig.connect(&fr, &FuncRemover::removeFunc);
        sig.connect(cbC);
        sig();
        CHECK("Disconnect своб.функции во время emit: cbC вызвана", g_counter == 3);
    }
    resetState();

    {
        Signal0 sig;
        SelfRemover sr1, sr2, sr3;
        sr1.sig = &sig;
        sr2.sig = &sig;
        sr3.sig = &sig;
        sig.connect(&sr1, &SelfRemover::remove);
        sig.connect(&sr2, &SelfRemover::remove);
        sig.connect(&sr3, &SelfRemover::remove);
        sig();
        sig();
        CHECK("3 SelfRemover: все вызвались и отключились",
              sr1.callCount == 1 && sr2.callCount == 1 && sr3.callCount == 1);
    }
    resetState();

    {
        Signal0 sig;
        Suicide* s1 = new Suicide();
        Suicide* s2 = new Suicide();
        s1->sig = &sig;
        s2->sig = &sig;
        sig.connect(&s1->m_dieSlot);
        sig.connect(&s2->m_dieSlot);
        sig();
        sig();
        CHECK("Двойное suicide: оба удалены, нет краша", g_counter == 2);
    }
}

// === Секция 13: Операторы с многоаргументными сигналами ===
void test_operators_multiargs() {
    printf("\n=== Секция 13: Операторы с многоаргументными сигналами ===\n");
    resetState();

    {
        Signal2<int, int> sig;
        sig += cbArg2;
        sig(5, 10);
        CHECK("Signal2 operator+=: работает", g_lastArg1 == 5 && g_lastArg2 == 10);
    }
    resetState();

    {
        Signal2<int, int> sig;
        sig += cbArg2;
        sig -= cbArg2;
        sig(5, 10);
        CHECK("Signal2 operator-=: слот удалён", g_counter == 0);
    }
    resetState();

    {
        Signal1<int> sig;
        sig += cbArg1;
        sig(42);
        CHECK("Signal1 operator+=: работает", g_lastArg1 == 42);
    }
    resetState();

    {
        Signal1<int> sig;
        sig += cbArg1;
        sig -= cbArg1;
        sig(42);
        CHECK("Signal1 operator-=: слот удалён", g_counter == 0);
    }
    resetState();

    {
        ArgReceiver obj;
        SlotMethodImpl2<ArgReceiver, int, int> slot(&obj, &ArgReceiver::onTwo);
        Signal2<int, int> sig;
        sig += &slot;
        sig(7, 8);
        CHECK("Signal2 operator+= со стековым слотом",
              obj.lastA == 7 && obj.lastB == 8);
    }
}

// === Секция 14: Сложные сценарии с памятью ===
void test_memory_scenarios() {
    printf("\n=== Секция 14: Сложные сценарии с памятью ===\n");
    resetState();

    {
        Counter obj;
        SlotMethodImpl<Counter> stackSlot(&obj, &Counter::inc);
        Signal0 sig;
        sig.connect(&stackSlot);
        sig.connect(&obj, &Counter::inc);
        sig();
        CHECK("Стековый + heap слот: оба вызваны", obj.count == 2);
    }
    resetState();

    {
        Counter obj;
        SlotMethodImpl<Counter> stackSlot(&obj, &Counter::inc);
        Signal0 sig;
        sig.connect(&stackSlot);
        sig.connect(&obj, &Counter::inc);
        sig.disconnect(&stackSlot);
        sig();
        CHECK("Disconnect стекового: heap продолжает работать", obj.count == 1);
    }
    resetState();

    {
        Counter obj;
        SlotMethodImpl<Counter> stackSlot(&obj, &Counter::inc);
        Signal0 sig;
        sig.connect(&stackSlot);
        SlotBase* heapHandle = sig.connect(&obj, &Counter::inc);
        sig.disconnect(heapHandle);
        sig();
        CHECK("Disconnect heap по указателю: стековый продолжает работать",
              obj.count == 1);
    }
    resetState();

    {
        ArgReceiver obj;
        Signal2<int, int> sig;
        sig.connect(&obj, &ArgReceiver::onTwo);
        sig(1, 2);
        sig.disconnect(static_cast<void*>(&obj));
        sig(3, 4);
        CHECK("Signal2: disconnect по объекту после emit",
              obj.lastA == 1 && obj.lastB == 2);
    }
    resetState();

    {
        Signal0 sig;
        for (int i = 0; i < 50; i++) {
            sig.connect(cbA);
        }
        sig();
        int afterConnect = g_counter;
        sig.disconnect(cbA);
        sig();
        int afterSecondEmit = g_counter;
        sig.clear();
        sig();
        int afterClear = g_counter;
        CHECK("50 подключений -> disconnect -> clear: корректно",
              afterConnect == 50 && afterSecondEmit == 99 && afterClear == 99);
    }
}

// === Секция 15: Разные типы сигналов вместе ===
void test_mixed_signal_types() {
    printf("\n=== Секция 15: Разные типы сигналов вместе ===\n");
    resetState();

    {
        Counter obj0;
        ConstReceiver obj1;
        Signal0 s0;
        Signal1<int> s1;
        s0.connect(&obj0, &Counter::inc);
        s1.connect(&obj1, &ConstReceiver::onArg);
        s0();
        s1(42);
        CHECK("Signal0 + Signal1: независимы",
              obj0.count == 1 && obj1.callCount == 1 && g_lastArg1 == 42);
    }
    resetState();

    {
        ArgReceiver obj;
        Signal2<int, int> s2;
        Signal3<int, int, int> s3;
        s2.connect(&obj, &ArgReceiver::onTwo);
        s3.connect(&obj, &ArgReceiver::onThree);
        s2(1, 2);
        s3(10, 20, 30);
        CHECK("Signal2 + Signal3 на одном объекте",
              obj.lastA == 10 && obj.lastB == 20 && obj.lastC == 30);
    }
    resetState();

    {
        ConstReceiver obj;
        SlotConstMethodImpl1<ConstReceiver, int> slot(&obj, &ConstReceiver::onArg);
        Signal1<int> sig;
        sig.connect(&slot);
        sig(5);
        sig.disconnect(&slot);
        sig(10);
        CHECK("Signal1: стековый слот + disconnect",
              obj.callCount == 1 && g_lastArg1 == 5);
    }
    resetState();

    {
        MultiMethod mm;
        Signal0 sig;
        sig.connect(&mm, &MultiMethod::methodA);
        sig.connect(&mm, &MultiMethod::constMethod);
        sig();
        CHECK("const + не-const методы одного объекта",
              mm.callA == 2 && g_counter == 2);
    }
    resetState();

    {
        Counter obj1, obj2;
        Signal0 sig;
        sig.connect(&obj1, &Counter::inc);
        sig.disconnect(static_cast<void*>(&obj2));
        sig();
        CHECK("Disconnect не-подключённого объекта: sig работает",
              obj1.count == 1 && g_counter == 1);
    }
}

// === Секция 16: Финальные проверки ===
void test_final() {
    printf("\n=== Секция 16: Финальные проверки ===\n");
    resetState();

    {
        Counter obj;
        obj.id = 10;
        Signal0 sig;
        sig.connect(cbA);
        sig.connect(&obj, &Counter::inc);
        Counter obj2;
        obj2.id = 11;
        SlotMethodImpl<Counter> slot(&obj2, &Counter::inc);
        sig.connect(&slot);
        sig();
        CHECK("FIFO: своб.->heap->стековый",
              g_logCount == 3 && g_log[0] == 1 && g_log[1] == 10 && g_log[2] == 11);
    }
    resetState();

    {
        Counter obj;
        Signal0 sig;
        for (int i = 0; i < 10; i++) {
            sig.connect(&obj, &Counter::inc);
        }
        sig.disconnect(static_cast<void*>(&obj));
        sig();
        CHECK("10 подключений + disconnect(obj): все удалены", obj.count == 0);
    }
    resetState();

    {
        Signal0 sig;
        {
            Counter a; a.id = 0;
            SlotMethodImpl<Counter> slotA(&a, &Counter::inc);
            sig.connect(&slotA);
            {
                Counter b; b.id = 0;
                SlotMethodImpl<Counter> slotB(&b, &Counter::inc);
                sig.connect(&slotB);
                sig();
            }
            sig();
        }
        sig();
        CHECK("Вложенные scope: 2->1->0 вызовов", g_counter == 3);
    }
    resetState();

    {
        ConstReceiver obj;
        Signal1<int> sig;
        {
            SlotConstMethodImpl1<ConstReceiver, int> slot(&obj, &ConstReceiver::onArg);
            sig.connect(&slot);
            sig(10);
        }
        sig(20);
        CHECK("Signal1: автоотписка стекового слота",
              obj.callCount == 1 && g_lastArg1 == 10);
    }
    resetState();

    {
        Signal0 sig0;
        Counter obj; obj.id = 0;
        SlotMethodImpl<Counter> stackSlot(&obj, &Counter::inc);
        sig0.connect(cbA);
        sig0.connect(&obj, &Counter::inc);
        sig0.connect(&stackSlot);
        sig0();

        Signal2<int, int> sig2;
        ArgReceiver argObj;
        sig2.connect(cbArg2);
        sig2.connect(&argObj, &ArgReceiver::onTwo);
        sig2(1, 2);

        bool ok = (g_counter == 5) &&
                  (argObj.lastA == 1 && argObj.lastB == 2);

        sig0.clear();
        sig2.clear();
        sig0();
        sig2(0, 0);

        ok = ok && (g_counter == 5);
        CHECK("Grand Finale: все возможности вместе", ok);
    }
}

// === Секция 17: Подключение слота во время emit_ ===
void test_connect_during_emit() {
    printf("\n=== Секция 17: Подключение слота во время emit_ ===\n");
    resetState();

    {
        Signal0 sig;
        ConnectDuringEmit cde;
        cde.sig = &sig;
        sig.connect(cbA);
        sig.connect(&cde, &ConnectDuringEmit::addSlot);
        sig();
        CHECK("Новый слот (своб.функция) не вызван в текущем emit",
              g_counter == 2 && g_logCount == 2 && g_log[0] == 1 && g_log[1] == 80);
    }
    resetState();

    {
        Signal0 sig;
        ConnectDuringEmit cde;
        cde.sig = &sig;
        sig.connect(cbA);
        sig.connect(&cde, &ConnectDuringEmit::addSlot);
        sig();
        resetState();
        sig();
        CHECK("Новый слот вызван в следующем emit",
              g_counter == 3 && g_logCount == 3 && g_log[0] == 1 && g_log[1] == 80 && g_log[2] == 2);
    }
    resetState();

    {
        Signal0 sig;
        ConnectMethodDuringEmit cmde;
        Counter target;
        target.id = 90;
        cmde.sig = &sig;
        cmde.target = &target;
        sig.connect(cbA);
        sig.connect(&cmde, &ConnectMethodDuringEmit::addMethodSlot);
        sig();
        CHECK("Новый слот (метод) не вызван в текущем emit",
              g_counter == 2 && target.count == 0);
    }
    resetState();

    {
        Signal0 sig;
        ConnectMethodDuringEmit cmde;
        Counter target;
        target.id = 90;
        cmde.sig = &sig;
        cmde.target = &target;
        sig.connect(cbA);
        sig.connect(&cmde, &ConnectMethodDuringEmit::addMethodSlot);
        sig();
        resetState();
        sig();
        CHECK("Новый слот (метод) вызван в следующем emit",
              g_counter == 3 && target.count == 1);
    }
    resetState();

    {
        Signal0 sig;
        ConnectDuringEmit cde1, cde2;
        cde1.sig = &sig;
        cde2.sig = &sig;
        sig.connect(cbA);
        sig.connect(&cde1, &ConnectDuringEmit::addSlot);
        sig.connect(&cde2, &ConnectDuringEmit::addSlot);
        sig();
        CHECK("Два новых слота не вызваны в текущем emit", g_counter == 3);
    }
    resetState();

    {
        Signal0 sig;
        ConnectDuringEmit cde;
        cde.sig = &sig;
        sig.connect(cbA);
        sig.connect(&cde, &ConnectDuringEmit::addSlot);
        sig();
        CHECK("Рекурсия: новый слот не вызван", g_counter == 2);
    }
}

// --- Классы для тестирования виртуальных функций ---

class VirtBase {
public:
    mutable int callCount;
    VirtBase() : callCount(0) {}
    virtual void onEvent() = 0;  // чистый виртуальный
    virtual ~VirtBase() {}
};

class VirtDerived : public VirtBase {
public:
    void onEvent() { callCount++; g_counter++; logId(90); }
};

class VirtDerived2 : public VirtBase {
public:
    void onEvent() { callCount++; g_counter++; logId(91); }
};

// Виртуальный метод с аргументом
class VirtArgBase {
public:
    mutable int callCount;
    mutable int lastVal;
    VirtArgBase() : callCount(0), lastVal(0) {}
    virtual void onValue(int v) = 0;
    virtual ~VirtArgBase() {}
};

class VirtArgDerived : public VirtArgBase {
public:
    void onValue(int v) { callCount++; g_counter++; lastVal = v; }
};

// Два уровня наследования
class VirtMid : public VirtBase {
public:
    mutable int midCount;
    VirtMid() : midCount(0) {}
    void onEvent() { midCount++; callCount++; g_counter++; logId(92); }
};

class VirtLeaf : public VirtMid {
public:
    void onEvent() { midCount += 10; callCount++; g_counter++; logId(93); }
};

// === Секция 18: Виртуальные функции ===
void test_virtual_functions() {
    printf("\n=== Секция 18: Виртуальные функции ===\n");
    resetState();

    {
        VirtDerived obj;
        Signal0 sig;
        sig.connect(static_cast<VirtBase*>(&obj), &VirtBase::onEvent);
        sig();
        CHECK("Виртуальный вызов: Derived::onEvent", obj.callCount == 1 && g_counter == 1);
    }
    resetState();

    {
        VirtDerived obj;
        Signal0 sig;
        sig.connect(static_cast<VirtBase*>(&obj), &VirtBase::onEvent);
        sig();
        CHECK("Виртуальный: лог от Derived (id=90)",
              g_logCount == 1 && g_log[0] == 90);
    }
    resetState();

    {
        VirtDerived d;
        VirtDerived2 d2;
        Signal0 sig;
        sig.connect(static_cast<VirtBase*>(&d), &VirtBase::onEvent);
        sig.connect(static_cast<VirtBase*>(&d2), &VirtBase::onEvent);
        sig();
        CHECK("Два Derived: каждый вызвал свой onEvent",
              d.callCount == 1 && d2.callCount == 1 && g_counter == 2);
    }
    resetState();

    {
        VirtDerived obj;
        Signal0 sig;
        sig.connect(static_cast<VirtBase*>(&obj), &VirtBase::onEvent);
        sig.disconnect(static_cast<void*>(&obj));
        sig();
        CHECK("disconnect(obj) после виртуального connect", obj.callCount == 0 && g_counter == 0);
    }
    resetState();

    {
        VirtArgDerived obj;
        Signal1<int> sig;
        sig.connect(static_cast<VirtArgBase*>(&obj), &VirtArgBase::onValue);
        sig(42);
        CHECK("Виртуальный с аргументом: Derived::onValue(42)",
              obj.callCount == 1 && obj.lastVal == 42);
    }
    resetState();

    {
        VirtLeaf leaf;
        Signal0 sig;
        sig.connect(static_cast<VirtBase*>(&leaf), &VirtBase::onEvent);
        sig();
        CHECK("Два уровня наследования: Leaf::onEvent (id=93)",
              g_logCount == 1 && g_log[0] == 93 && leaf.callCount == 1);
    }
    resetState();

    {
        // Указатель на Base, объект — Leaf
        VirtBase* p = new VirtLeaf();
        Signal0 sig;
        sig.connect(p, &VirtBase::onEvent);
        sig();
        CHECK("Base* -> Leaf: виртуальная диспетчеризация", g_counter == 1);
        sig.disconnect(p);
        delete p;
        sig();
        CHECK("Base* -> Leaf: после delete + disconnect безопасно", g_counter == 1);
    }
    resetState();

    {
        // Стековый слот с виртуальным методом
        VirtDerived obj;
        SlotMethodImpl<VirtBase> slot(static_cast<VirtBase*>(&obj), &VirtBase::onEvent);
        Signal0 sig;
        sig.connect(&slot);
        sig();
        CHECK("Стековый слот + виртуальный метод: работает", obj.callCount == 1);
    }
}

// === Секция 19: slotCount() ===
void test_slot_count() {
    printf("\n=== Секция 19: slotCount() ===\n");
    resetState();

    {
        Signal0 sig;
        CHECK("slotCount: пустой сигнал == 0", sig.slotCount() == 0);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig.connect(cbB);
        sig.connect(cbC);
        CHECK("slotCount: 3 подключения == 3", sig.slotCount() == 3);
    }
    resetState();

    {
        Signal0 sig;
        Counter c1, c2;
        sig.connect(&c1, &Counter::inc);
        sig.connect(&c2, &Counter::inc);
        CHECK("slotCount: 2 метода == 2", sig.slotCount() == 2);
        sig.disconnect(&c1);
        CHECK("slotCount: после disconnect == 1", sig.slotCount() == 1);
    }
    resetState();

    {
        Signal0 sig;
        sig.connect(cbA);
        sig.connect(cbB);
        sig.clear();
        CHECK("slotCount: после clear == 0", sig.slotCount() == 0);
    }
    resetState();

    {
        Signal1<int> sig;
        sig.connect(cbArg1);
        CHECK("slotCount: Signal1<int> == 1", sig.slotCount() == 1);
    }
    resetState();
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    consoleKeeper.ApplyRussianSettings(SE_ANSI_1251);    

    printf("============================================================\n");
    printf("  ТЕСТЫ СИСТЕМЫ СИГНАЛОВ И СЛОТОВ\n");
    printf("  Среда: Borland C++ Builder 6.0 / C++98\n");
    printf("============================================================\n");

    test_basic_signal0();
    test_stack_slots();
    test_disconnect();
    test_operators();
    test_signal1();
    test_signal2();
    test_signal_multi_args();
    test_reentrancy();
    test_complex();
    test_refcount_and_memory();
    test_edge_cases();
    test_advanced_reentrancy();
    test_operators_multiargs();
    test_memory_scenarios();
    test_mixed_signal_types();
    test_final();
    test_connect_during_emit();
    test_virtual_functions();
    test_slot_count();

    printf("\n============================================================\n");
    printf("  ИТОГО: %d пройдено, %d провалено (из %d тестов)\n",
           g_passed, g_failed, g_testNum);
    printf("============================================================\n");

    return g_failed > 0 ? 1 : 0;
}
 
