#pragma once

#include <list>
#include <memory>
#include <mutex>
#include "abstractSet.h"
#include "wrapperSet.h"
#include "doNothingPolicy.h"
#include "egm/managedPtr.h"

namespace YAML {
    class Node;
}

namespace EGM {
    class SetStateException : public std::exception {
        std::string m_msg;
        public:
            SetStateException(){};
            SetStateException(std::string msg) : m_msg("Error in Uml Set: " + msg) {};
            const char* what() const throw() override {
                return m_msg.c_str();
            };
    };
    template <class T>
    class OppositeInterface {
        public:
            virtual ~OppositeInterface() = default;
            virtual bool enabled() = 0;
            virtual void addOpposite(T& el) = 0;
            virtual void removeOpposite(T& el) = 0;
            bool skip = false; // unused rn
    };

    template <class T>
    class NoOpposite : public OppositeInterface<T> {
        public:
            bool enabled() override {
                return false;
            }
            void addOpposite(__attribute__((unused)) T& el) override {}
            void removeOpposite(__attribute__((unused)) T& el) override {}
    };

    template <template <class> class T,  class U, class DataTypePolicy, class ApiPolicy>
    class PrivateSet : virtual public AbstractSet , virtual public DataTypePolicy, virtual public ApiPolicy {

        public:
            using ManagedType = T<typename U::manager::template GenBaseHierarchy<T>>;
        protected:
            friend ManagedType;
            friend U;
            friend ApiPolicy;
            template <template <class> class V, class W, class OtherDataTypePolicy, class OtherApiPolicy>
            friend class PrivateSet;

        private:
            std::mutex m_mutex;
        protected:
            U& m_el;
            std::unique_ptr<OppositeInterface<ManagedType>> m_opposite = std::unique_ptr<NoOpposite<ManagedType>>(new NoOpposite<ManagedType>());
   
            void runAddPolicy(AbstractElement& el) override {
                // check if manager is in state to run add policies
                if (!m_el.m_manager.policiesEnabled())
                    return;

                ApiPolicy::elementAdded(dynamic_cast<ManagedType&>(el), m_el);
            }
            void runRemovePolicy(AbstractElement& el) override {
                ApiPolicy::elementRemoved(dynamic_cast<ManagedType&>(el), m_el);
            }
            bool oppositeEnabled() const override {
                return m_opposite->enabled();
            }
            void oppositeAdd(AbstractElement& el) override {
                m_opposite->addOpposite(dynamic_cast<ManagedType&>(el));
            }
            void oppositeRemove(AbstractElement& el) override {
                m_opposite->removeOpposite(dynamic_cast<ManagedType&>(el));
            }
            void addToOpposite(AbstractElementPtr ptr) override {
                std::list<std::shared_ptr<SetStructure>> queue;
                std::unordered_set<std::shared_ptr<SetStructure>> visited;
                queue.push_back(this->m_structure->m_rootRedefinedSet);
                while (!queue.empty()) {
                    auto front = queue.front();
                    queue.pop_front();
                    if (visited.count(front)) {
                        continue;
                    }
                    visited.insert(front);
                    bool oppositeRan = false;
                    if (ptr.loaded()) {
                        if (!oppositeRan && front->m_set.oppositeEnabled()) {
                            front->m_set.oppositeAdd(*ptr);
                            oppositeRan = true;
                        }
                    }
                    for (std::shared_ptr<SetStructure> redefinedSet : front->m_redefinedSets) {
                        if (ptr.loaded()) {
                            if (!oppositeRan && redefinedSet->m_set.oppositeEnabled()) {
                                redefinedSet->m_set.oppositeAdd(*ptr);
                                oppositeRan = true;
                            }
                        }
                    }
                    if (!oppositeRan) {
                        for (std::shared_ptr<SetStructure> superSet : front->m_superSets) {
                            queue.push_back(superSet);
                        }
                    }
                }
            }
            bool hasData() const override {
                return DataTypePolicy::hasData();
            }
            bool containsData(AbstractElementPtr ptr) const override {
                return DataTypePolicy::containsData(ptr);
            }
            bool removeData(AbstractElementPtr ptr) override {
                return DataTypePolicy::removeData(ptr);
            }
            void innerAdd(AbstractElementPtr ptr) override {
                auto rootRedefinedSet = m_structure->m_rootRedefinedSet;
                if (rootRedefinedSet.get() != m_structure.get()) {
                    return rootRedefinedSet->m_set.innerAdd(ptr);
                }

                // std::lock_guard<std::mutex> setLock(m_mutex);

                nonOppositeAdd(ptr);
               
                // run opposite
                addToOpposite(ptr);
            }
            void run_add_policy_for_set(AbstractSet& set, AbstractElement& el) const {
                set.runAddPolicy(el);
            }
            bool check_opposite_enabled_for_set(AbstractSet& set) const {
                return oppositeEnabled();
            }
            void run_add_opposite_for_set(AbstractSet& set, AbstractElement& el) const {
                set.oppositeAdd(el);
            }
            void nonOppositeAdd(AbstractElementPtr ptr) override {
                nonPolicyAdd(ptr);

                // run policies
                std::list<std::shared_ptr<SetStructure>> queue;
                std::unordered_set<std::shared_ptr<SetStructure>> visited;
                queue.push_back(m_structure->m_rootRedefinedSet);
                while (!queue.empty()) {
                    auto front = queue.front();
                    queue.pop_front();
                    if (visited.count(front)) {
                        continue;
                    }
                    visited.insert(front);
                    front->m_set.runAddPolicy(*ptr);
                    for (auto redefinedSet : front->m_redefinedSets) {
                        redefinedSet->m_set.runAddPolicy(*ptr);
                    }
                    for (auto superSet : front->m_superSets) {
                        queue.push_back(superSet);
                    }
                }
            }
            void nonPolicyAdd(AbstractElementPtr ptr) override {

                auto rootRedefinedSet = m_structure->m_rootRedefinedSet;

                // look at supersets to allocate data
                {
                    std::list<std::shared_ptr<SetStructure>> queue;
                    std::unordered_set<std::shared_ptr<SetStructure>> visited;
                    queue.push_back(rootRedefinedSet);
                    while (!queue.empty()) {
                        auto front = queue.front();
                        queue.pop_front();
                        if (visited.count(front)) {
                            continue;
                        }
                        visited.insert(front);
                        front->m_set.allocatePtr(ptr, *rootRedefinedSet);
                        for (auto redefinedSet : front->m_redefinedSets) {
                            redefinedSet->m_set.allocatePtr(ptr, *rootRedefinedSet);
                        }
                        for (auto superSet : front->m_superSets) {
                            queue.push_back(superSet);
                        }
                    }
                }

                // add data via policy
                DataTypePolicy::addData(ptr);

                // mark superSets that we have data
                {
                    std::list<std::shared_ptr<SetStructure>> queue;
                    std::unordered_set<std::shared_ptr<SetStructure>> visited;
                    for (auto superSet : rootRedefinedSet->m_superSets) {
                        queue.push_back(superSet);
                    }
                    while (!queue.empty()) {
                        auto front = queue.front();
                        queue.pop_front();
                        if (visited.count(front)) {
                            continue;
                        }
                        visited.insert(front);
                        front->m_subSetsWithData.insert(rootRedefinedSet);
                        if (!front->m_set.hasData()) {
                            for (auto superSet : front->m_superSets) {
                                queue.push_back(superSet);
                            }
                        }
                    }
                }
                
                // increase size
                {
                    std::list<std::shared_ptr<SetStructure>> queue;
                    std::unordered_set<std::shared_ptr<SetStructure>> visited;
                    queue.push_back(rootRedefinedSet);
                    while (!queue.empty()) {
                        auto front = queue.front();
                        queue.pop_front();
                        if (visited.count(front)) {
                            continue;
                        }
                        visited.insert(front);
                        front->m_size++;
                        for (auto redefinedSet : front->m_redefinedSets) {
                            redefinedSet->m_size++;
                        }
                        for (auto superSet : front->m_superSets) {
                            queue.push_back(superSet);
                        }
                    }
                }
                m_el.m_node.lock()->setReference(m_structure, ptr);
            }
            void innerRemove(AbstractElementPtr ptr) override {
                auto rootRedefinedSet = m_structure->m_rootRedefinedSet;
                if (rootRedefinedSet.get() != m_structure.get()) {
                    return rootRedefinedSet->m_set.innerRemove(ptr);
                }

                // TODO figure out threading
                // std::lock_guard<std::mutex> setLock(m_mutex);
                auto setwithEl = nonOppositeRemoveHelper(ptr);

                // run opposite
                {
                    std::list<std::shared_ptr<SetStructure>> queue;
                    std::unordered_set<std::shared_ptr<SetStructure>> visited;
                    queue.push_back(setwithEl);
                    while (!queue.empty()) {
                        auto front = queue.front();
                        queue.pop_front();
                        if (visited.count(front)) {
                            continue;
                        }
                        visited.insert(front);
                        bool oppositeRan = false;
                        if (ptr.loaded()) {
                            if (!oppositeRan && front->m_set.oppositeEnabled()) {
                                front->m_set.oppositeRemove(*ptr);
                                oppositeRan = true;
                            }
                        }
                        for (auto redefinedSet : front->m_redefinedSets) {
                            if (ptr.loaded()) {
                                if (!oppositeRan && redefinedSet->m_set.oppositeEnabled()) {
                                    redefinedSet->m_set.oppositeRemove(*ptr);
                                    oppositeRan = true;
                                }
                            }
                        }
                        if (!oppositeRan) {
                            for (auto superSet : front->m_superSets) {
                                queue.push_back(superSet);
                            }
                        }
                    }
                }

                m_el.m_node.lock()->removeReference(m_structure, ptr);
            }
            std::shared_ptr<SetStructure> nonOppositeRemoveHelper(AbstractElementPtr ptr) {
                auto rootRedefinedSet = m_structure->m_rootRedefinedSet;

                // remove
                std::shared_ptr<SetStructure> setwithEl;
                {
                    std::list<std::shared_ptr<SetStructure>> queue;
                    queue.push_back(rootRedefinedSet);
                    std::unordered_set<std::shared_ptr<SetStructure>> visited;
                    while (!queue.empty()) {
                        auto front = queue.front();
                        queue.pop_front();
                        if (visited.count(front)) {
                            continue;
                        }
                        visited.insert(front);
                        if (front->m_set.containsData(ptr)) {
                            setwithEl = front;
                            front->m_set.removeData(ptr);
                            if (!front->m_set.hasData()) {
                                // remove us from all super sets m_subSetsWithData unordered_set
                                std::list<std::shared_ptr<SetStructure>> adjustQueue;
                                for (auto superSet : front->m_superSets) {
                                    adjustQueue.push_back(superSet);
                                }
                                while (!adjustQueue.empty()) {
                                    auto adjustFront = adjustQueue.front();
                                    adjustQueue.pop_front();
                                    adjustFront->m_subSetsWithData.erase(front);
                                    if (!adjustFront->m_set.hasData()) {
                                        for (auto adjustSuperSet : adjustFront->m_superSets) {
                                            adjustQueue.push_back(adjustSuperSet);
                                        }
                                    }
                                }
                            }
                        } else {
                            for (auto subSet : front->m_subSetsWithData) {
                                queue.push_back(subSet);
                            }
                        }
                    }
                }

                if (!setwithEl) {
                    throw SetStateException("could not find element being removed in set!");
                }

                // look at supersets to deallocate data
                {
                    std::list<std::shared_ptr<SetStructure>> queue;
                    std::unordered_set<std::shared_ptr<SetStructure>> visited;
                    queue.push_back(setwithEl);
                    while (!queue.empty()) {
                        auto front = queue.front();
                        queue.pop_front();
                        if (visited.count(front)) {
                            continue;
                        }
                        visited.insert(front);
                        front->m_set.deAllocatePtr(ptr);
                        for (auto redefinedSet : front->m_redefinedSets) {
                            redefinedSet->m_set.deAllocatePtr(ptr);
                        }
                        for (auto superSet : front->m_superSets) {
                            queue.push_back(superSet);
                        }
                    }
                }

                // run removal policies adjust size
                {
                    std::list<std::shared_ptr<SetStructure>> queue;
                    std::unordered_set<std::shared_ptr<SetStructure>> visited;
                    queue.push_back(setwithEl);
                    while (!queue.empty()) {
                        auto front = queue.front();
                        queue.pop_front();
                        if (visited.count(front)) {
                            continue;
                        }
                        visited.insert(front);
                        if (ptr.loaded()) {
                            front->m_set.runRemovePolicy(*ptr);
                        }
                        front->m_size--;
                        for (auto redefinedSet : front->m_redefinedSets) {
                            if (ptr.loaded()) {
                                redefinedSet->m_set.runRemovePolicy(*ptr);
                            }
                            redefinedSet->m_size--;
                        }
                        for (auto superSet : front->m_superSets) {
                            queue.push_back(superSet);
                        }
                    }
                }
                return setwithEl;
            }
            void nonOppositeRemove(AbstractElementPtr ptr) override {
                nonOppositeRemoveHelper(ptr);
            }
            struct PtrPolicy {
                ManagedPtr<ManagedType> get(AbstractElementPtr ptr) {
                    return ManagedPtr<ManagedType>(ptr);    
                }
            };
            typedef WrapperSet<ManagedPtr<ManagedType>, PtrPolicy> PtrSet;
        public:
            PrivateSet(U* el) : m_el(*el) {}
            PrivateSet& operator=(PrivateSet& rhs) = delete;
            PrivateSet(PrivateSet& rhs) = delete;
            virtual ~PrivateSet() {}
            AbstractElement& getOwner() const override {
                return m_el;
            }
            bool contains(AbstractElementPtr ptr) const override {
                if (m_structure->m_rootRedefinedSet.get() != m_structure.get()) {
                    return m_structure->m_rootRedefinedSet->m_set.contains(ptr);
                }
                if (containsData(ptr)) {
                    return true;
                }
                for (auto setWithData : m_structure->m_subSetsWithData) {
                    if (setWithData->m_set.contains(ptr)) {
                        return true;
                    }
                }
                return false;
            }
            bool contains(ID id) const override {
                return contains(m_el.m_manager.createPtr(id));
            }
            bool contains(ManagedType& el) const {
                return contains(ManagedPtr<ManagedType>(&el));
            }
            template <class S>
            void opposite(S& (ManagedType::*oppositeSignature)()) {
                class OppositeInterfaceAdapter : public OppositeInterface<ManagedType> {
                    public:
                        S& (ManagedType::*signature)() = 0;
                        U& me;
                        OppositeInterfaceAdapter(U& u, S& (ManagedType::*sig)()) : me(u) {
                            signature = sig;
                        }
                        bool enabled() override {
                            return true;
                        }
                        void addOpposite(ManagedType& el) override {
                            (el.*signature)().m_structure->m_rootRedefinedSet->m_set.nonOppositeAdd(ManagedPtr<U>(&me));
                        }
                        void removeOpposite(ManagedType& el) override {
                            (el.*signature)().m_structure->m_rootRedefinedSet->m_set.nonOppositeRemove(ManagedPtr<U>(&me));
                        }
                };
                m_opposite = std::unique_ptr<OppositeInterfaceAdapter>(new OppositeInterfaceAdapter(m_el, oppositeSignature));
            }
            void opposite(std::unique_ptr<OppositeInterface<ManagedType>> interface) {
                m_opposite = std::move(interface);
            }
            PtrSet ptrs() const {
                return PtrSet(this);
            }
            IDSet ids() const override {
                return IDSet(this);
            }
    };
}
