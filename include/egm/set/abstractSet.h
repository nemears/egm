#pragma once

#include <memory>
#include <unordered_set>
#include "egm/id.h"

namespace EGM {
    enum class SetType {
        SET,
        SINGLETON,
        ORDERED_SET,
        LIST,
        BAG
    };
    
    // This is to convey ownership
    //      COMPOSITE: Element owns all elements in this set
    //      ANTICOMPOSITE: elements in this set must be the owner 
    //                     (can only be singleton, there may only be one anticomposite full set per element)
    //      NONE: no relation to ownership
    enum class CompositionType {
        COMPOSITE,
        ANTI_COMPOSITE,
        NONE
    };

    class AbstractSet;

    class SetStructure {
        public:
            AbstractSet& m_set;
            std::unordered_set<std::shared_ptr<SetStructure>> m_subSetsWithData;
            std::unordered_set<std::shared_ptr<SetStructure>> m_superSets;
            std::unordered_set<std::shared_ptr<SetStructure>> m_subSets;
            std::unordered_set<std::shared_ptr<SetStructure>> m_redefinedSets;
            std::shared_ptr<SetStructure> m_rootRedefinedSet;
            size_t m_size = 0;
            CompositionType m_composition = CompositionType::NONE;
            SetStructure(AbstractSet& set) : m_set(set) {}
    };

    template <class>
    class ManagedPtr;
    class AbstractElement;
    using AbstractElementPtr = ManagedPtr<AbstractElement>;
    template <class, class>
    class WrapperSet;
    struct IDPolicy;
    using IDSet = WrapperSet<ID, IDPolicy>;

    class AbstractSet {

        template <class Tlist, class P1>
        friend class Manager;

        template <template <class> class T, class U, class DataTypePolicy, class ApiPolicy>
        friend class PrivateSet;
        template <template <class> class T, class U>
        friend class SetDataPolicy;
        template <class T>
        friend class SingletonDataPolicy;
        template <template <class> class T, class U, class ApiPolicy>
        friend class Singleton;
        template <template <class> class T, class U, class ApiPolicy>
        friend class OrderedSet;
        template <class T>
        friend class Element;
        template <class S, class WrapperPolicy>
        friend class WrapperSet;
        friend class AbstractElement;
        
        protected:
            std::shared_ptr<SetStructure> m_structure;
            virtual bool hasData() const = 0;
            virtual bool containsData(AbstractElementPtr ptr) const = 0;
            virtual bool removeData(AbstractElementPtr ptr) = 0;
            virtual void runAddPolicy(AbstractElement& el) = 0;
            virtual void runRemovePolicy(AbstractElement& el) = 0;
            virtual bool oppositeEnabled() const = 0;
            virtual void oppositeAdd(AbstractElement& el) = 0;
            virtual void oppositeRemove(AbstractElement& el) = 0;
            virtual void addToOpposite(AbstractElementPtr ptr) = 0;
            virtual void nonOppositeAdd(AbstractElementPtr ptr) = 0;
            virtual void nonPolicyAdd(AbstractElementPtr ptr) = 0;
            virtual void innerAdd(AbstractElementPtr ptr) = 0;
            virtual void nonOppositeRemove(AbstractElementPtr ptr) = 0;
            virtual void innerRemove(AbstractElementPtr ptr) = 0;
            virtual void allocatePtr(__attribute__((unused)) AbstractElementPtr ptr, __attribute__((unused)) SetStructure& set);
            virtual void deAllocatePtr(__attribute__((unused)) AbstractElementPtr ptr);

            class iterator {
                template <class Tlist, class P1>
                friend class Manager;
                template <template <class> class T, class U>
                friend class SetDataPolicy;
                template <class T>
                friend class SingletonDataPolicy;
                template <class S, class WrapperPolicy>
                friend class WrapperSet;
                template <class>
                friend class IndexablePolicy;

                protected:
                    virtual std::unique_ptr<iterator> clone() const = 0;
                public:
                    size_t m_hash = 0;
                    virtual AbstractElementPtr getCurr() const = 0;
                    virtual void next() = 0;
                    iterator() {}
                    iterator(const iterator& rhs) {
                        m_hash = rhs.m_hash;
                    }
                    virtual ~iterator() {}
                    bool operator==(const iterator& rhs) const {
                        return rhs.m_hash == m_hash;
                    }
                    bool operator!=(const iterator& rhs) const {
                        return rhs.m_hash != m_hash;
                    }
            };
        public:
            AbstractSet() {
                m_structure = std::make_shared<SetStructure>(*this);
                m_structure->m_rootRedefinedSet = m_structure;
            }
            virtual ~AbstractSet() {
                for (auto subSet : m_structure->m_subSets) {
                    auto superSetsIt = subSet->m_superSets.begin();
                    while (
                            superSetsIt != subSet->m_superSets.end() &&
                            (*superSetsIt).get() != m_structure->m_rootRedefinedSet.get()) 
                    {
                        superSetsIt++;
                    }
                    if (superSetsIt != subSet->m_superSets.end()) {
                        subSet->m_superSets.erase(superSetsIt);
                    }
                }
                for (auto superSet : m_structure->m_superSets) {
                    auto subSetsIt = superSet->m_subSets.begin();
                    while (
                        subSetsIt != superSet->m_subSets.end() &&
                        (*subSetsIt).get() != m_structure->m_rootRedefinedSet.get()        
                    ) {
                        subSetsIt++;
                    }
                    if (subSetsIt != superSet->m_subSets.end()) {
                        superSet->m_subSets.erase(subSetsIt);
                    }
                    
                    auto subSetsWithDataIt = superSet->m_subSetsWithData.begin();
                    while (
                        subSetsWithDataIt != superSet->m_subSetsWithData.end() &&
                        (*subSetsWithDataIt).get() != m_structure.get()
                    ) {
                        subSetsWithDataIt++;
                    }
                    if (subSetsWithDataIt != superSet->m_subSetsWithData.end()) {
                        superSet->m_subSetsWithData.erase(subSetsWithDataIt);
                    }
                }
                for (auto redefinedSet : m_structure->m_redefinedSets) {
                    redefinedSet->m_rootRedefinedSet.reset();
                }
                m_structure->m_subSets.clear();
                m_structure->m_superSets.clear();
                m_structure->m_redefinedSets.clear();
                m_structure->m_subSetsWithData.clear();
                m_structure->m_rootRedefinedSet.reset();
                m_structure.reset();
            }
            virtual void subsets(AbstractSet& superSet) {
                auto rootRedefinedSet = m_structure->m_rootRedefinedSet;
                auto superSetRootRedefinedSet = superSet.m_structure->m_rootRedefinedSet;
                superSetRootRedefinedSet->m_subSets.insert(rootRedefinedSet);
                rootRedefinedSet->m_superSets.insert(superSetRootRedefinedSet);
                if (superSetRootRedefinedSet->m_composition != CompositionType::NONE) {
                    rootRedefinedSet->m_composition = superSetRootRedefinedSet->m_composition;
                }
            }
            virtual void redefines(AbstractSet& redefinedSet) {
                auto redefinedStructure = redefinedSet.m_structure->m_rootRedefinedSet;
                for (auto superSet : redefinedStructure->m_superSets) {
                    m_structure->m_superSets.insert(superSet);
                    superSet->m_subSets.erase(redefinedStructure);
                    superSet->m_subSets.insert(m_structure);
                }
                for (auto subSet : redefinedStructure->m_subSets) {
                    m_structure->m_subSets.insert(subSet);
                    subSet->m_superSets.erase(redefinedStructure);
                    subSet->m_superSets.insert(m_structure);
                }
                for (auto redefinedSetRedefinedSet : redefinedStructure->m_redefinedSets) {
                    m_structure->m_redefinedSets.insert(redefinedSetRedefinedSet);
                    redefinedSetRedefinedSet->m_rootRedefinedSet = m_structure;
                    redefinedSetRedefinedSet->m_redefinedSets.erase(redefinedStructure);
                    redefinedSetRedefinedSet->m_redefinedSets.insert(m_structure);
                }
                m_structure->m_redefinedSets.insert(redefinedStructure);
                redefinedStructure->m_superSets.clear();
                redefinedStructure->m_subSets.clear();
                redefinedStructure->m_redefinedSets.clear();
                redefinedStructure->m_rootRedefinedSet = m_structure;
                m_structure->m_rootRedefinedSet = m_structure;
                if (m_structure->m_composition != CompositionType::NONE) {
                    redefinedSet.setComposition(m_structure->m_composition);
                } else if (redefinedStructure->m_composition != CompositionType::NONE) {
                    this->setComposition(redefinedStructure->m_composition);
                }
            }
            void setComposition(CompositionType composition) {
                m_structure->m_composition = composition;
            }
            CompositionType getComposition() const {
                return m_structure->m_rootRedefinedSet->m_composition;
            }
            virtual std::unique_ptr<iterator> beginPtr() const = 0;
            virtual std::unique_ptr<iterator> endPtr() const = 0;
            virtual bool contains(AbstractElementPtr ptr) const = 0;
            virtual bool contains(ID id) const = 0;
            size_t size() const {
                return m_structure->m_size;
            }
            bool empty() const {
                return m_structure->m_size == 0;
            }
            virtual SetType setType() const = 0;
            virtual bool readonly() const {
                return true;
            }
            bool rootSet() const {
                return m_structure->m_rootRedefinedSet == m_structure;
            }
            bool isSubSetOf(AbstractSet& set) const {
                std::list<std::shared_ptr<SetStructure>> queue = {m_structure};
                while (!queue.empty()) {
                    auto front = queue.front();
                    queue.pop_front();
                    if (front == set.m_structure) {
                        return true;
                    }
                    for (auto superSet : front->m_superSets) {
                        queue.push_back(superSet);
                    }
                }
                return false;
            }
            AbstractSet* subSetContains(ID id) const {
                for (auto subSet : m_structure->m_rootRedefinedSet->m_subSets) {
                    if (subSet->m_set.contains(id)) {
                        return &subSet->m_set;
                    }
                }
                return 0;
            }
            virtual IDSet ids() const = 0;
    };


    class AbstractReadableSet : virtual public AbstractSet {
        public:
            virtual void add(ID id) = 0;
            bool readonly() const override {
                return false;
            }
    };    
}

inline std::ostream& operator<<(std::ostream& stream, const EGM::SetType& setType) {
    switch (setType) {
        case EGM::SetType::SET : return stream << "Set";
        case EGM::SetType::SINGLETON : return stream << "Singleton";
        case EGM::SetType::ORDERED_SET : return stream << "Ordered Set";
        case EGM::SetType::LIST : return stream << "List";
        case EGM::SetType::BAG : return stream << "Bag";
    }
    return stream << "** ERROR in EGM::SetType::operator<< Contact Dev! **";
}

