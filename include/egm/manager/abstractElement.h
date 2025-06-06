#pragma once

#include <memory>
#include "egm/id.h"
#include "egm/set/abstractSet.h"
#include "abstractManager.h"

namespace EGM {

    class AbstractManager;
    struct ManagerNode;
    template <class T>
    class ManagedPtr;
    class AbstractSet;

    // non template class
    class AbstractElement {

        template <class T>
        friend class ManagedPtr;

        friend struct ManagerNode;

        template <template <class> class T, class U, class DataTypePolicy, class ApiPolicy>
        friend class PrivateSet;

        template <template <class> class T, class U, class ApiPolicy>
        friend class Singleton;

        template <template <class> class T, class U, class ApiPolicy>
        friend class Set;
        
        template <template <class> class T, class U, class ApiPolicy>
        friend class OrderedSet;

        template <class Tlist, class P1>
        friend class Manager;

        protected:
            const std::size_t m_elementType;
            AbstractManager& m_manager;
            std::weak_ptr<ManagerNode> m_node;
            ID m_id = ID::randomID();

            void addToReadonlySet(AbstractSet& set, AbstractElement& el);
            void removeFromReadonlySet(AbstractSet& set, AbstractElement& el);
            std::shared_ptr<ManagerNode> getNode(AbstractElement& el) {
                return el.m_node.lock();
            }

            AbstractElement(std::size_t elementType, AbstractManager& manager) : m_elementType(elementType), m_manager(manager) {}
        public:
            virtual ~AbstractElement() {}
            AbstractElement(const AbstractElement&) = delete;
            AbstractElement& operator=(const AbstractElement&) = delete;
            ID getID() const {
                return m_id;
            }
            void setID(ID id);
            inline friend bool operator==(const AbstractElement& lhs, const AbstractElement& rhs) {
                return lhs.m_id == rhs.m_id;
            };
            inline friend bool operator!=(const AbstractElement& lhs, const AbstractElement& rhs) {
                return lhs.m_id != rhs.m_id;
            };
            std::size_t getElementType() const {
                return m_elementType;
            }
    };
}
