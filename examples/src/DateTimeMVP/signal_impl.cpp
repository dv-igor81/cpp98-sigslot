// Реализация signal_impl.h. C++98, namespace signals.
#include "signal_impl.h"

#include <cassert>

namespace signals {

// --- SlotBaseCore ---

SlotBaseCore::~SlotBaseCore()
{
    if (m_signalLinks) {
        m_isDestructing = true;
        disconnectFromAll();
    }
}

void SlotBaseCore::attachToSignal(SignalBase* sig)
{
    SignalLink* link = new SignalLink(sig);
    link->next = m_signalLinks;
    m_signalLinks = link;
}

void SlotBaseCore::detachFromSignal(SignalBase* sig)
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

void SlotBaseCore::disconnectFromAll()
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

// --- SignalBase ---

SignalBase::SignalBase() : m_head(NULL), m_tail(NULL), m_emitDepth(0) {}

SignalBase::~SignalBase()
{
    if (m_emitDepth != 0) {
        assert(m_emitDepth == 0 && "FATAL: Deleting signal while it is emitting!");
        return;
    }
    clear();
}

SlotBaseCore* SignalBase::addSlot(SlotBaseCore* slot)
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

void SignalBase::sweepDeadNodes()
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
                assert(s->m_refCount >= 0 && "m_refCount went negative");
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

void SignalBase::disconnect(SlotBaseCore* targetSlot)
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

void SignalBase::disconnect(void* obj)
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

bool SignalBase::handleDestructingSlotDisconnect(Node* n)
{
    if (n->slot->m_isDestructing) {
        n->slot->m_refCount--;
        n->slot->detachFromSignal(this);
        n->slot = NULL;
        return true;
    }
    return false;
}

int SignalBase::slotCount() const
{
    int count = 0;
    Node* n = m_head;
    while (n) {
        if (!n->isDead && n->slot) count++;
        n = n->next;
    }
    return count;
}

void SignalBase::clear()
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

// --- Signal0 ---

Signal0::Signal0() : SignalBase() {}

SlotBase* Signal0::connect(void(*func)())
{
    return static_cast<SlotBase*>(addOwnedSlot(new SlotFunctionPtr(func)));
}

SlotBase* Signal0::connect(SlotBase* slot)
{
    return static_cast<SlotBase*>(addSlot(slot));
}

SlotBase* Signal0::connect(SlotBase& slot)
{
    return connect(&slot);
}

void Signal0::disconnect(void* obj) { SignalBase::disconnect(obj); }

void Signal0::disconnect(SlotBase* slot) { SignalBase::disconnect(slot); }

void Signal0::disconnect(SlotBase& slot) { disconnect(&slot); }

void Signal0::disconnect(void(*func)())
{
    if (!func) return;

    Node* n = m_head;
    while (n) {
        if (!n->isDead && n->slot && n->slot->isFreeFunction()) {
            if (static_cast<SlotFunctionPtr*>(n->slot)->isEqual(func)) {
                n->isDead = true;
                if (handleDestructingSlotDisconnect(n)) {
                    // Слот разрушается — продолжаем поиск
                } else {
                    if (m_emitDepth == 0) sweepDeadNodes();
                    break;
                }
            }
        }
        n = n->next;
    }
}

void Signal0::clear() { SignalBase::clear(); }

void Signal0::emit_() {
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

void Signal0::operator()() { emit_(); }

} // namespace signals
