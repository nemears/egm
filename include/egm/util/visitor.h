#pragma once

#include "templateTypeList.h"
#include "integerList.h"
#include "egm/manager/dummyManager.h"

namespace EGM {
    template <class List, template <class> class Curr, class AllTypesList, class Bases = typename Curr<DummyManager::BaseElement>::Info::BaseList>
    struct AddChildrenTypes;

    template <class List, template <class> class Curr, class AllTypesList, template <class> class Front, template <class> class ... Bases>
    struct AddChildrenTypes<List, Curr, AllTypesList, TemplateTypeList<Front, Bases...>> {
        static const std::size_t front_type = TemplateTypeListIndex<Front, AllTypesList>::result;
        using ListAndFront = std::conditional_t<HasInt<front_type, List>::value, List ,typename IntAppend<List, front_type>::type>;
        using result = typename AddChildrenTypes<
                typename AddChildrenTypes<
                    ListAndFront, 
                    Front,
                    AllTypesList
                >::result, 
                Curr, 
                AllTypesList,
                TemplateTypeList<Bases...>
            >::result;
    };

    template <class List, template <class> class Curr, class AllTypesList>
    struct AddChildrenTypes<List, Curr, AllTypesList, TemplateTypeList<>> {
        using result = List;
    };

    template <class Visitor, template <class> class Curr, class AllTypesList, class Visited = IntList<>, class Bases = typename Curr<DummyManager::BaseElement>::Info::BaseList>
    struct VisitBasesDFS;

    template <class Visitor, template <class> class Curr, class AllTypesList, class Visited, template <class> class Front, template <class> class ... Rest>
    struct VisitBasesDFS<Visitor, Curr, AllTypesList, Visited, TemplateTypeList<Front, Rest...>> {
        using CurrentTypeList = TemplateTypeList<Front, Rest...>;
        static void visit(Visitor& visitor) {
            if constexpr  (TemplateTypeListSize<typename Curr<DummyManager::BaseElement>::Info::BaseList>::result == TemplateTypeListSize<CurrentTypeList>::result) {
                visitor.template visit<Curr>();
            }
            constexpr std::size_t BaseID = TemplateTypeListIndex<Front, AllTypesList>::result;
            constexpr std::size_t CurrID = TemplateTypeListIndex<Curr, AllTypesList>::result;
            using VisitedAndCurr = std::conditional_t<HasInt<CurrID, Visited>::value, Visited, typename IntAppend<Visited, CurrID>::type>;
            if constexpr (!HasInt<BaseID, VisitedAndCurr>::value) {
                VisitBasesDFS<Visitor, Front, AllTypesList, VisitedAndCurr>::visit(visitor);
                using NewVisited = typename AddChildrenTypes<typename IntAppend<VisitedAndCurr, BaseID>::type, Front, AllTypesList>::result;
                VisitBasesDFS<Visitor, Curr, AllTypesList, NewVisited, TemplateTypeList<Rest...>>::visit(visitor);
            } else {
                VisitBasesDFS<Visitor, Curr, AllTypesList, VisitedAndCurr, TemplateTypeList<Rest...>>::visit(visitor);
            }
        }
    };

    template <class Visitor, template <class> class Curr, class AllTypesList, class Visited>
    struct VisitBasesDFS<Visitor, Curr, AllTypesList, Visited, TemplateTypeList<>> {
        static void visit(Visitor& visitor) {
            if constexpr (TemplateTypeListSize<typename Curr<DummyManager::BaseElement>::Info::BaseList>::result == 0) {
                visitor.template visit<Curr>();
            }
        }
    };

    template <class Visitor, template <class> class Start, class AllTypesList>
    void visitBasesDFS(Visitor& visitor) {
        VisitBasesDFS<Visitor, Start, AllTypesList>::visit(visitor);
    }

    template <class Visitor, class AllTypes, class CurrentList, class Visited = IntList<>>
    struct VisitBasesBFS;

    template <class Visitor, class AllTypes, template <class> class Front, template <class> class ... Rest, class Visited>
    struct VisitBasesBFS<Visitor, AllTypes, TemplateTypeList<Front, Rest...>, Visited> {
        static void visit(Visitor& visitor) {
            constexpr int FrontID = TemplateTypeListIndex<Front, AllTypes>::result;
            if constexpr (!HasInt<FrontID, Visited>::value) {
                visitor.template visit<Front>();
                using NewVisited = typename IntAppend<Visited, FrontID>::type;
                using NewList = typename TemplateTypeListCat<TemplateTypeList<Rest...>, typename Front<DummyManager::BaseElement>::Info::BaseList>::result;
                VisitBasesBFS<Visitor, AllTypes, NewList, NewVisited>::visit(visitor);
            } else {
                VisitBasesBFS<Visitor, AllTypes, TemplateTypeList<Rest...>, Visited>::visit(visitor);
            }
        }
    };

    template <class Visitor, class AllTypes, class Visited>
    struct VisitBasesBFS<Visitor, AllTypes, TemplateTypeList<>, Visited> {
        static void visit(__attribute__((unused)) Visitor& visitor) {}
    };

    template <class Visitor, template <class> class Start, class AllTypes>
    void visitBasesBFS(Visitor& visitor) {
        VisitBasesBFS<Visitor, AllTypes, TemplateTypeList<Start>>::visit(visitor);
    }

    template <class Queue>
    struct AllBasesBFS;

    template <template <class> class Front, template <class> class ... Rest>
    struct AllBasesBFS<TemplateTypeList<Front, Rest...>> {
        using result = typename TemplateTypeListCat<
                TemplateTypeList<Front>, 
                typename AllBasesBFS<
                    typename TemplateTypeListCat<
                        TemplateTypeList<Rest...>, 
                        typename Front<DummyManager::BaseElement>::Info::BaseList
                    >::result
                >::result
            >::result;
    };

    template <>
    struct AllBasesBFS<TemplateTypeList<>> {
        using result = TemplateTypeList<>;
    };

    template <class Visitor, class AllTypes, class ReverseList, class Visited = IntList<>>
    struct VisitBasesReverseBFS;

    template <class Visitor, class AllTypes, class Visited>
    struct VisitBasesReverseBFS<Visitor, AllTypes, TemplateTypeList<>, Visited> {
        static void visit(__attribute__((unused)) Visitor& visitor) {}
    };

    template <class Visitor, class AllTypes, template <class> class Last, template <class> class ... Rest, class Visited>
    struct VisitBasesReverseBFS<Visitor, AllTypes, TemplateTypeList<Last, Rest...>, Visited> {
        static void visit(Visitor& visitor) {
            constexpr int last_id = TemplateTypeListIndex<Last, AllTypes>::result;
            if constexpr (!HasInt<last_id, Visited>::value) {
                visitor.template visit<Last>();
                using NewVisited = typename IntAppend<Visited, last_id>::type;
                VisitBasesReverseBFS<Visitor, AllTypes, TemplateTypeList<Rest...>, NewVisited>::visit(visitor);
            } else {
                VisitBasesReverseBFS<Visitor, AllTypes, TemplateTypeList<Rest...>, Visited>::visit(visitor);
            }
        }
    };

    template <class Visitor, class AllTypes, template <class> class Start>
    void visitBasesReverseBFS(Visitor& visitor) {
        return VisitBasesReverseBFS<Visitor, AllTypes, typename ReverseList<typename AllBasesBFS<TemplateTypeList<Start>>::result>::result>::visit(visitor);
    }
}
