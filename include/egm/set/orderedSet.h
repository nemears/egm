#pragma once

#include "egm/set/abstractSet.h"
#include "egm/set/doNothingPolicy.h"
#include "egm/set/privateSet.h"
#include "egm/set/set.h"
#include "egm/managedPtr.h"
#include <memory>

namespace EGM {

    template <template <class> class T, class Manager>
    struct OrderedSetNode  {
        ManagedPtr<T<typename Manager::template GenBaseHierarchy<T>>> m_ptr;
        std::shared_ptr<OrderedSetNode> m_prev;
        std::shared_ptr<OrderedSetNode> m_next;
        OrderedSetNode(const OrderedSetNode& rhs) {
            m_prev = rhs.m_prev;
            m_next = rhs.m_next;
        }
        void destroy() {
            if (m_prev.get()) {
                m_prev->m_next.reset();
            }
            m_prev.reset();
            if (m_next.get()) {
                m_next->m_prev.reset();
            }
            m_next.reset();
        }
        template <class V>
        OrderedSetNode(const ManagedPtr<V>& ptr) {
            this->m_ptr = ptr;
        }
    };

    template <template <class> class T, class U>
    class OrderedSetDataPolicy : virtual public SetDataPolicy<T, typename U::manager> {
        protected:
            using ManagedType = T<typename U::manager::template GenBaseHierarchy<T>>;

            std::shared_ptr<OrderedSetNode<T, typename U::manager>> m_first;
            std::shared_ptr<OrderedSetNode<T, typename U::manager>> m_last;
            
            void allocatePtr(AbstractElementPtr ptr, SetStructure& set) override {
                SetDataPolicy<T, U>::allocatePtr(ptr, set);
                std::shared_ptr<OrderedSetNode<T, typename U::manager>> orderedPtr = std::make_shared<OrderedSetNode<T, typename U::manager>>(ptr);
                if (!m_first) {
                    m_first = orderedPtr;
                }
                orderedPtr->m_prev = m_last;
                if (m_last) {
                    m_last->m_next = orderedPtr;
                }
                m_last = orderedPtr;
            }
            void deAllocatePtr(AbstractElementPtr ptr) override {

                // TODO slow, would be quicker to not iterate over entire thing

                auto orderedPtr = m_first;
                while (orderedPtr && orderedPtr->m_ptr.id() != ptr.id()) {
                    orderedPtr = orderedPtr->m_next;
                }

                // the ptr is still in our set
                if (orderedPtr && orderedPtr->m_ptr.id() == ptr.id()) {
                    if (m_last == orderedPtr) {
                        m_last = orderedPtr->m_prev;
                    }
                    if (m_first == orderedPtr) {
                        m_first = orderedPtr->m_next;
                    }
                    if (orderedPtr->m_prev) {
                        orderedPtr->m_prev->m_next = orderedPtr->m_next;
                    }
                    if (orderedPtr->m_next) {
                        orderedPtr->m_next->m_prev = orderedPtr->m_prev;
                    }
                }
            }
            class iterator : public AbstractSet::iterator {
                friend class OrderedSetDataPolicy;
                const OrderedSetDataPolicy& m_set;
                std::shared_ptr<OrderedSetNode<T, typename U::manager>> m_curr;
                protected:
                    AbstractElementPtr getCurr() const override {
                        if (m_curr.get()) {
                            return m_curr->m_ptr;
                        }
                        return AbstractElementPtr();
                    }
                    void next() override {
                        if (m_curr) {
                            if (m_curr->m_ptr.id() == m_set.m_last->m_ptr.id()) {
                                m_curr = nullptr;
                            } else {
                                m_curr = m_curr->m_next;
                            }
                        }
                        if (m_curr.get()) {
                            std::hash<ID> hasher;
                            m_hash = hasher(m_curr->m_ptr.id());
                        } else {
                            m_hash = 0;
                        }
                    }
                    std::unique_ptr<AbstractSet::iterator> clone() const override {
                        return std::unique_ptr<iterator>(new iterator(*this));
                    }
                public:
                    iterator(const iterator& rhs) : AbstractSet::iterator(rhs) , m_set(rhs.m_set) {
                        m_curr = rhs.m_curr;
                    }
                    iterator(const OrderedSetDataPolicy& set) : m_set(set) {};
                    T<ManagedType>& operator*() {
                        return dynamic_cast<ManagedType&>(*getCurr());
                    }
                    ManagedPtr<ManagedType> operator->() {
                        return getCurr(); 
                    }
                    iterator operator++() {
                        next();
                        return *this;
                    }
                    iterator operator++(int) {
                        return ++(*this);
                    }        
            };
        public:
            ~OrderedSetDataPolicy() {
                auto curr = m_first;
                while (curr) {
                    auto oldCurr = curr;
                    curr = oldCurr->m_next;
                    oldCurr->m_prev = 0;
                    oldCurr->m_next = 0;
                }
                m_first = 0;
                m_last = 0;
            }
            std::unique_ptr<AbstractSet::iterator> beginPtr() const override {
                return std::make_unique<iterator>(begin());
            };
            std::unique_ptr<AbstractSet::iterator> endPtr() const override {
                return std::unique_ptr<iterator>(new iterator(end()));
            }
            ManagedPtr<ManagedType> front() const {
                if (m_first.get()) {
                    return m_first->m_ptr;
                }
                return ManagedPtr<ManagedType>();
            }
            ManagedPtr<ManagedType> back() const {
                if (m_last.get()) {
                    return m_last->m_ptr;
                }
                return ManagedPtr<ManagedType>();
            }
            iterator begin() const {
                iterator it(*this);
                it.m_curr = m_first;
                if (it.m_curr.get()) {
                    std::hash<ID> hasher;
                    it.m_hash = hasher(it.m_curr->m_ptr.id());
                }
                return it;
            }
            iterator end() const {
                return iterator(*this);
            }
            SetType setType() const override {
                return SetType::ORDERED_SET;
            }
            ManagedPtr<ManagedType> get(ID id) const {
                return SetDataPolicy<T, U>::get(id);
            }
            ManagedPtr<ManagedType> get(size_t index) const {
                size_t currIndex = 0;
                auto currPtr = m_first;
                while (currIndex != index && currPtr) {
                    currPtr = currPtr->m_next;
                    currIndex++;
                }
                if (currIndex != index) {
                    throw SetStateException("index larger than set size!");
                }
                return currPtr->m_ptr;
            }
    };

    template <template <class> class T, class U, class ApiPolicy = DoNothingPolicy>
    using ReadOnlyOrderedSet = PrivateSet<T, U, OrderedSetDataPolicy<T, U>, ApiPolicy>;
    
    template <template <class> class T, class U, class ApiPolicy = DoNothingPolicy>
    class OrderedSet : public ReadOnlyOrderedSet<T, U, ApiPolicy> , public AbstractReadableSet {

        using ManagedType = T<typename U::Manager::template GenBaseHierarchy<T>>;

        public:
            OrderedSet(U* me) : ReadOnlyOrderedSet<T, U, ApiPolicy>(me) {}
            void add(ManagedPtr<ManagedType> ptr) {
                this->innerAdd(ptr);
            }
            void add(ID id) override {
                this->m_structure->m_rootRedefinedSet->m_set.nonPolicyAdd(this->m_el.m_manager.createPtr(id));
            }
            void add(ManagedType& el) {
                this->innerAdd(ManagedPtr<ManagedType>(&el));
            }
            template <class ... Ts>
            void add(ManagedType& el, Ts& ... els) { 
                add(el);
                add(els...);
            }
            void remove(ManagedPtr<ManagedType> ptr) {
                this->innerRemove(ptr);
            }
            void remove(ID& id) {
                this->innerRemove(this->m_el.m_manager->createPtr(id));
            }
            void remove(ManagedType& el) {
                this->innerRemove(ManagedPtr<ManagedType>(&el));
            }
            void clear() {
                while (this->front()) {
                    remove(this->front());
                }
            }
    };
}
