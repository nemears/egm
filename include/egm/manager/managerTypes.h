#pragma once

#include <functional>
#include <string>
#include <memory>
#include <type_traits>
#include "abstractElement.h"
#include "abstractManager.h"
#include "typeInfo.h"
#include "egm/util/templateTypeList.h"
#include "egm/util/visitor.h"
#include "egm/managedPtr.h"

namespace EGM {
    template <class TypeList>
    class ManagerTypes : virtual public AbstractManager {
        public:
            class BaseElement;
            
            // Gen Base Hierarchy is a class that will be used to create dynamic objects that inherit from and can correspond to their bases
            // This is used mostly internally but all types created by the manager will be of this policy
            template <template <class> class T, class Bases = typename T<BaseElement>::Info::BaseList>
            struct GenBaseHierarchy;
        protected:
            struct AbstractManagerTypeInfo {
                virtual ~AbstractManagerTypeInfo() {}
                virtual void forEachSet(BaseElement& el, std::function<void(std::string, AbstractSet&)>)  = 0;
                virtual bool is(std::size_t type) = 0;
                virtual AbstractElementPtr create(ManagerTypes&) = 0;
            };

            std::unordered_map<std::size_t, std::unique_ptr<AbstractManagerTypeInfo>> m_types;

        public:
            using Types = TypeList;
            template <template <class> class Type>
            using ElementType = TemplateTypeListIndex<Type, Types>;
            // Base element for all types created by manager, the policy classes provided will be filled out with
            // this BaseElement as part of their policy
            class BaseElement : public AbstractElement {
                protected:    
                    using AbstractElement::AbstractElement;
                    BaseElement(std::size_t elementType, AbstractManager& manager) : AbstractElement(elementType, manager) {}
                public:
                    using manager = ManagerTypes;
                    // is function to compare types compile time O(1)
                    template <template <class> class T>
                    bool is() const {
                        return dynamic_cast<ManagerTypes&>(m_manager).m_types.at(m_elementType)->is(ElementType<T>::result); 
                    }

                    // as to cast to other managed types
                    template <template<class> class T>
                    T<typename ManagerTypes::template GenBaseHierarchy<T>>& as() {
                        if (is<T>()) {
                            return dynamic_cast<T<typename ManagerTypes::template GenBaseHierarchy<T>>&>(*this);
                        }
                        throw ManagerStateException("Can not convert element to that type!");
                    }
                    ManagerTypes& getManager() {
                        return dynamic_cast<ManagerTypes&>(m_manager);
                    }
            };

            template <template <class> class Type>
            struct ManagerTypeInfo : public AbstractManagerTypeInfo {
                private:
                    using Function = std::function<void(std::string, AbstractSet&)>;
                    template <template <class> class CurrType = Type, std::size_t I = 0>
                    bool isHelper(std::size_t type) {
                        using BaseList = typename CurrType<BaseElement>::Info::BaseList;
                        if constexpr (I < TemplateTypeListSize<BaseList>::result) {
                            if (isHelper<TemplateTypeListType<I, BaseList>::template result>(type)) {
                                return true; 
                            }
                            return isHelper<CurrType, I + 1>(type);
                        } else {
                            if (ElementType<CurrType>::result == type) {
                                return true;
                            }
                            return false;
                        }
                    }
                    struct ForEachSetVisitor {
                        BaseElement& el;
                        Function f;
                        template <template <class> class Curr>
                        void visit() {
                            if constexpr (HasSets<Curr>{}) {
                                for (auto& setPair : Curr<BaseElement>::Info::sets(el.template as<Curr>())) {
                                    f(setPair.first, *setPair.second);
                                }
                            }
                        }
                    };
                public:
                    void forEachSet(BaseElement& el, Function f) override {
                        ForEachSetVisitor visitor { el, f };
                        visitBasesDFS<ForEachSetVisitor, Type, Types>(visitor);
                    }
                    bool is(std::size_t type) override {
                        return isHelper(type);                        
                    }
                    AbstractElementPtr create(ManagerTypes& manager) override {
                        return manager.template create<Type>();
                    }
            };

        public:
            template <template <class> class T>
            struct GenBaseHierarchy<T, TemplateTypeList<>> : virtual public BaseElement {
                protected:
                    GenBaseHierarchy(std::size_t elementType, AbstractManager& manager) : BaseElement(elementType, manager) {}
            };

            template <template <class> class T, template <class> class First, template <class> class ... Types>
            struct GenBaseHierarchy<T, TemplateTypeList<First, Types...>> :
                virtual public First<GenBaseHierarchy<First>>,
                public GenBaseHierarchy<T, TemplateTypeList<Types...>>
            {
                protected:
                    GenBaseHierarchy(std::size_t elementType, AbstractManager& manager) : 
                        BaseElement(elementType, manager),
                        First<GenBaseHierarchy<First>>(elementType, manager),
                        GenBaseHierarchy<T, TemplateTypeList<Types...>>(elementType, manager)
                    {}
            };
        private:
            template <class TypesToPopulate, class Dummy = void>
            struct PopulateTypes;

            template <template <class> class First, template <class> class ... Rest, class Dummy>
            struct PopulateTypes<TemplateTypeList<First, Rest ...>, Dummy> {
                static void populate(ManagerTypes& manager) {
                    manager.m_types.emplace(ElementType<First>::result, std::make_unique<ManagerTypeInfo<First>>());
                    PopulateTypes<TemplateTypeList<Rest ...>>::populate(manager);
                }
            };

            template <class Dummy>
            struct PopulateTypes<TemplateTypeList<>, Dummy> {
                static void populate(__attribute__((unused)) ManagerTypes& manager) {}
            };
        protected:
            virtual ManagedPtr<BaseElement> registerPtr(std::shared_ptr<AbstractElement> ptr) = 0;

        private:
            // IsAbstract Implementation
            template <template <class> class Type>
            static auto testName(int) -> TemplateTrue<decltype(ElementInfo<Type>::name())>;
            template <template <class> class>
            static auto testName(...) -> std::false_type;
            template <template <class> class Type>
            struct HasName : decltype(testName<Type>(0)) {};
            template <template <class> class Type>
            static auto testAbstract(int) -> TemplateTrue<decltype(ElementInfo<Type>::abstract)>;
            template <template <class> class Type>
            static auto testAbstract(...) -> std::false_type;
            template <template <class> class Type>
            struct HasAbstract : decltype(testAbstract<Type>(0)) {};
        protected:
            template <template <class> class Type, bool TypeHasAbstract  = HasAbstract<Type>{}>
            struct IsAbstract;

            template <template <class> class Type>
            struct IsAbstract<Type, true> : public std::conditional_t<ElementInfo<Type>::abstract || !HasName<Type>{}, std::true_type, std::false_type> {};
            template <template <class> class Type>
            struct IsAbstract<Type, false> : public std::conditional_t<!HasName<Type>{}, std::true_type, std::false_type> {};
        public:
            ManagerTypes() {
                PopulateTypes<Types>::populate(*this);
            }
            // create factory function
            template <template <class> class T>
            ManagedPtr<T<GenBaseHierarchy<T>>> create() {
                if constexpr (IsAbstract<T>{}) {
                    throw ManagerStateException("Trying to instantiate an abstract type!");
                }
                auto ptr = std::make_shared<T<GenBaseHierarchy<T>>>(ElementType<T>::result, *this);
                return registerPtr(ptr);
            }
            AbstractElementPtr create(std::size_t elementType) override {
                return m_types.at(elementType)->create(*this);
            }

            template <template <class> class Type>
            using Implementation = Type<GenBaseHierarchy<Type>>;
            template <template <class> class Type>
            using Pointer = EGM::ManagedPtr<Implementation<Type>>;
    };
}
