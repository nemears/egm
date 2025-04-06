#pragma once

#include <memory>
#include "egm/manager/abstractElement.h"
#include "egm/manager/managerNode.h"
#include "egm/manager/abstractManager.h"

namespace std {
    template <class T>
    struct hash<EGM::ManagedPtr<T>> {
        std::size_t operator()(const EGM::ManagedPtr<T>& ptr) const;
    };
}

namespace EGM {

    struct NullPtrException : public std::exception {
        const char* what() const  throw() override {
            return "tried to access null ManagedPtr";
        };
    };

    template <
                template <class> class T,
                class DataTypePolicy,
                class ApiPolicy,
                class U
            >
    class PrivateSet;

    

    template <class T>
    class ManagedPtr : public AbstractPtr {

        template <template <class> class U, class ApiPolicy, class V> friend class Singleton;
        friend class AbstractAccessPolicy;
        template <class U> friend class ManagedPtr;
        template <
                template <class> class V,
                class DataTypePolicy,
                class ApiPolicy,
                class U
            >
        friend class PrivateSet;

        private:
            std::weak_ptr<T> getPtr() const {
                if (m_id == ID::nullID()) {
                    throw NullPtrException();
                } else if (m_ptr.lock()) {
                    return m_ptr;
                } else {
                    AbstractElementPtr temp = m_node.lock()->m_manager.abstractGet(m_id);
                    if (!m_ptr.lock()) {
                        const_cast<ManagedPtr<T>*>(this)->m_ptr = std::dynamic_pointer_cast<T>(temp->m_node.lock()->m_ptr);
                    }
                    const_cast<ManagedPtr<T>*>(this)->m_node = m_ptr.lock()->m_node.lock();
                    return  m_ptr;
                }
            }
            void setFromRaw(const T* rawPtr) {
                if (rawPtr) {
                    m_id = rawPtr->getID();
                    m_node = rawPtr->m_node;
                    m_ptr = std::dynamic_pointer_cast<T>(m_node.lock()->m_ptr);
                    m_node.lock()->addPtr(this);
                } 
            }
        protected:
            std::weak_ptr<T> m_ptr;

            void reindex(ID newID, AbstractElement* el) {
                m_id = newID;
                m_ptr = el;
                m_node = m_ptr.lock()->m_node;
            }
            void setPtr(std::shared_ptr<AbstractElement> ptr) override {
                m_ptr = std::dynamic_pointer_cast<T>(ptr);
            }
            template <class U = AbstractElement>
            void reassignPtr(const ManagedPtr<U>& rhs) {
                auto myPtr = m_ptr.lock();
                if (myPtr) {
                    myPtr->m_node.lock()->removePtr(this);
                }
                m_id = rhs.m_id;
                m_node = rhs.m_node;
                if (m_node.lock()) {
                    m_ptr = std::dynamic_pointer_cast<T>(rhs.m_ptr.lock());
                    m_node.lock()->addPtr(this);
                }
            }
        public:
            T& operator*() const {
                return *getPtr().lock();
            }
            std::shared_ptr<T> operator->() const {
                return getPtr().lock(); 
            }
            operator bool() const {
                return m_id != ID::nullID();
            }
            bool operator==(const AbstractElement* lhs) const {
                return m_id == lhs->m_id;
            }
            bool operator==(const AbstractElement& lhs) const {
                return m_id == lhs.m_id;
            }
            template <class OtherType>
            bool operator==(const ManagedPtr<OtherType>& lhs) const {
                return m_id == lhs.id();
            }
            void operator=(const T* el) {
                if (m_node.lock()) {
                    m_node.lock()->removePtr(this);
                }
                setFromRaw(el);
            }
            void operator=(const ManagedPtr& rhs) {
                reassignPtr(rhs);
            }
            ManagedPtr(const ManagedPtr& rhs) {
                reassignPtr(rhs);
            }
            template <class U>
            ManagedPtr(const ManagedPtr<U>& rhs) {
                reassignPtr(rhs);
            }
            template <class U>
            ManagedPtr(const ManagedPtr<U>* rhs) {
                reassignPtr(*rhs);
            }
            std::shared_ptr<T> ptr() {
                return getPtr().lock();
            }
            ID id() const {
                return m_id;
            }
            bool has() const {
                return m_id != ID::nullID();
            }
            bool loaded() const {
                if (m_id != ID::nullID()) {
                    return m_ptr.lock() != 0;
                }
                return false;
            }
            void release() {
                if (!m_ptr.lock()) {
                    return;
                }
                m_node.lock()->m_manager.release(*m_ptr.lock());
            }
            void aquire() {
                AbstractElementPtr temp = m_node.lock()->m_manager.abstractGet(m_id);
                if (!temp) {
                    throw ManagerStateException("Could not find element within manager memory or persistence");
                }
                m_ptr = std::dynamic_pointer_cast<T>(temp.ptr());
                m_node = m_ptr.lock()->m_node.lock();
            }
            ManagedPtr(T* el) {
                setFromRaw(el);
            }
            ManagedPtr() {
                // nothing
            }
            virtual ~ManagedPtr() {
                if (m_id == ID::nullID()) {
                    return;
                }
                if (m_node.lock()) {
                    m_node.lock()->removePtr(this);
                }
            }
    };
}

// hash operator injection
namespace std {
    template <class T> 
    std::size_t hash<EGM::ManagedPtr<T>>::operator()(const EGM::ManagedPtr<T>& ptr) const {

        /**
         * TODO better hashing
         **/

        hash<string> hasher;
        return hasher(ptr.id().string());
    }
}

namespace EGM {
    inline void AbstractSet::allocatePtr(__attribute__((unused)) AbstractElementPtr ptr, __attribute__((unused)) SetStructure& set) {}
    inline void AbstractSet::deAllocatePtr(__attribute__((unused)) AbstractElementPtr ptr) {} 
    inline void AbstractElement::addToReadonlySet(AbstractSet& set, AbstractElement& el) {
        set.innerAdd(&el);
    }
    inline void AbstractElement::removeFromReadonlySet(AbstractSet& set, AbstractElement& el) {
        set.innerRemove(&el);
    }
    inline void AbstractElement::setID(ID id)  {
        m_manager.reindex(m_id, id);
        m_id = id; 
    } 
}
