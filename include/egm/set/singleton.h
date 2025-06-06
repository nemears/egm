#pragma once

#include <memory>
#include "egm/manager/abstractManager.h"
#include "abstractSet.h"
#include "doNothingPolicy.h"
#include "privateSet.h"
#include "egm/managedPtr.h"

namespace EGM {
    template <class T>
    class SingletonDataPolicy : virtual public AbstractSet {
        protected:
            ManagedPtr<T> m_data;
            class iterator : public AbstractSet::iterator {
                friend class SingletonDataPolicy;
                private:
                    std::weak_ptr<SetStructure> m_me;
                    bool m_done = true;
                    std::unique_ptr<AbstractSet::iterator> clone() const override {
                        return std::make_unique<iterator>(*this);
                    }
                    AbstractElementPtr getCurr() const override {
                        if (m_done) {
                            return ManagedPtr<T>();
                        }
                        if (m_me.lock()->m_set.rootSet()) {
                            auto& me = dynamic_cast<SingletonDataPolicy&>(m_me.lock()->m_set);
                            if (me.m_data) {
                                return me.m_data;
                            }
                        } else {
                            return m_me.lock()->m_rootRedefinedSet->m_set.beginPtr()->getCurr();
                        }

                        return (*m_me.lock()->m_subSetsWithData.begin())->m_set.beginPtr()->getCurr();
                    }
                    void next() override {
                        m_done = true;
                        m_hash = 0;
                    }
                public:
                    iterator() {};
                    iterator(const iterator& rhs) : AbstractSet::iterator(rhs) {
                        m_me = rhs.m_me.lock();
                        m_done = rhs.m_done;
                    }
                    T& operator*() {
                        return *getCurr();
                    }
                    ManagedPtr<T> operator->() {
                        return getCurr();
                    }
                    iterator operator++() {
                        next();
                        return *this;
                    }
                    iterator operator++(int) {
                        return (*this)++;
                    }
            };
        protected:
            void allocatePtr(AbstractElementPtr ptr, __attribute__((unused)) SetStructure& set) override {
                if (m_data && m_data.id() != ptr.id()) {
                    innerRemove(m_data);
                }
                auto val = get();
                if (val.id() == ptr.id()) {
                    m_structure->m_size--;
                }
            }
            bool hasData() const {
                return m_data.has();
            }
            bool containsData(ManagedPtr<T> ptr) const {
                return ptr.id() == m_data.id(); // TODO rn we are allowing null
            }
            void addData(ManagedPtr<T> ptr) {
                
                m_data = ptr;
            }
            bool removeData(ManagedPtr<T> ptr) {
                if (m_data.id() != ptr.id()) {
                    throw SetStateException("Tried to remove element that was not in singleton");
                }
                m_data = ManagedPtr<T>();
                return true;
            }
        public:
            ManagedPtr<T> get() const {
                if (m_structure->m_rootRedefinedSet.get() != m_structure.get()) {
                    return m_structure->m_rootRedefinedSet->m_set.beginPtr()->getCurr();
                }
                if (m_data) {
                    return m_data;
                }
                for (auto subSetWithData : m_structure->m_subSetsWithData) {
                    auto end = subSetWithData->m_set.endPtr();
                    auto begin = subSetWithData->m_set.beginPtr();
                    if (*begin != *end) {
                        return ManagedPtr<T>(subSetWithData->m_set.beginPtr()->getCurr());
                    }
                }
                return ManagedPtr<T>();
            }
            std::unique_ptr<AbstractSet::iterator> beginPtr() const override {
                return std::unique_ptr<AbstractSet::iterator>(new iterator(begin()));
            }
            std::unique_ptr<AbstractSet::iterator> endPtr() const override {
                return std::unique_ptr<AbstractSet::iterator>(new iterator(end()));
            }
            iterator begin() const {
                auto val = get();
                if (val) {
                    iterator ret;
                    std::hash<ID> hasher;
                    ret.m_hash = hasher(val.id());
                    ret.m_me = m_structure;
                    ret.m_done = false;
                    return ret;
                }
                return end();
            }
            iterator end() const {
                iterator ret;
                ret.m_me = m_structure;
                return iterator();
            }
            SetType setType() const override {
                return SetType::SINGLETON;
            }
    };

    template <template <class> class T, class U, class ApiPolicy = DoNothingPolicy>
    using ReadOnlySingleton = PrivateSet<T, U, SingletonDataPolicy<T<typename U::manager::template GenBaseHierarchy<T>>>, ApiPolicy>;

    template <template <class> class T, class U, class ApiPolicy = DoNothingPolicy>
    class Singleton : public ReadOnlySingleton<T,U,ApiPolicy> , public AbstractReadableSet {
        public: 
            using ManagedType = typename ReadOnlySingleton<T,U,ApiPolicy>::ManagedType;
        private:
            void checkCurrentValueHelper() {
                AbstractSet& redefinedSet = this->m_structure->m_rootRedefinedSet->m_set;
                AbstractElementPtr currVal = this->get();
                if (currVal) {
                    redefinedSet.innerRemove(currVal);
                }                
            }
            void setHelper(AbstractElementPtr ptr) {
                checkCurrentValueHelper();
                if (ptr) {
                    this->innerAdd(ptr);
                }
            }
        public:
            Singleton(U* me) : ReadOnlySingleton<T, U, ApiPolicy>(me) {}
            void set(ManagedPtr<ManagedType> ptr) {
                this->setHelper(ptr);
            }
            template <class Type>
            void set(Type& ref) {
                AbstractElementPtr ptr(&ref);
                this->setHelper(ptr);
            }
            void set(ID& id) {
                if (id == ID::nullID()) {
                    set(0);
                    return;
                }
                ManagedPtr<ManagedType> ptr = this->m_el.m_manager.createPtr(id); 
                if (ptr) {
                    this->m_structure->m_rootRedefinedSet->m_set.nonPolicyAdd(ptr);
                }
            }
            void add(ID id) {
                set(id);
            }
    };
}
