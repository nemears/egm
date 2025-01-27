#pragma once

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include "abstractManager.h"
#include "abstractElement.h"
#include "egm/set/abstractSet.h"

namespace EGM {
    struct SetReference {
        std::weak_ptr<SetStructure> m_set;
        SetReference(std::shared_ptr<SetStructure> set) : m_set(set) {}
        bool operator==(const SetReference& rhs) const {
            return m_set.lock() == rhs.m_set.lock();
        }
    };
}

namespace std {
    template<>
    struct hash<EGM::SetReference> {
        std::size_t operator()(const EGM::SetReference& ref) const {
            return reinterpret_cast<std::size_t>(ref.m_set.lock().get());
        }
    };
}

namespace EGM {

    struct ManagerNode;

    class AbstractPtr {
        
        friend struct ManagerNode;

        template <class, class>
        friend class Manager;
        
        protected:
            std::weak_ptr<ManagerNode> m_node;
            ID m_id = ID::nullID();
            virtual void setPtr(std::shared_ptr<AbstractElement> ptr) = 0;
    };

    struct ManagerNode {
        // reference structs
        struct NodeReference {
            std::unordered_set<SetReference> m_sets;
            std::weak_ptr<ManagerNode> m_node;
            NodeReference(std::shared_ptr<ManagerNode> node) : m_node(node) {}
        };

        // members
        const ID m_id;
        std::shared_ptr<AbstractElement> m_ptr = 0;
        AbstractManager& m_manager;
        std::weak_ptr<ManagerNode> m_myPtr;
        std::unordered_set<AbstractPtr*> m_ptrs; // don't know how to not use raw ptr here
        std::unordered_map<ID, NodeReference> m_references;
        ManagerNode(AbstractElement* ptr) : m_id(ptr->m_id), m_ptr(ptr), m_manager(ptr->m_manager) {}
        ManagerNode(std::shared_ptr<AbstractElement> ptr) : m_id(ptr->m_id), m_ptr(ptr), m_manager(ptr->m_manager) {}
        ManagerNode(ID id, AbstractManager& manager) : m_id(id), m_manager(manager) {}
        ManagerNode(const ManagerNode& rhs) = delete;
        ~ManagerNode() {
            for (auto& pair : m_references) {
                auto& referenceReferences = pair.second.m_node.lock()->m_references;
                auto referenceMatch = referenceReferences.find(m_id);
                if (referenceMatch != referenceReferences.end()) {
                    referenceReferences.erase(referenceMatch);
                }
            }
        }
        void addPtr(AbstractPtr* ptr) {
            m_ptrs.insert(ptr);
        }
        void removePtr(AbstractPtr* ptr) {
            auto id = ptr->m_id;
            m_ptrs.erase(ptr);
            if (m_ptrs.empty() && !m_ptr) {
                m_manager.destroy(id);
            }
        }
        void setReference(std::shared_ptr<SetStructure> set, AbstractPtr& ptr) {
            auto referenceMatch = m_references.find(ptr.m_id);
            if (referenceMatch == m_references.end()) {
                auto& reference = setReference(ptr.m_node.lock());
                reference.m_sets.emplace(set);
            } else {
                referenceMatch->second.m_sets.emplace(set);
            }
        }
        NodeReference& setReference(std::shared_ptr<ManagerNode> node) {
            auto referenceMatch = m_references.find(node->m_id);
            if (referenceMatch == m_references.end()) {
                auto& reference = m_references.emplace(node->m_id, node).first->second;
                node->setReference(m_myPtr.lock());
                return reference;
            }
            return referenceMatch->second;
        }
        void removeReference(std::shared_ptr<SetStructure> set, AbstractPtr& ptr) {
            auto referenceMatch = m_references.find(ptr.m_id);
            referenceMatch->second.m_sets.erase(set);
            if (referenceMatch->second.m_sets.empty()) {
                // check to remove the other side
                auto& ptrReferences = ptr.m_node.lock()->m_references;
                auto ptrReferenceMatch = ptrReferences.find(m_id);
                if (ptrReferenceMatch != ptrReferences.end() && ptrReferenceMatch->second.m_sets.empty()) {
                    ptrReferences.erase(ptrReferenceMatch);
                    m_references.erase(referenceMatch);
                }
            }
        }
    };
}


