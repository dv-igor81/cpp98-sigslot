// ============================================================================
// signal_impl.h -- Система сигналов и слотов для C++98 (BCB 6.0)
// ============================================================================
//
// Header-only: вся реализация inline для исключения .cpp-файла.
// Это позволяет поместить signal_impl в Shared (только .h).
//
// Принципы: реентерабельность (m_emitDepth), отложенное удаление (isDead),
// двусторонняя связь (слот<->сигнал), RAII (EmitGuard), типобезопасность,
// поддержка const-методов.
//
// ============================================================================

#ifndef SIGNAL_IMPL_H
#define SIGNAL_IMPL_H

#ifndef NULL
#define NULL 0
#endif

namespace signals {

class SignalBase;

// ============================================================================
// SlotBaseCore -- базовый класс любого слота.
// Поддерживает: счётчик ссылок (m_refCount), флаг владения (m_isOwnedBySignal),
// флаг разрушения (m_isDestructing), список подключённых сигналов (m_signalLinks).
// ============================================================================

class SlotBaseCore {

private:
    int m_refCount;           // Количество узлов, ссылающихся на слот
    bool m_isOwnedBySignal;   // Слот создан сигналом (delete при refCount==0)
    bool m_isDestructing;     // Деструктор выполняется

    SlotBaseCore(const SlotBaseCore&);
    SlotBaseCore& operator=(const SlotBaseCore&);

protected:
    struct SignalLink {
        SignalBase* signal;
        SignalLink* next;
        SignalLink(SignalBase* s) : signal(s), next(NULL) {}
    };

    SignalLink* m_signalLinks; // Список сигналов, к которым привязан слот

    virtual ~SlotBaseCore();   // protected -- внешний код не делает delete

public:
    SlotBaseCore()
        : m_refCount(0), m_isOwnedBySignal(false),
          m_isDestructing(false), m_signalLinks(NULL) {}

    virtual void* targetObject() const { return NULL; }
    virtual bool isFreeFunction() const { return false; }

private:
    void attachToSignal(SignalBase* sig);
    void detachFromSignal(SignalBase* sig);
    void disconnectFromAll();

    friend class SignalBase;
};


// ============================================================================
// SignalBase -- реентерабельная реализация с отложенным удалением.
// Управляет связным списком узлов-подписчиков, сборкой мусора (sweepDeadNodes).
// ============================================================================

class SignalBase {

protected:
    struct Node {
        SlotBaseCore* slot;
        Node* next;
        bool isDead;  // Надгробие -- логическое удаление
        Node(SlotBaseCore* s) : slot(s), next(NULL), isDead(false) {}
    };

    Node* m_head;
    Node* m_tail;       // Для O(1) вставки в конец
    int m_emitDepth;    // Глубина вложенных emit_

    SignalBase();
    ~SignalBase();

    SignalBase(const SignalBase&);
    SignalBase& operator=(const SignalBase&);

    // Удаляет мёртвые узлы физически (только при emitDepth==0)
    void sweepDeadNodes();

    // Добавляет слот без передачи владения
    SlotBaseCore* addSlot(SlotBaseCore* slot);

    // Добавляет слот с передачей владения (m_isOwnedBySignal=true)
    SlotBaseCore* addOwnedSlot(SlotBaseCore* slot) {
        if (!slot) return NULL;
        slot->m_isOwnedBySignal = true;
        return addSlot(slot);
    }

    // RAII: увеличивает m_emitDepth, при выходе -- sweepDeadNodes
    struct EmitGuard {
        int& depth;
        SignalBase* sig;
        EmitGuard(int& d, SignalBase* s) : depth(d), sig(s) { depth++; }
        ~EmitGuard() {
            depth--;
            if (depth == 0) sig->sweepDeadNodes();
        }
    };

public:
    void disconnect(void* obj);
    void disconnect(SlotBaseCore* slot);
    void clear();

    // Бухгалтерия для разрушающегося слота в disconnect(func)
    bool handleDestructingSlotDisconnect(Node* n);

    // Количество активных слотов (isDead==false, slot!=NULL)
    int slotCount() const;

    friend struct EmitGuard;
};


// ============================================================================
// SlotBase -- абстрактный базовый класс для слота void()
// ============================================================================

class SlotBase : public SlotBaseCore {
public:
    virtual void call() = 0;
};


// ============================================================================
// SlotFunctionPtr -- обёртка для свободной функции void()
// ============================================================================

class SlotFunctionPtr : public SlotBase {
    typedef void (*FuncPtr)();
    FuncPtr m_func;
public:
    SlotFunctionPtr(FuncPtr func) : m_func(func) {}
    void call() { if (m_func) m_func(); }
    bool isFreeFunction() const { return true; }
    bool isEqual(FuncPtr func) const { return m_func == func; }
};


// ============================================================================
// SlotMethodImpl -- обёртка для метода объекта (не const)
// ============================================================================

template<typename Receiver>
class SlotMethodImpl : public SlotBase {
    Receiver* m_obj;
    void (Receiver::*m_method)();
public:
    SlotMethodImpl(Receiver* o, void (Receiver::*m)()) : m_obj(o), m_method(m) {}
    void call() { if (m_obj) (m_obj->*m_method)(); }
    void* targetObject() const { return m_obj; }
};


// ============================================================================
// SlotConstMethodImpl -- обёртка для const-метода объекта
// ============================================================================

template<typename Receiver>
class SlotConstMethodImpl : public SlotBase {
    Receiver* m_obj;
    void (Receiver::*m_method)() const;
public:
    SlotConstMethodImpl(Receiver* o, void (Receiver::*m)() const) : m_obj(o), m_method(m) {}
    void call() { if (m_obj) (m_obj->*m_method)(); }
    void* targetObject() const { return m_obj; }
};


// ============================================================================
// Signal0 -- сигнал без аргументов
// ============================================================================

class Signal0 : private SignalBase {

public:
    Signal0() : SignalBase() {}

    SlotBase* connect(void(*func)()) {
        return static_cast<SlotBase*>(addOwnedSlot(new SlotFunctionPtr(func)));
    }

    SlotBase* connect(SlotBase* slot) {
        return static_cast<SlotBase*>(addSlot(slot));
    }

    SlotBase* connect(SlotBase& slot) {
        return connect(&slot);
    }

    template<typename Receiver>
    SlotBase* connect(Receiver* obj, void (Receiver::*method)()) {
        SlotMethodImpl<Receiver>* slot = new SlotMethodImpl<Receiver>(obj, method);
        return static_cast<SlotBase*>(addOwnedSlot(slot));
    }

    template<typename Receiver>
    SlotBase* connect(Receiver* obj, void (Receiver::*method)() const) {
        SlotConstMethodImpl<Receiver>* slot = new SlotConstMethodImpl<Receiver>(obj, method);
        return static_cast<SlotBase*>(addOwnedSlot(slot));
    }

    void disconnect(void* obj) { SignalBase::disconnect(obj); }

    void disconnect(SlotBase* slot) { SignalBase::disconnect(slot); }

    void disconnect(SlotBase& slot) { disconnect(&slot); }

    void disconnect(void(*func)()) {
        if (!func) return;

        Node* n = m_head;
        while (n) {
            if (!n->isDead && n->slot && n->slot->isFreeFunction()) {
                if (static_cast<SlotFunctionPtr*>(n->slot)->isEqual(func)) {
                    n->isDead = true;
                    if (handleDestructingSlotDisconnect(n)) {
                        // Слот разрушается -- продолжаем поиск
                    } else {
                        if (m_emitDepth == 0) sweepDeadNodes();
                        break;
                    }
                }
            }
            n = n->next;
        }
    }

    void operator+=(void(*func)()) { connect(func); }
    void operator-=(void(*func)()) { disconnect(func); }
    void operator+=(SlotBase* slot) { addSlot(slot); }
    void operator-=(SlotBase* slot) { disconnect(slot); }
    void operator+=(SlotBase& slot) { connect(slot); }
    void operator-=(SlotBase& slot) { disconnect(slot); }

    void clear() { SignalBase::clear(); }

    void emit_() {
        EmitGuard guard(m_emitDepth, this);
        Node* snapshot_tail = m_tail;

        Node* n = m_head;
        while (n) {
            if (!n->isDead && n->slot) {
                static_cast<SlotBase*>(n->slot)->call();
            }
            if (n == snapshot_tail) break;
            n = n->next;
        }
    }

    void operator()() { emit_(); }

    int slotCount() const { return SignalBase::slotCount(); }
};


// ============================================================================
// Макросы генерации Signal1--Signal8 (C++98 не имеет вариативных шаблонов)
// ============================================================================

#define SIGNAL_TYPES_1 typename T1
#define SIGNAL_PURE_1 T1
#define SIGNAL_ARGS_1 T1 a1
#define SIGNAL_FWD_1 a1

#define SIGNAL_TYPES_2 typename T1, typename T2
#define SIGNAL_PURE_2 T1, T2
#define SIGNAL_ARGS_2 T1 a1, T2 a2
#define SIGNAL_FWD_2 a1, a2

#define SIGNAL_TYPES_3 typename T1, typename T2, typename T3
#define SIGNAL_PURE_3 T1, T2, T3
#define SIGNAL_ARGS_3 T1 a1, T2 a2, T3 a3
#define SIGNAL_FWD_3 a1, a2, a3

#define SIGNAL_TYPES_4 typename T1, typename T2, typename T3, typename T4
#define SIGNAL_PURE_4 T1, T2, T3, T4
#define SIGNAL_ARGS_4 T1 a1, T2 a2, T3 a3, T4 a4
#define SIGNAL_FWD_4 a1, a2, a3, a4

#define SIGNAL_TYPES_5 typename T1, typename T2, typename T3, typename T4, typename T5
#define SIGNAL_PURE_5 T1, T2, T3, T4, T5
#define SIGNAL_ARGS_5 T1 a1, T2 a2, T3 a3, T4 a4, T5 a5
#define SIGNAL_FWD_5 a1, a2, a3, a4, a5

#define SIGNAL_TYPES_6 typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
#define SIGNAL_PURE_6 T1, T2, T3, T4, T5, T6
#define SIGNAL_ARGS_6 T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6
#define SIGNAL_FWD_6 a1, a2, a3, a4, a5, a6

#define SIGNAL_TYPES_7 typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
#define SIGNAL_PURE_7 T1, T2, T3, T4, T5, T6, T7
#define SIGNAL_ARGS_7 T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7
#define SIGNAL_FWD_7 a1, a2, a3, a4, a5, a6, a7

#define SIGNAL_TYPES_8 typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8
#define SIGNAL_PURE_8 T1, T2, T3, T4, T5, T6, T7, T8
#define SIGNAL_ARGS_8 T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7, T8 a8
#define SIGNAL_FWD_8 a1, a2, a3, a4, a5, a6, a7, a8


// ============================================================================
// DECLARE_SIGNAL_N -- генерирует SlotBaseN, SlotFunctionPtrN,
// SlotMethodImplN, SlotConstMethodImplN и SignalN для N аргументов.
// Новые слоты, подключённые во время emit_, не вызываются в текущем цикле
// (остановка по snapshot_tail).
// ============================================================================
#define DECLARE_SIGNAL_N(N) \
\
    template<SIGNAL_TYPES_##N> \
    class SlotBase##N : public SlotBaseCore { \
    public: virtual void call(SIGNAL_ARGS_##N) = 0; }; \
\
    template<SIGNAL_TYPES_##N> \
    class SlotFunctionPtr##N : public SlotBase##N<SIGNAL_PURE_##N> { \
        typedef void (*FuncPtr)(SIGNAL_ARGS_##N); FuncPtr m_func; \
    public: \
        SlotFunctionPtr##N(FuncPtr func) : m_func(func) {} \
        void call(SIGNAL_ARGS_##N) { if (m_func) m_func(SIGNAL_FWD_##N); } \
        bool isFreeFunction() const { return true; } \
        bool isEqual(FuncPtr func) const { return m_func == func; } }; \
\
    template<typename Receiver, SIGNAL_TYPES_##N> \
    class SlotMethodImpl##N : public SlotBase##N<SIGNAL_PURE_##N> { \
        Receiver* m_obj; void (Receiver::*m_method)(SIGNAL_ARGS_##N); \
    public: \
        SlotMethodImpl##N(Receiver* o, void (Receiver::*m)(SIGNAL_ARGS_##N)) : m_obj(o), m_method(m) {} \
        void call(SIGNAL_ARGS_##N) { if (m_obj) (m_obj->*m_method)(SIGNAL_FWD_##N); } \
        void* targetObject() const { return m_obj; } }; \
\
    template<typename Receiver, SIGNAL_TYPES_##N> \
    class SlotConstMethodImpl##N : public SlotBase##N<SIGNAL_PURE_##N> { \
        Receiver* m_obj; void (Receiver::*m_method)(SIGNAL_ARGS_##N) const; \
    public: \
        SlotConstMethodImpl##N(Receiver* o, void (Receiver::*m)(SIGNAL_ARGS_##N) const) : m_obj(o), m_method(m) {} \
        void call(SIGNAL_ARGS_##N) { if (m_obj) (m_obj->*m_method)(SIGNAL_FWD_##N); } \
        void* targetObject() const { return m_obj; } }; \
\
    template<SIGNAL_TYPES_##N> \
    class Signal##N : private SignalBase { \
    public: \
        Signal##N() : SignalBase() {} \
        SlotBase##N<SIGNAL_PURE_##N>* connect(void(*func)(SIGNAL_ARGS_##N)) { return static_cast<SlotBase##N<SIGNAL_PURE_##N>*>(addOwnedSlot(new SlotFunctionPtr##N<SIGNAL_PURE_##N>(func))); } \
        SlotBase##N<SIGNAL_PURE_##N>* connect(SlotBase##N<SIGNAL_PURE_##N>* slot) { return static_cast<SlotBase##N<SIGNAL_PURE_##N>*>(addSlot(slot)); } \
        SlotBase##N<SIGNAL_PURE_##N>* connect(SlotBase##N<SIGNAL_PURE_##N>& slot) { return connect(&slot); } \
        template<typename Receiver> \
        SlotBase##N<SIGNAL_PURE_##N>* connect(Receiver* obj, void (Receiver::*method)(SIGNAL_ARGS_##N)) { \
            SlotMethodImpl##N<Receiver, SIGNAL_PURE_##N>* slot = new SlotMethodImpl##N<Receiver, SIGNAL_PURE_##N>(obj, method); \
            return static_cast<SlotBase##N<SIGNAL_PURE_##N>*>(addOwnedSlot(slot)); \
        } \
        template<typename Receiver> \
        SlotBase##N<SIGNAL_PURE_##N>* connect(Receiver* obj, void (Receiver::*method)(SIGNAL_ARGS_##N) const) { \
            SlotConstMethodImpl##N<Receiver, SIGNAL_PURE_##N>* slot = new SlotConstMethodImpl##N<Receiver, SIGNAL_PURE_##N>(obj, method); \
            return static_cast<SlotBase##N<SIGNAL_PURE_##N>*>(addOwnedSlot(slot)); \
        } \
        void disconnect(void* obj) { SignalBase::disconnect(obj); } \
        void disconnect(SlotBase##N<SIGNAL_PURE_##N>* slot) { SignalBase::disconnect(slot); } \
        void disconnect(SlotBase##N<SIGNAL_PURE_##N>& slot) { disconnect(&slot); } \
        void disconnect(void(*func)(SIGNAL_ARGS_##N)) { \
            if (!func) return; \
            Node* n = m_head; while (n) { \
                if (!n->isDead && n->slot && n->slot->isFreeFunction()) { \
                    if (static_cast<SlotFunctionPtr##N<SIGNAL_PURE_##N>*>(n->slot)->isEqual(func)) { \
                        n->isDead = true; \
                        if (handleDestructingSlotDisconnect(n)) { \
                        } else { \
                            if (m_emitDepth == 0) sweepDeadNodes(); \
                            break; \
                        } \
                    } \
                } \
                n = n->next; \
            } \
        } \
        void operator+=(void(*func)(SIGNAL_ARGS_##N)) { connect(func); } \
        void operator-=(void(*func)(SIGNAL_ARGS_##N)) { disconnect(func); } \
        void operator+=(SlotBase##N<SIGNAL_PURE_##N>* slot) { addSlot(slot); } \
        void operator-=(SlotBase##N<SIGNAL_PURE_##N>* slot) { disconnect(slot); } \
        void operator+=(SlotBase##N<SIGNAL_PURE_##N>& slot) { connect(slot); } \
        void operator-=(SlotBase##N<SIGNAL_PURE_##N>& slot) { disconnect(slot); } \
        void clear() { SignalBase::clear(); } \
        int slotCount() const { return SignalBase::slotCount(); } \
        void emit_(SIGNAL_ARGS_##N) { \
            EmitGuard guard(m_emitDepth, this); \
            Node* snapshot_tail = m_tail; \
            Node* n = m_head; while (n) { \
                if (!n->isDead && n->slot) { \
                    static_cast<SlotBase##N<SIGNAL_PURE_##N>*>(n->slot)->call(SIGNAL_FWD_##N); \
                } \
                if (n == snapshot_tail) break; \
                n = n->next; \
            } \
        } \
        void operator()(SIGNAL_ARGS_##N) { emit_(SIGNAL_FWD_##N); } \
    };

#ifdef __BORLANDC__
#pragma warn -8027  // функции с while не могут быть inline (ожидаемо для макросов)
#endif
DECLARE_SIGNAL_N(1)
DECLARE_SIGNAL_N(2)
DECLARE_SIGNAL_N(3)
DECLARE_SIGNAL_N(4)
DECLARE_SIGNAL_N(5)
DECLARE_SIGNAL_N(6)
DECLARE_SIGNAL_N(7)
DECLARE_SIGNAL_N(8)
#ifdef __BORLANDC__
#pragma warn .8027  // восстановить
#endif

// Очистка макросов
#undef DECLARE_SIGNAL_N
#undef SIGNAL_TYPES_1
#undef SIGNAL_PURE_1
#undef SIGNAL_ARGS_1
#undef SIGNAL_FWD_1
#undef SIGNAL_TYPES_2
#undef SIGNAL_PURE_2
#undef SIGNAL_ARGS_2
#undef SIGNAL_FWD_2
#undef SIGNAL_TYPES_3
#undef SIGNAL_PURE_3
#undef SIGNAL_ARGS_3
#undef SIGNAL_FWD_3
#undef SIGNAL_TYPES_4
#undef SIGNAL_PURE_4
#undef SIGNAL_ARGS_4
#undef SIGNAL_FWD_4
#undef SIGNAL_TYPES_5
#undef SIGNAL_PURE_5
#undef SIGNAL_ARGS_5
#undef SIGNAL_FWD_5
#undef SIGNAL_TYPES_6
#undef SIGNAL_PURE_6
#undef SIGNAL_ARGS_6
#undef SIGNAL_FWD_6
#undef SIGNAL_TYPES_7
#undef SIGNAL_PURE_7
#undef SIGNAL_ARGS_7
#undef SIGNAL_FWD_7
#undef SIGNAL_TYPES_8
#undef SIGNAL_PURE_8
#undef SIGNAL_ARGS_8
#undef SIGNAL_FWD_8


// ============================================================================
// Inline-реализация SlotBaseCore (после полного определения SignalBase)
// ============================================================================

inline SlotBaseCore::~SlotBaseCore()
{
    if (m_signalLinks) {
        m_isDestructing = true;
        disconnectFromAll();
    }
}

inline void SlotBaseCore::attachToSignal(SignalBase* sig)
{
    SignalLink* link = new SignalLink(sig);
    link->next = m_signalLinks;
    m_signalLinks = link;
}

inline void SlotBaseCore::detachFromSignal(SignalBase* sig)
{
    SignalLink* prev = NULL;
    SignalLink* n = m_signalLinks;
    while (n) {
        SignalLink* next = n->next;
        if (n->signal == sig) {
            if (prev) prev->next = next; else m_signalLinks = next;
            delete n;
            break;
        }
        prev = n;
        n = next;
    }
}

inline void SlotBaseCore::disconnectFromAll()
{
    SignalLink* current = m_signalLinks;
    m_signalLinks = NULL;

    while (current) {
        SignalBase* sig = current->signal;
        SignalLink* next = current->next;
        delete current;
        sig->disconnect(this);
        current = next;
    }
}


// ============================================================================
// Inline-реализация SignalBase
// ============================================================================

inline SignalBase::SignalBase() : m_head(NULL), m_tail(NULL), m_emitDepth(0) {}

inline SignalBase::~SignalBase()
{
    if (m_emitDepth != 0) {
        // FATAL: удаление сигнала во время emit
        // В header-only сборке assert может быть недоступен
        // (заголовок <cassert> не включён намеренно для совместимости)
        return;
    }
    clear();
}

inline SlotBaseCore* SignalBase::addSlot(SlotBaseCore* slot)
{
    if (!slot) return NULL;

    slot->m_refCount++;
    Node* node = new Node(slot);

    if (!m_head) {
        m_head = node;
        m_tail = node;
    } else {
        m_tail->next = node;
        m_tail = node;
    }

    slot->attachToSignal(this);
    return slot;
}

inline void SignalBase::sweepDeadNodes()
{
    Node* prev = NULL;
    Node* n = m_head;

    while (n) {
        if (n->isDead) {
            Node* next = n->next;
            if (prev) prev->next = next; else m_head = next;
            if (n == m_tail) m_tail = prev;

            SlotBaseCore* s = n->slot;
            delete n;

            if (s) {
                s->m_refCount--;
                s->detachFromSignal(this);
                if (s->m_refCount == 0 && s->m_isOwnedBySignal) {
                    delete s;
                }
            }
            n = next;
        } else {
            prev = n;
            n = n->next;
        }
    }
}

inline void SignalBase::disconnect(SlotBaseCore* targetSlot)
{
    if (!targetSlot) return;

    Node* n = m_head;
    while (n) {
        if (!n->isDead && n->slot == targetSlot) {
            n->isDead = true;
            if (targetSlot->m_isDestructing) {
                targetSlot->m_refCount--;
                targetSlot->detachFromSignal(this);
                n->slot = NULL;
            } else {
                break;
            }
        }
        n = n->next;
    }

    if (m_emitDepth == 0) sweepDeadNodes();
}

inline void SignalBase::disconnect(void* obj)
{
    if (!obj) return;

    Node* n = m_head;
    while (n) {
        if (!n->isDead && n->slot && n->slot->targetObject() == obj) {
            n->isDead = true;
            if (n->slot->m_isDestructing) {
                n->slot->m_refCount--;
                n->slot->detachFromSignal(this);
                n->slot = NULL;
            }
        }
        n = n->next;
    }

    if (m_emitDepth == 0) sweepDeadNodes();
}

inline bool SignalBase::handleDestructingSlotDisconnect(Node* n)
{
    if (n->slot->m_isDestructing) {
        n->slot->m_refCount--;
        n->slot->detachFromSignal(this);
        n->slot = NULL;
        return true;
    }
    return false;
}

inline int SignalBase::slotCount() const
{
    int count = 0;
    Node* n = m_head;
    while (n) {
        if (!n->isDead && n->slot) count++;
        n = n->next;
    }
    return count;
}

inline void SignalBase::clear()
{
    if (m_emitDepth > 0) {
        Node* n = m_head;
        while (n) {
            if (!n->isDead) {
                n->isDead = true;
                if (n->slot && n->slot->m_isDestructing) {
                    n->slot->m_refCount--;
                    n->slot->detachFromSignal(this);
                    n->slot = NULL;
                }
            }
            n = n->next;
        }
    } else {
        Node* n = m_head;
        while (n) {
            Node* next = n->next;
            SlotBaseCore* s = n->slot;
            delete n;
            if (s) {
                s->m_refCount--;
                s->detachFromSignal(this);
                if (s->m_refCount == 0 && s->m_isOwnedBySignal) {
                    delete s;
                }
            }
            n = next;
        }
        m_head = NULL;
        m_tail = NULL;
    }
}

} // namespace signals

#endif // SIGNAL_IMPL_H
