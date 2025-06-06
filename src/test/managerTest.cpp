#include "gtest/gtest.h"
#include "egm/egm.h"

using namespace EGM;

class ManagerTest : public ::testing::Test {};

template <class T>
struct TClass1 {};

template <class T>
struct TClass2 {};

TEST_F(ManagerTest, TemplateTypeListTypeTest) {

    using TestTypes = TemplateTypeList<TClass1, TClass2>;

    auto isTypeResultSame = std::is_same<
            TClass1<int>,
            TemplateTypeListType<0, TestTypes>::result<int>
        >::value;

    ASSERT_TRUE(isTypeResultSame);

    auto indexResult1 = TemplateTypeListIndex<TClass1, TestTypes>::result;
    ASSERT_EQ(indexResult1, 0);
    auto indexResult2 = TemplateTypeListIndex<TClass2, TestTypes>::result;
    ASSERT_EQ(indexResult2, 1);
    auto indexResult3 = TemplateTypeListIndex<TemplateTypeListType<0, TestTypes>::result, TestTypes>::result;
    ASSERT_EQ(indexResult3, 0);
}

namespace EGM {
    template <class ManagerPolicy>
    struct Base : public ManagerPolicy {
        using Info = TypeInfo<Base>;
        Base() :
            ManagerPolicy::manager::BaseElement(0, dummyManager),
            ManagerPolicy(0, dummyManager) 
        {}
        Base(std::size_t elementType, AbstractManager& manager) :
            ManagerPolicy::manager::BaseElement(elementType, manager),
            ManagerPolicy(elementType, manager) 
        {}
        virtual ~Base() {}
    };

    template <>
    struct ElementInfo<Base> {};

    template <class ManagerPolicy>
    struct Right : public ManagerPolicy {
        using Info = TypeInfo<Right, TemplateTypeList<Base>>;
        Right() :
            ManagerPolicy::manager::BaseElement(0, dummyManager),
            ManagerPolicy(0, dummyManager) 
        {}
        Right(std::size_t elementType, AbstractManager& manager) :
            ManagerPolicy::manager::BaseElement(elementType, manager),
            ManagerPolicy(elementType, manager) 
        {}
        virtual ~Right() {}
    };
    
    template <>
    struct ElementInfo<Right> {};

    template <class ManagerPolicy>
    struct Left : public ManagerPolicy {
        using Info = TypeInfo<Left, TemplateTypeList<Base>>;
        Left () :
            ManagerPolicy::manager::BaseElement(0, dummyManager),
            ManagerPolicy(0, dummyManager) 
        {}
        Left (std::size_t elementType, AbstractManager& manager) :
            ManagerPolicy::manager::BaseElement(elementType, manager),
            ManagerPolicy(elementType, manager) 
        {}
        virtual ~Left() {}
    };
    
    template <>
    struct ElementInfo<Left> {};

    template <class ManagerPolicy>
    struct Diamond : public ManagerPolicy {
        using Info = TypeInfo<Diamond, TemplateTypeList<Right, Left>>;
        Diamond() :
            ManagerPolicy::manager::BaseElement(0, dummyManager),
            ManagerPolicy(0, dummyManager) 
        {}
        Diamond(std::size_t elementType, AbstractManager& manager) :
            ManagerPolicy::manager::BaseElement(elementType, manager),
            ManagerPolicy(elementType, manager) 
        {}
        virtual ~Diamond() {}
    };
    
    template <>
    struct ElementInfo<Diamond> {
        static constexpr bool abstract = false;
        static const std::string name() {
            return "Diamond";
        }
    };

    using DiamondTypes = TemplateTypeList<Base, Right, Left, Diamond>;
    using DiamondManager = Manager<DiamondTypes>;
}

TEST_F(ManagerTest, diamondElementTest) {
    DiamondManager m;
    auto diamond = m.create<Diamond>();
    auto& right = diamond->as<Right>();
    ASSERT_EQ(right, dynamic_cast<Right<DiamondManager::GenBaseHierarchy<Right>>&>(*diamond));
    auto& left = diamond->as<Left>();
    ASSERT_EQ(left, dynamic_cast<Left<DiamondManager::GenBaseHierarchy<Left>>&>(*diamond));
    auto& base = diamond->as<Base>();
    ASSERT_EQ(base, dynamic_cast<Base<DiamondManager::GenBaseHierarchy<Base>>&>(*diamond));
}

namespace EGM {

    template <class>
    struct DerivedReferencingBase;

    template <class ManagerPolicy>
    struct BaseReferencingDerived : public ManagerPolicy {
        using Info = TypeInfo<BaseReferencingDerived>;
        using DerivedSet = Set<DerivedReferencingBase, BaseReferencingDerived>;
        DerivedSet derived = DerivedSet(this);
        DerivedSet& getDerived() {
            return derived;
        }
        void init() {
            derived.opposite(&DerivedSet::ManagedType::getBase);
        }
        BaseReferencingDerived() :
            ManagerPolicy::manager::BaseElement(0, dummyManager),
            ManagerPolicy(0, dummyManager)
        {
            init();
        }
        BaseReferencingDerived(std::size_t elementType, AbstractManager& manager) :
            ManagerPolicy::manager::BaseElement(elementType, manager),
            ManagerPolicy(elementType, manager)
        {
            init();
        }
    };

    template <>
    struct ElementInfo<BaseReferencingDerived> {
        static const bool abstract = false;
        static const std::string name() {
            return "BaseReferencingDerived";
        }
    };
    
    template <class ManagerPolicy>
    struct DerivedReferencingBase : public ManagerPolicy {
        using Info = TypeInfo<DerivedReferencingBase, TemplateTypeList<BaseReferencingDerived>>;
        using BaseSet = Set<BaseReferencingDerived, DerivedReferencingBase>;
        BaseSet base = BaseSet(this);
        BaseSet& getBase() {
            return base;
        }
        void init() {
            base.opposite(&BaseSet::ManagedType::getDerived);
        }
        DerivedReferencingBase () :
            ManagerPolicy::manager::BaseElement(0, dummyManager),
            ManagerPolicy(0, dummyManager)
        {
            init();
        }
        DerivedReferencingBase (std::size_t elementType, AbstractManager& manager) :
            ManagerPolicy::manager::BaseElement(elementType, manager),
            ManagerPolicy(elementType, manager)
        {
            init();
        }
    };

    template <>
    struct ElementInfo<DerivedReferencingBase> {
        static const bool abstract = false;
        static const std::string name() {
            return "DerivedReferencingBase";
        }
    };

    using CircularDependencyManager = Manager<TemplateTypeList<BaseReferencingDerived, DerivedReferencingBase>>;
}

TEST_F(ManagerTest, circularReferenceTest) {
    CircularDependencyManager m;
    auto base = m.create<BaseReferencingDerived>();
    auto derived = m.create<DerivedReferencingBase>();
    base->derived.add(derived);
    ASSERT_TRUE(base->derived.contains(derived));
    ASSERT_TRUE(derived->base.contains(base));
}

namespace EGM {
    template <class ManagerPolicy>
    struct TestElement : public ManagerPolicy {
        using Info = TypeInfo<TestElement>;
        using OwnedElementsSet = Set<TestElement, TestElement>;
        using OwnerSingleton = Singleton<TestElement, TestElement>;
        OwnedElementsSet ownedElements = OwnedElementsSet(this);
        OwnerSingleton owner = OwnerSingleton(this);
        OwnedElementsSet& getOwnedElements() {
            return ownedElements;
        }
        OwnerSingleton& getOwnerSingleton() {
            return owner;
        } 
        void init() {
            ownedElements.setComposition(CompositionType::COMPOSITE);
            ownedElements.opposite(&OwnerSingleton::ManagedType::getOwnerSingleton);
            owner.setComposition(CompositionType::ANTI_COMPOSITE);
            owner.opposite(&OwnedElementsSet::ManagedType::getOwnedElements);
        }
        TestElement() :
            ManagerPolicy::manager::BaseElement(0, dummyManager),
            ManagerPolicy(0, dummyManager)
        {
            init();
        }
        TestElement(std::size_t elementType, AbstractManager& manager) :
            ManagerPolicy::manager::BaseElement(elementType, manager),
            ManagerPolicy(elementType, manager)
        {
            init();
        }
    };

    template <>
    struct ElementInfo<TestElement> {
        static const bool abstract = false; // for testing
        static const std::string name() {
            return "TestElement";
        }
        template <class ManagerPolicy>
        static SetList sets(TestElement<ManagerPolicy>& el) {
            return SetList {
                std::make_pair<std::string, AbstractSet*>("ownedElements", &el.getOwnedElements()),
                std::make_pair<std::string, AbstractSet*>("owner", &el.getOwnerSingleton())
            };
        }
    };

    using TestElementManager = Manager<TemplateTypeList<TestElement>>;
}

TEST_F(ManagerTest, testElementTest) {
    TestElementManager m;
    auto owner = m.create<TestElement>();
    auto ownee = m.create<TestElement>();
    owner->ownedElements.add(ownee);
    ASSERT_TRUE(owner->ownedElements.contains(ownee));
    ASSERT_EQ(*ownee->owner.get(), *owner);
}

TEST_F(ManagerTest, testElementReleaseAndAquireTest) {
    TestElementManager m;
    m.mount(".");
    auto owner = m.create<TestElement>();
    auto ownee = m.create<TestElement>();
    owner->ownedElements.add(ownee);
    owner.release();
    ASSERT_FALSE(owner.loaded());
    owner.aquire();
    ASSERT_TRUE(owner.loaded());
    ASSERT_EQ(owner->ownedElements.size(), 1);
    ASSERT_EQ(owner->ownedElements.front(), *ownee);
    ownee.release();
    ASSERT_FALSE(ownee.loaded());
    ownee.aquire();
    ASSERT_TRUE(ownee->owner.get());
    ASSERT_EQ(*ownee->owner.get(), *owner);
}

namespace EGM {
    template <class>
    struct TestNamespace;

    template <class ManagerPolicy>
    struct TestNamedElement : public ManagerPolicy {
        using Info = TypeInfo<TestNamedElement, TemplateTypeList<TestElement>>;
        using NamespaceSingleton = Singleton<TestNamespace, TestNamedElement>;
        NamespaceSingleton _namespace = NamespaceSingleton(this);
        std::string name;
        NamespaceSingleton& getNamespaceSingleton() {
            return _namespace;
        }
        void init() {
            _namespace.subsets(ManagerPolicy::owner);
            _namespace.opposite(&NamespaceSingleton::ManagedType::getOwnedMembers);
        }
        TestNamedElement() :
            ManagerPolicy::manager::BaseElement(0, dummyManager),
            ManagerPolicy(0, dummyManager)
        {
            init();
        }
        TestNamedElement(std::size_t elementType, AbstractManager& manager) :
            ManagerPolicy::manager::BaseElement(elementType, manager),
            ManagerPolicy(elementType, manager)
        {
            init();
        }
    };

    template <>
    struct ElementInfo<TestNamedElement> {
        static const bool abstract = false;
        static const std::string name() {
            return "TestNamedElement";
        }
        template <class Policy>
        static SetList sets(TestNamedElement<Policy>& el) {
            return SetList {
                std::make_pair<std::string, AbstractSet*>("namespace", &el._namespace)
            };
        }
        template <class Policy>
        struct NameDataPolicy : public AbstractDataPolicy {
            ManagedPtr<TestNamedElement<Policy>> el;
            NameDataPolicy(TestNamedElement<Policy>& ref) {
                el = ManagedPtr<TestNamedElement<Policy>>(&ref);
            }
            std::string getData() override {
                return el->name;
            }
            void setData(std::string data) override {
                el->name = data;
            }
        };
        template <class Policy>
        static DataList data(TestNamedElement<Policy>& el) {
            return DataList {
                createDataPair<NameDataPolicy<Policy>>("name", el)
            };
        }
    };

    template <class ManagerPolicy>
    struct TestNamespace : public ManagerPolicy {
        using Info = TypeInfo<TestNamespace, TemplateTypeList<TestNamedElement>>;
        using NamedElementSet = Set<TestNamedElement, TestNamespace>;
        NamedElementSet members = NamedElementSet(this);
        NamedElementSet ownedMembers = NamedElementSet(this);
        NamedElementSet& getMembers() {
            return members;
        }
        NamedElementSet& getOwnedMembers() {
            return ownedMembers;
        }
        void init() {
            ownedMembers.subsets(ManagerPolicy::ownedElements);
            ownedMembers.subsets(members);
            ownedMembers.opposite(&NamedElementSet::ManagedType::getNamespaceSingleton);
        }
        TestNamespace() :
            ManagerPolicy::manager::BaseElement(0, dummyManager),
            ManagerPolicy(0, dummyManager)
        {
            init();
        }
        TestNamespace(std::size_t elementType, AbstractManager& manager) :
            ManagerPolicy::manager::BaseElement(elementType, manager),
            ManagerPolicy(elementType, manager)
        {
            init();
        }
    };

    template <>
    struct ElementInfo<TestNamespace> {
        static const bool abstract = false;
        static const std::string name() {
            return "TestNamespace";
        }
        template <class Policy>
        static SetList sets(TestNamespace<Policy>& el) {
            return SetList {
                std::make_pair<std::string, AbstractSet*>("members", &el.members),
                std::make_pair<std::string, AbstractSet*>("ownedMembers", &el.ownedMembers)
            };
        }
    };

    using TestNamespaceManager = Manager<TemplateTypeList<TestElement, TestNamedElement, TestNamespace>>;
}

TEST_F(ManagerTest, testNamespaceTest) {
    TestNamespaceManager m;
    auto nmspc = m.create<TestNamespace>();
    auto member = m.create<TestNamedElement>();
    nmspc->ownedMembers.add(member);
    ASSERT_TRUE(nmspc->ownedMembers.contains(member));
    ASSERT_TRUE(nmspc->members.contains(member));
    ASSERT_TRUE(nmspc->ownedElements.contains(member));
    ASSERT_EQ(*member->_namespace.get(), *nmspc);
    ASSERT_EQ(*member->owner.get(), *nmspc);
}

TEST_F(ManagerTest, releaseAndAquireTestNamespace) {
    TestNamespaceManager m;
    m.mount(".");
    auto nmspc = m.create<TestNamespace>();
    auto member = m.create<TestNamedElement>();
    member->name = "test";
    member->_namespace.set(nmspc);
    nmspc.release();
    ASSERT_FALSE(nmspc.loaded());
    nmspc.aquire();
    ASSERT_TRUE(nmspc.loaded());
    ASSERT_EQ(nmspc->ownedMembers.size(), 1);
    member.release();
    ASSERT_FALSE(member.loaded());
    member.aquire();
    ASSERT_TRUE(member->_namespace.get());
    ASSERT_EQ(member->_namespace.get(), nmspc);
    ASSERT_EQ(member->name, "test");
}

namespace EGM {
    template <class>
    struct TestPackage;

    template <class ManagerPolicy>
    struct TestPackageablElement : public ManagerPolicy {
        using Info = TypeInfo<TestPackageablElement, TemplateTypeList<TestNamedElement>>;
        using PackageSingleton = Singleton<TestPackage, TestPackageablElement>;
        PackageSingleton owningPackage = PackageSingleton(this);
        PackageSingleton& getOwningPackageSingleton() {
            return owningPackage;
        }
        void init() {
            owningPackage.subsets(ManagerPolicy::_namespace);
            owningPackage.opposite(&PackageSingleton::ManagedType::getPackagedElements);
        }
        TestPackageablElement() :
            ManagerPolicy::manager::BaseElement(0, dummyManager),
            ManagerPolicy(0, dummyManager)
        {
            init();
        }
        TestPackageablElement(std::size_t elementType, AbstractManager& manager) :
            ManagerPolicy::manager::BaseElement(elementType, manager),
            ManagerPolicy(elementType, manager)
        {
            init();
        }
    };

    template <>
    struct ElementInfo<TestPackageablElement> {
        static const bool abstract = false;
        static const std::string name() {
            return "TestPackageableElement";
        }
    };

    template <class ManagerPolicy>
    struct TestPackage : public ManagerPolicy {
        using Info = TypeInfo<TestPackage, TemplateTypeList<TestPackageablElement, TestNamespace>>;
        using PackageableElementSet = Set<TestPackageablElement, TestPackage>;
        PackageableElementSet packagedElements = PackageableElementSet(this);
        PackageableElementSet& getPackagedElements() {
            return packagedElements;
        }
        void init() {
            packagedElements.subsets(ManagerPolicy::ownedMembers);
            packagedElements.opposite(&PackageableElementSet::ManagedType::getOwningPackageSingleton);
        }
        TestPackage() :
            ManagerPolicy::manager::BaseElement(0, dummyManager),
            ManagerPolicy(0, dummyManager)
        {
            init();
        }
        TestPackage(std::size_t elementType, AbstractManager& manager) :
            ManagerPolicy::manager::BaseElement(elementType, manager),
            ManagerPolicy(elementType, manager) 
        {
            init();    
        }
    };

    template <>
    struct ElementInfo<TestPackage> {
        static const bool abstract = false;
        static const std::string name() {
            return "TestPackage";
        }
    };

    using TestPackageManager = Manager<TemplateTypeList<TestElement, TestNamedElement, TestNamespace, TestPackageablElement, TestPackage>>;
}

TEST_F(ManagerTest, testPackageTest) {
    TestPackageManager m;
    auto pckg = m.create<TestPackage>();
    auto pckgdEl = m.create<TestPackage>();
    pckg->packagedElements.add(pckgdEl);
    ASSERT_TRUE(pckg->packagedElements.contains(pckgdEl));
    ASSERT_TRUE(pckg->ownedMembers.contains(pckgdEl));
    ASSERT_TRUE(pckg->members.contains(pckgdEl));
    ASSERT_TRUE(pckg->ownedElements.contains(pckgdEl));
    ASSERT_EQ(*pckgdEl->owningPackage.get(), *pckg);
    ASSERT_EQ(*pckgdEl->_namespace.get(), *pckg);
    ASSERT_EQ(*pckgdEl->owner.get(), *pckg);
}

namespace EGM {
    template <class>
    struct Innie;

    template <class ManagerPolicy>
    struct Outtie : public ManagerPolicy {
        using Info = TypeInfo<Outtie>;
        using InnieSet = Set<Innie, Outtie>;
        InnieSet innies = InnieSet(this);
        void init() {
            innies.setComposition(CompositionType::COMPOSITE);
        }
        MANAGED_ELEMENT_CONSTRUCTOR(Outtie);
    };

    template <>
    struct ElementInfo<Outtie> {
        static std::string name() { return "Outtie"; }
        template <class Policy>
        static SetList sets(Outtie<Policy>& el) {
            return SetList {
                make_set_pair("innies", el.innies)
            };
        }
    };

    template <class ManagerPolicy>
    struct Innie : public ManagerPolicy {
        using Info = TypeInfo<Innie>;
        void init() {}
        MANAGED_ELEMENT_CONSTRUCTOR(Innie);
    };

    template <>
    struct ElementInfo<Innie> {
        static std::string name() { return "Innie"; }
    };

    using InnieOuttieManager = Manager<TemplateTypeList<Outtie, Innie>>;
}

TEST_F(ManagerTest, emitCompositeSetTest) {
    InnieOuttieManager m;
    auto innie = m.create<Innie>();
    auto outtie = m.create<Outtie>();
    outtie->setID(ID::fromString("zN-UM2AHrXX07rAiNxTmmMwLYI1O"));
    innie->setID(ID::fromString("FqaulNq6bCe_8J5M0Ff2oCCaQD05")); 
    outtie->innies.add(innie);
    std::string expectedEmit = R""""(Outtie:
  id: zN-UM2AHrXX07rAiNxTmmMwLYI1O
  innies:
    - Innie:
        id: FqaulNq6bCe_8J5M0Ff2oCCaQD05)"""";
    std::string generatedEmit;
    ASSERT_NO_THROW(generatedEmit = m.dump(*outtie));
    std::cout << generatedEmit << '\n';
    ASSERT_EQ(expectedEmit, generatedEmit);
}

TEST_F(ManagerTest, parseCompositeSetTest) {
    InnieOuttieManager m;
    ASSERT_NO_THROW(m.open("../src/test/parseCompositeTest.yml"));
    auto el = m.getRoot();
    ASSERT_EQ(el->getElementType(), InnieOuttieManager::ElementType<Outtie>::result);
    auto& outtie = el->as<Outtie>();
    ASSERT_EQ(outtie.innies.size(), 1);
    auto innie = outtie.innies.front();
    ASSERT_EQ(innie.id(), ID::fromString("ZhUTBs8FQ2Mgb5XbfCsySrmyrvTh"));
}

class CreationStoragePolicy {
    public:
        ID last_created = ID::nullID();
    protected:
        void create_storage(AbstractElement& el) {
            last_created = el.getID();
        }

        // doing minimal needed for policy
        AbstractElementPtr loadElement(ID id) {
            return AbstractElementPtr();
        }
        void saveElement(AbstractElement& el) {
        }
        AbstractElementPtr loadAll(std::string path) {
            return AbstractElementPtr();
        }
        AbstractElementPtr loadAll() {
            return AbstractElementPtr();
        }
        void saveAll(AbstractElement& root, std::string location) {}
        void saveAll(AbstractElement& root) {}
        void eraseEl(ID id) {}
};

using CreationStorageTestManager = Manager<TemplateTypeList<Base, Right, Left, Diamond>, CreationStoragePolicy>;

TEST_F(ManagerTest, CreationStorageTest) {
    CreationStorageTestManager m;
    auto element = m.create<Diamond>();
    ASSERT_EQ(m.last_created, element.id());
}
