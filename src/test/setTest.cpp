#include "gtest/gtest.h"
#include "egm/egm.h"

namespace EGM {
class SetTest : public ::testing::Test {
   
};

template <class ManagerPolicy>
class TestPackage : public ManagerPolicy {
    public:
        using Info = TypeInfo<TestPackage>;
        void init() {}
        MANAGED_ELEMENT_CONSTRUCTOR(TestPackage);
};

template <>
struct ElementInfo<TestPackage> {
    static std::string name() { return "TestPackage"; }
};

template <class ManagerPolicy>
class TestPackageSetElement : public ManagerPolicy  {
    public:
        Set<TestPackage, TestPackageSetElement> set = Set<TestPackage, TestPackageSetElement>(this);
        void init() {}
        using Info = TypeInfo<TestPackageSetElement>;
        MANAGED_ELEMENT_CONSTRUCTOR(TestPackageSetElement);
};

template<>
struct ElementInfo<TestPackageSetElement> {
    static std::string name() { return "TestPackageSetElement"; }
};

using BasicSetTestManager = Manager<TemplateTypeList<TestPackage, TestPackageSetElement>>;

TEST_F(SetTest, basicSetTest) {
    size_t numPackages = 20;
    BasicSetTestManager m;
    auto testEl = m.create<TestPackageSetElement>();
    auto& pckg = *m.create<TestPackage>();
    testEl->set.add(pckg);
    ASSERT_FALSE(testEl->set.empty());
    ASSERT_EQ(*testEl->set.get(pckg.getID()), pckg);
    std::vector<ID> ids(numPackages);
    ids[0] = pckg.getID();
    for (size_t i = 0; i < numPackages - 1; i++) {
        auto& p = *m.create<TestPackage>();
        ids[i+1] = p.getID();
        testEl->set.add(p);
    }
    size_t i = 0;
    for (const ID id : ids) {
        ASSERT_TRUE(testEl->set.contains(id)) << "index at " << i;
        ASSERT_EQ(testEl->set.get(id)->getID(), id);
        i++;
    }
}

TEST_F(SetTest, basicRemoveTest) {
    BasicSetTestManager m;
    auto testEl = m.create<TestPackageSetElement>();
    auto pckg1 = m.create<TestPackage>();
    auto pckg2 = m.create<TestPackage>();
    auto pckg3 = m.create<TestPackage>();
    auto pckg4 = m.create<TestPackage>();
    auto pckg5 = m.create<TestPackage>();
    testEl->set.add(pckg1);
    testEl->set.add(pckg2);
    testEl->set.add(pckg3);
    testEl->set.add(pckg4);
    testEl->set.add(pckg5);
    testEl->set.remove(pckg1);
    ASSERT_EQ(testEl->set.size(), 4);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_TRUE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_TRUE(testEl->set.contains(pckg4));
    ASSERT_TRUE(testEl->set.contains(pckg5));
    testEl->set.remove(pckg5);
    ASSERT_EQ(testEl->set.size(), 3);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_TRUE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_TRUE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    testEl->set.remove(pckg2);
    ASSERT_EQ(testEl->set.size(), 2);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_TRUE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    testEl->set.remove(pckg4);
    ASSERT_EQ(testEl->set.size(), 1);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    testEl->set.remove(pckg3);
    ASSERT_EQ(testEl->set.size(), 0);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_FALSE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
}

TEST_F(SetTest, consistentIdBasicRemoveTest) {
    BasicSetTestManager m;
    auto testEl = m.create<TestPackageSetElement>();
    auto pckg1 = m.create<TestPackage>();
    pckg1->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA1"));
    auto pckg2 = m.create<TestPackage>();
    pckg2->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA2"));
    auto pckg3 = m.create<TestPackage>();
    pckg3->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA3"));
    auto pckg4 = m.create<TestPackage>();
    pckg4->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA4"));
    auto pckg5 = m.create<TestPackage>();
    pckg5->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA5"));
    testEl->set.add(pckg1);
    testEl->set.add(pckg2);
    testEl->set.add(pckg3);
    testEl->set.add(pckg4);
    testEl->set.add(pckg5);
    testEl->set.remove(pckg1);
    ASSERT_EQ(testEl->set.size(), 4);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_TRUE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_TRUE(testEl->set.contains(pckg4));
    ASSERT_TRUE(testEl->set.contains(pckg5));
    testEl->set.remove(pckg5);
    ASSERT_EQ(testEl->set.size(), 3);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_TRUE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_TRUE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    testEl->set.remove(pckg2);
    ASSERT_EQ(testEl->set.size(), 2);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_TRUE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    testEl->set.remove(pckg4);
    ASSERT_EQ(testEl->set.size(), 1);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    testEl->set.remove(pckg3);
    ASSERT_EQ(testEl->set.size(), 0);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_FALSE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
}

TEST_F(SetTest, longerConsistentIdBasicRemoveTest) {
    BasicSetTestManager m;
    auto testEl = m.create<TestPackageSetElement>();
    auto pckg1 = m.create<TestPackage>();
    pckg1->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA1"));
    auto pckg2 = m.create<TestPackage>();
    pckg2->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA2"));
    auto pckg3 = m.create<TestPackage>();
    pckg3->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA3"));
    auto pckg4 = m.create<TestPackage>();
    pckg4->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA4"));
    auto pckg5 = m.create<TestPackage>();
    pckg5->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA5"));
    auto pckg6 = m.create<TestPackage>();
    pckg5->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA6"));
    auto pckg7 = m.create<TestPackage>();
    pckg5->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA7"));
    auto pckg8 = m.create<TestPackage>();
    pckg8->setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAA8"));
    testEl->set.add(pckg1);
    testEl->set.add(pckg2);
    testEl->set.add(pckg3);
    testEl->set.add(pckg4);
    testEl->set.add(pckg5);
    testEl->set.add(pckg6);
    testEl->set.add(pckg7);
    testEl->set.add(pckg8);
    testEl->set.remove(pckg1);
    ASSERT_EQ(testEl->set.size(), 7);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_TRUE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_TRUE(testEl->set.contains(pckg4));
    ASSERT_TRUE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_TRUE(testEl->set.contains(pckg7));
    ASSERT_TRUE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg5);
    ASSERT_EQ(testEl->set.size(), 6);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_TRUE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_TRUE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_TRUE(testEl->set.contains(pckg7));
    ASSERT_TRUE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg2);
    ASSERT_EQ(testEl->set.size(), 5);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_TRUE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_TRUE(testEl->set.contains(pckg7));
    ASSERT_TRUE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg4);
    ASSERT_EQ(testEl->set.size(), 4);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_TRUE(testEl->set.contains(pckg7));
    ASSERT_TRUE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg7);
    ASSERT_EQ(testEl->set.size(), 3);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_FALSE(testEl->set.contains(pckg7));
    ASSERT_TRUE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg3);
    ASSERT_EQ(testEl->set.size(), 2);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_FALSE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_FALSE(testEl->set.contains(pckg7));
    ASSERT_TRUE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg8);
    ASSERT_EQ(testEl->set.size(), 1);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_FALSE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_FALSE(testEl->set.contains(pckg7));
    ASSERT_FALSE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg6);
    ASSERT_EQ(testEl->set.size(), 0);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_FALSE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_FALSE(testEl->set.contains(pckg6));
    ASSERT_FALSE(testEl->set.contains(pckg7));
    ASSERT_FALSE(testEl->set.contains(pckg8));
}

TEST_F(SetTest, longerBasicRemoveTest) {
    BasicSetTestManager m;
    auto testEl = m.create<TestPackageSetElement>();
    auto pckg1 = m.create<TestPackage>();
    auto pckg2 = m.create<TestPackage>();
    auto pckg3 = m.create<TestPackage>();
    auto pckg4 = m.create<TestPackage>();
    auto pckg5 = m.create<TestPackage>();
    auto pckg6 = m.create<TestPackage>();
    auto pckg7 = m.create<TestPackage>();
    auto pckg8 = m.create<TestPackage>();
    testEl->set.add(pckg1);
    testEl->set.add(pckg2);
    testEl->set.add(pckg3);
    testEl->set.add(pckg4);
    testEl->set.add(pckg5);
    testEl->set.add(pckg6);
    testEl->set.add(pckg7);
    testEl->set.add(pckg8);
    testEl->set.remove(pckg1);
    ASSERT_EQ(testEl->set.size(), 7);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_TRUE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_TRUE(testEl->set.contains(pckg4));
    ASSERT_TRUE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_TRUE(testEl->set.contains(pckg7));
    ASSERT_TRUE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg5);
    ASSERT_EQ(testEl->set.size(), 6);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_TRUE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_TRUE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_TRUE(testEl->set.contains(pckg7));
    ASSERT_TRUE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg2);
    ASSERT_EQ(testEl->set.size(), 5);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_TRUE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_TRUE(testEl->set.contains(pckg7));
    ASSERT_TRUE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg4);
    ASSERT_EQ(testEl->set.size(), 4);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_TRUE(testEl->set.contains(pckg7));
    ASSERT_TRUE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg7);
    ASSERT_EQ(testEl->set.size(), 3);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_TRUE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_FALSE(testEl->set.contains(pckg7));
    ASSERT_TRUE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg3);
    ASSERT_EQ(testEl->set.size(), 2);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_FALSE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_FALSE(testEl->set.contains(pckg7));
    ASSERT_TRUE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg8);
    ASSERT_EQ(testEl->set.size(), 1);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_FALSE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_TRUE(testEl->set.contains(pckg6));
    ASSERT_FALSE(testEl->set.contains(pckg7));
    ASSERT_FALSE(testEl->set.contains(pckg8));
    testEl->set.remove(pckg6);
    ASSERT_EQ(testEl->set.size(), 0);
    ASSERT_FALSE(testEl->set.contains(pckg1));
    ASSERT_FALSE(testEl->set.contains(pckg2));
    ASSERT_FALSE(testEl->set.contains(pckg3));
    ASSERT_FALSE(testEl->set.contains(pckg4));
    ASSERT_FALSE(testEl->set.contains(pckg5));
    ASSERT_FALSE(testEl->set.contains(pckg6));
    ASSERT_FALSE(testEl->set.contains(pckg7));
    ASSERT_FALSE(testEl->set.contains(pckg8));
}

template <class ManagerPolicy>
class TestSubsetsElement : public ManagerPolicy {
    public:
        using Info = TypeInfo<TestSubsetsElement>;
        Set<TestPackage, TestSubsetsElement> root = Set<TestPackage, TestSubsetsElement>(this);
        Set<TestPackage, TestSubsetsElement> sub = Set<TestPackage, TestSubsetsElement>(this);
    private:
        void init() {
            sub.subsets(root);
        }
    public:
        MANAGED_ELEMENT_CONSTRUCTOR(TestSubsetsElement);
};

template <>
struct ElementInfo<TestSubsetsElement> {
    static std::string name() { return "TestSubsetsElement"; }
};

using TestSubsetsManager = Manager<TemplateTypeList<TestPackage, TestSubsetsElement>>;

TEST_F(SetTest, basicSubsetsTest) {
    TestSubsetsManager m;
    auto testEl = m.create<TestSubsetsElement>();
    auto& pckg = *m.create<TestPackage>();
    testEl->sub.add(pckg);
    ASSERT_EQ(testEl->sub.size(), 1);
    ASSERT_TRUE(testEl->sub.contains(pckg.getID()));
    ASSERT_EQ(*testEl->sub.get(pckg.getID()), pckg);
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_TRUE(testEl->root.contains(pckg.getID()));
    ASSERT_EQ(*testEl->root.get(pckg.getID()), pckg);
    auto& clazz = *m.create<TestPackage>();
    testEl->root.add(clazz);
    ASSERT_EQ(testEl->root.size(), 2);
    ASSERT_TRUE(testEl->root.contains(clazz.getID()));
    ASSERT_EQ(*testEl->root.get(clazz.getID()), clazz);
    ASSERT_EQ(testEl->sub.size(), 1);
    ASSERT_FALSE(testEl->sub.contains(clazz.getID()));
    ASSERT_FALSE(testEl->sub.get(clazz.getID()));
}

TEST_F(SetTest, iterateOverSubsettedElement) {
    TestSubsetsManager m;
    auto testEl = m.create<TestSubsetsElement>();
    auto a = m.create<TestPackage>();
    auto b = m.create<TestPackage>();
    auto c = m.create<TestPackage>();
    a->setID(ID::fromString("&90IAqvc&wUnewHz0xLI4fPYNXUe"));
    b->setID(ID::fromString("04zH_c&oPfM5KXPqJXA0_7AzIzcy"));
    c->setID(ID::fromString("buttEyLdYRCk_zbuttYrQyy42yHr")); 
    // inorder
    //
    // &90IAqvc&wUnewHz0xLI4fPYNXUe
    // buttEyLdYRCk_zbuttYrQyy42yHr
    // 04zH_c&oPfM5KXPqJXA0_7AzIzcy
    //
    // WwowBIeuOqdXecMITJkHZWbnD94G
    testEl->root.add(a);
    std::unordered_set<std::shared_ptr<TestPackage<TestSubsetsManager::GenBaseHierarchy<TestPackage>>>> pckgs;
    pckgs.insert(a.ptr());
    testEl->sub.add(b);
    pckgs.insert(b.ptr());
    testEl->sub.add(c);
    pckgs.insert(c.ptr());
    ASSERT_EQ(pckgs.size(), 3);
    auto it = testEl->root.begin();
    std::cout << "test setup done" << std::endl;
    std::cout << "first elment is " << it->getID().string() << std::endl;
    it++;
    std::cout << "incremented once!" << std::endl;
    std::cout << "second elment is " << it->getID().string() << std::endl;
    ASSERT_TRUE(it != testEl->root.end());
    it++;
    std::cout << "incremented twice!" << std::endl;
    std::cout << "third elment is " << it->getID().string() << std::endl;
    ASSERT_TRUE(it != testEl->root.end());
    it++;
    std::cout << "incremented three times (should be done)" << std::endl;
    ASSERT_EQ(it.m_hash, testEl->root.end().m_hash);
    ASSERT_EQ(it, testEl->root.end());
}

template <class ManagerPolicy>
class Test2SubsetsElement : public ManagerPolicy {
    public:
        Set<TestPackage, Test2SubsetsElement> set1 = Set<TestPackage, Test2SubsetsElement>(this);
        Set<TestPackage, Test2SubsetsElement> set2 = Set<TestPackage, Test2SubsetsElement>(this);
        Set<TestPackage, Test2SubsetsElement> sub = Set<TestPackage, Test2SubsetsElement>(this);
        void init() {
            sub.subsets(set1);
            sub.subsets(set2);
        }
        using Info = TypeInfo<Test2SubsetsElement>;
        MANAGED_ELEMENT_CONSTRUCTOR(Test2SubsetsElement);
};

template <>
struct ElementInfo<Test2SubsetsElement> {
    static std::string name() { return "Test2SubsetsElement"; }
};

using MultiSubsetsTestManager = Manager<TemplateTypeList<TestPackage, Test2SubsetsElement>>;

TEST_F(SetTest, multiSubsetsTest) {
    MultiSubsetsTestManager m;
    auto testEl = m.create<Test2SubsetsElement>();
    auto& clazz = *m.create<TestPackage>();
    testEl->set1.add(clazz);
    ASSERT_FALSE(testEl->set1.empty());
    ASSERT_EQ(testEl->set1.size(), 1);
    ASSERT_TRUE(testEl->set1.contains(clazz.getID()));
    ASSERT_TRUE(testEl->set2.empty());
    ASSERT_EQ(testEl->set2.size(), 0);
    ASSERT_FALSE(testEl->set2.contains(clazz.getID()));
    ASSERT_TRUE(testEl->sub.empty());
    ASSERT_EQ(testEl->sub.size(), 0);
    ASSERT_FALSE(testEl->sub.contains(clazz.getID()));
    auto& inst = *m.create<TestPackage>();
    testEl->set2.add(inst);
    ASSERT_FALSE(testEl->set1.empty());
    ASSERT_EQ(testEl->set1.size(), 1);
    ASSERT_FALSE(testEl->set1.contains(inst.getID()));
    ASSERT_FALSE(testEl->set2.empty());
    ASSERT_EQ(testEl->set2.size(), 1);
    ASSERT_TRUE(testEl->set2.contains(inst.getID()));
    ASSERT_TRUE(testEl->sub.empty());
    ASSERT_EQ(testEl->sub.size(), 0);
    ASSERT_FALSE(testEl->sub.contains(inst.getID()));
    auto& pckg = *m.create<TestPackage>();
    testEl->sub.add(pckg);
    ASSERT_FALSE(testEl->set1.empty());
    ASSERT_EQ(testEl->set1.size(), 2);
    ASSERT_TRUE(testEl->set1.contains(pckg.getID()));
    ASSERT_FALSE(testEl->set2.empty());
    ASSERT_EQ(testEl->set2.size(), 2);
    ASSERT_TRUE(testEl->set2.contains(pckg.getID()));
    ASSERT_FALSE(testEl->sub.empty());
    ASSERT_EQ(testEl->sub.size(), 1);
    ASSERT_TRUE(testEl->sub.contains(pckg.getID()));
}

template <class ManagerPolicy>
class Test3SubsetsElement : public ManagerPolicy {
    public:
        Set<TestPackage, Test3SubsetsElement> root = Set<TestPackage, Test3SubsetsElement>(this);
        Set<TestPackage, Test3SubsetsElement> intermediate = Set<TestPackage, Test3SubsetsElement>(this);
        Set<TestPackage, Test3SubsetsElement> sub = Set<TestPackage, Test3SubsetsElement>(this);
        void init() {
            intermediate.subsets(root);
            sub.subsets(intermediate);
        }
        using Info = TypeInfo<Test3SubsetsElement>;
        MANAGED_ELEMENT_CONSTRUCTOR(Test3SubsetsElement);
};

template <>
struct ElementInfo<Test3SubsetsElement> {
    static std::string name() { return "Test3SubsetsElement"; }
};

using Test3SubsetsManager = Manager<TemplateTypeList<TestPackage, Test3SubsetsElement>>;

TEST_F(SetTest, removeFromSubsettedSequenceTest) {
    Test3SubsetsManager m;
    auto testEl = m.create<Test3SubsetsElement>();
    auto& pckg = *m.create<TestPackage>();
    testEl->sub.add(pckg);
    std::cout << "about to remove element" << std::endl;
    testEl->sub.remove(pckg);
    std::cout << "removed element" << std::endl;
    ASSERT_EQ(testEl->sub.size(), 0);
    ASSERT_EQ(testEl->intermediate.size(), 0);
    ASSERT_EQ(testEl->root.size(), 0);
    ASSERT_FALSE(testEl->sub.contains(pckg));
    ASSERT_FALSE(testEl->intermediate.contains(pckg));
    ASSERT_FALSE(testEl->root.contains(pckg));
    testEl->sub.add(pckg);
    std::cout << "about to remove element" << std::endl;
    testEl->intermediate.remove(pckg);
    std::cout << "removed element" << std::endl;
    ASSERT_EQ(testEl->sub.size(), 0);
    ASSERT_EQ(testEl->intermediate.size(), 0);
    ASSERT_EQ(testEl->root.size(), 0);
    ASSERT_FALSE(testEl->sub.contains(pckg));
    ASSERT_FALSE(testEl->intermediate.contains(pckg));
    ASSERT_FALSE(testEl->root.contains(pckg));
}

TEST_F(SetTest, addToSetTwice) {
    BasicSetTestManager m;
    auto testEl = m.create<TestPackageSetElement>();
    auto& p = *m.create<TestPackage>();
    testEl->set.add(p);
    ASSERT_THROW(testEl->set.add(p), SetStateException);
}

template <class ManagerPolicy>
class TestElement2 : public ManagerPolicy {
    public:
        using Info = TypeInfo<TestElement2>;
        using OppositeSet = Set<TestElement2, TestElement2>; 
        OppositeSet m_others = OppositeSet(this);
        OppositeSet& getOthers() { return m_others; }
        void init() {
            m_others.opposite(&TestElement2::getOthers);
        }
        MANAGED_ELEMENT_CONSTRUCTOR(TestElement2);
};

template <>
struct ElementInfo<TestElement2> {
    static std::string name() { return "TestElement2"; }
};

using TestElement2Manager = Manager<TemplateTypeList<TestElement2>>;

TEST_F(SetTest, oppositeTest) {
    TestElement2Manager m;
    auto& t1 = *m.create<TestElement2>();
    auto& t2 = *m.create<TestElement2>();
    t1.getOthers().add(t2);
    ASSERT_EQ(t2.getOthers().size(), 1);
    ASSERT_TRUE(t2.getOthers().contains(t1.getID()));
}

template <class ManagerPolicy>
class RedefinedTestElement : public ManagerPolicy {
    public:
        Set<TestPackage, RedefinedTestElement> rootSet = Set<TestPackage, RedefinedTestElement>(this);
        Set<TestPackage, RedefinedTestElement> redefiningSet = Set<TestPackage, RedefinedTestElement>(this);
        void init() {
            redefiningSet.redefines(rootSet);
        }
        using Info = TypeInfo<RedefinedTestElement>;
        MANAGED_ELEMENT_CONSTRUCTOR(RedefinedTestElement);
};

template <>
struct ElementInfo<RedefinedTestElement> {
    static std::string name() { return "RedefinedTestElement"; }
};

using RedefinedTestElementManager = Manager<TemplateTypeList<TestPackage, RedefinedTestElement>>;

TEST_F(SetTest, setRedefinesTest) {
    RedefinedTestElementManager m;
    auto testEl = m.create<RedefinedTestElement>();
    auto& p = *m.create<TestPackage>();
    testEl->redefiningSet.add(p);
    ASSERT_FALSE(testEl->rootSet.empty());
    ASSERT_FALSE(testEl->redefiningSet.empty());
    ASSERT_EQ(testEl->rootSet.size(), 1);
    ASSERT_EQ(testEl->rootSet.size(), 1);
    testEl->redefiningSet.remove(p);
    ASSERT_TRUE(testEl->rootSet.empty());
    ASSERT_TRUE(testEl->redefiningSet.empty());
    ASSERT_EQ(testEl->rootSet.size(), 0);
    ASSERT_EQ(testEl->redefiningSet.size(), 0);
    auto& p2 = *m.create<TestPackage>();
    testEl->rootSet.add(p);
    ASSERT_FALSE(testEl->rootSet.empty());
    ASSERT_FALSE(testEl->redefiningSet.empty());
    ASSERT_EQ(testEl->rootSet.size(), 1);
    ASSERT_EQ(testEl->redefiningSet.size(), 1);
    testEl->rootSet.remove(p);
    ASSERT_TRUE(testEl->rootSet.empty());
    ASSERT_TRUE(testEl->redefiningSet.empty());
    ASSERT_EQ(testEl->rootSet.size(), 0);
    ASSERT_EQ(testEl->redefiningSet.size(), 0);
    testEl->rootSet.add(p);
    testEl->redefiningSet.add(p2);
    ASSERT_FALSE(testEl->rootSet.empty());
    ASSERT_FALSE(testEl->redefiningSet.empty());
    ASSERT_EQ(testEl->rootSet.size(), 2);
    ASSERT_EQ(testEl->redefiningSet.size(), 2);
    auto& p3 = *m.create<TestPackage>();
    testEl->rootSet.add(p3);
    ASSERT_FALSE(testEl->rootSet.empty());
    ASSERT_FALSE(testEl->redefiningSet.empty());
    ASSERT_EQ(testEl->rootSet.size(), 3);
    ASSERT_EQ(testEl->redefiningSet.size(), 3);
}

template <class ManagerPolicy>
class PolicyTestElement : public ManagerPolicy {
    public:
        struct TestPolicy;
        struct TestPolicy {
            void elementAdded(TestPackage<typename ManagerPolicy::manager::template GenBaseHierarchy<TestPackage>>& el, PolicyTestElement& me) {
                me.count++;
            }
            void elementRemoved(TestPackage<typename ManagerPolicy::manager::template GenBaseHierarchy<TestPackage>>& el, PolicyTestElement& me) {
                me.count--;
            }
        };        
        Set<TestPackage, PolicyTestElement, TestPolicy> policySet = Set<TestPackage, PolicyTestElement, TestPolicy>(this);
        Set<TestPackage, PolicyTestElement> redefinedSet = Set<TestPackage, PolicyTestElement>(this);
        void init() {
            redefinedSet.redefines(policySet);
        }
        size_t count = 0;
        using Info = TypeInfo<PolicyTestElement>;
        MANAGED_ELEMENT_CONSTRUCTOR(PolicyTestElement);
};

template <>
struct ElementInfo<PolicyTestElement> {
    static std::string name() { return "PolicyTestElement"; }
};

using PolicyTestElementManager = Manager<TemplateTypeList<TestPackage, PolicyTestElement>>;

TEST_F(SetTest, setRedefinedWFunctors) {
    PolicyTestElementManager m;
    auto testEl = m.create<PolicyTestElement>();
    auto& g = *m.create<TestPackage>();
    testEl->redefinedSet.add(g);
    ASSERT_EQ(testEl->count, 1);
    testEl->redefinedSet.remove(g);
    ASSERT_EQ(testEl->count, 0);
}

template <class ManagerPolicy>
class TestOrderedSetElement : public ManagerPolicy {
    public:
        using Info = TypeInfo<TestOrderedSetElement>;
        OrderedSet<TestPackage, TestOrderedSetElement> set = OrderedSet<TestPackage, TestOrderedSetElement>(this);
    private:
        void init() {}
    public:
        MANAGED_ELEMENT_CONSTRUCTOR(TestOrderedSetElement);
};

template <>
struct ElementInfo<TestOrderedSetElement> {
    static std::string name() { return "TestOrderedSetElement"; }
};

using TestOrderedSetElementManager = Manager<TemplateTypeList<TestPackage, TestOrderedSetElement>>;

TEST_F(SetTest, addToOrderedSetTest) {
    TestOrderedSetElementManager m;
    auto testEl = m.create<TestOrderedSetElement>();
    auto p = m.create<TestPackage>();
    testEl->set.add(p);
    ASSERT_EQ(testEl->set.front(), p);
    ASSERT_EQ(testEl->set.back(), p);
    auto p2 = m.create<TestPackage>();
    testEl->set.add(p2);
    ASSERT_EQ(testEl->set.front(), p);
    ASSERT_EQ(testEl->set.back(), p2);
    auto p3 = m.create<TestPackage>();
    testEl->set.add(p3);
    ASSERT_EQ(testEl->set.front(), p);
    ASSERT_EQ(testEl->set.back(), p3);
    ASSERT_TRUE(testEl->set.contains(p.id()));
    // ASSERT_TRUE(testEl->set.contains("1"));
    ASSERT_EQ(testEl->set.get(p.id()), p);
    // ASSERT_EQ(testEl->set.get("1"), p);
    ASSERT_TRUE(testEl->set.contains(p2.id()));
    // ASSERT_TRUE(testEl->set.contains("2"));
    ASSERT_EQ(testEl->set.get(p2.id()), p2);
    // ASSERT_EQ(testEl->set.get("2"), p2);
    ASSERT_TRUE(testEl->set.contains(p3.id()));
    // ASSERT_TRUE(testEl->set.contains("3"));
    ASSERT_EQ(testEl->set.get(p3.id()), p3);
    // ASSERT_EQ(testEl->set.get("3"), p3);
    std::array<decltype(p), 3> ps = {p, p2, p3};
    int i = 0;
    for (auto& pckg : testEl->set) {
        ASSERT_EQ(*ps[i], pckg);
        i++;
    }
    ASSERT_EQ(testEl->set.get(0), p);
    ASSERT_EQ(testEl->set.get(1), p2);
    ASSERT_EQ(testEl->set.get(2), p3);
}

template <class ManagerPolicy>
class TestElementSubsetsOrderedSets : public ManagerPolicy {
    public:
        using Info = TypeInfo<TestElementSubsetsOrderedSets>;
        OrderedSet<TestPackage, TestElementSubsetsOrderedSets> root = OrderedSet<TestPackage, TestElementSubsetsOrderedSets>(this);
        OrderedSet<TestPackage, TestElementSubsetsOrderedSets> sub = OrderedSet<TestPackage, TestElementSubsetsOrderedSets>(this);
    private:
        void init() {
            sub.subsets(root);
        }
    public:
        MANAGED_ELEMENT_CONSTRUCTOR(TestElementSubsetsOrderedSets);
};

template <>
struct ElementInfo<TestElementSubsetsOrderedSets> {
    static std::string name() { return "TestElementSubsetsOrderedSets"; }
};

using SubsetsOrderedSetsManager = Manager<TemplateTypeList<TestPackage, TestElementSubsetsOrderedSets>>;


TEST_F(SetTest, subsetOrderedSets) {
    SubsetsOrderedSetsManager m;
    auto testEl = m.create<TestElementSubsetsOrderedSets>();
    auto& p = *m.create<TestPackage>();
    testEl->sub.add(p);
    ASSERT_EQ(*testEl->sub.front(), p);
    ASSERT_FALSE(testEl->root.empty());
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_EQ(*testEl->root.front(), p);
    auto& c = *m.create<TestPackage>();
    testEl->root.add(c);
    ASSERT_FALSE(testEl->sub.contains(c.getID()));
    ASSERT_TRUE(testEl->root.contains(c.getID()));
    ASSERT_EQ(*testEl->root.back(), c);
    int i = 0;
    for (__attribute__((unused)) auto& pckg : testEl->sub) {
        i++;
    }
    ASSERT_EQ(i, 1);
    i = 0;
    for (__attribute__((unused)) auto& pckg : testEl->root) {
        i++;
    }
    ASSERT_EQ(i, 2);
}

template <class ManagerPolicy>
class TestElementOrderedSubsetsSet : public ManagerPolicy {
    public:
        using Info = TypeInfo<TestElementOrderedSubsetsSet>;
        Set<TestPackage, TestElementOrderedSubsetsSet> root = Set<TestPackage, TestElementOrderedSubsetsSet>(this);
        OrderedSet<TestPackage, TestElementOrderedSubsetsSet> sub = OrderedSet<TestPackage, TestElementOrderedSubsetsSet>(this);
    private:
        void init() {
            sub.subsets(root);
        }
    public:
        MANAGED_ELEMENT_CONSTRUCTOR(TestElementOrderedSubsetsSet);
};

template <>
struct ElementInfo<TestElementOrderedSubsetsSet> {
    static std::string name() { return "TestElementOrderedSubsetsSet"; }
};

using ElementOrderedSubsetsSetManager = Manager<TemplateTypeList<TestPackage, TestElementOrderedSubsetsSet>>;

TEST_F(SetTest, orderedSetSubSetsSet) {
    ElementOrderedSubsetsSetManager m;
    auto testEl = m.create<TestElementOrderedSubsetsSet>();
    auto& p = *m.create<TestPackage>();
    testEl->sub.add(p);
    ASSERT_EQ(*testEl->sub.front(), p);
    ASSERT_FALSE(testEl->root.empty());
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_EQ(*testEl->root.get(p.getID()), p);
    auto& c = *m.create<TestPackage>();
    testEl->root.add(c);
    ASSERT_FALSE(testEl->sub.contains(c.getID()));
    ASSERT_TRUE(testEl->root.contains(c.getID()));
    ASSERT_EQ(*testEl->root.get(c.getID()), c);
    int i = 0;
    for (__attribute__((unused)) auto& pckg : testEl->sub) {
        i++;
    }
    ASSERT_EQ(i, 1);
    i = 0;
    for (__attribute__((unused)) auto& pckg : testEl->root) {
        i++;
    }
    ASSERT_EQ(i, 2);
}

template <class ManagerPolicy>
class TestSingletonElement : public ManagerPolicy {
    public:
        using Info = TypeInfo<TestSingletonElement>;
        Set<TestPackage, TestSingletonElement> root = Set<TestPackage, TestSingletonElement>(this);
        Singleton<TestPackage, TestSingletonElement> singleton = Singleton<TestPackage, TestSingletonElement>(this);
    private:
        void init() {
            singleton.subsets(root);
        }
    public:
        MANAGED_ELEMENT_CONSTRUCTOR(TestSingletonElement);
};

template <>
struct ElementInfo<TestSingletonElement> {
    static std::string name() { return "TestSingletonElement"; }
};

using TestSingletonManager = Manager<TemplateTypeList<TestPackage, TestSingletonElement>>;

TEST_F(SetTest, singletonTest) {
    TestSingletonManager m;
    auto testEl = m.create<TestSingletonElement>();
    ASSERT_TRUE(testEl->singleton.empty());
    ASSERT_EQ(testEl->singleton.size(), 0);
    auto& p = *m.create<TestPackage>();
    testEl->singleton.set(p);
    ASSERT_EQ(testEl->singleton.size(), 1);
    ASSERT_FALSE(testEl->singleton.empty());
    ASSERT_TRUE(testEl->singleton.get());
    ASSERT_EQ(*testEl->singleton.get(), p);
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_FALSE(testEl->root.empty());
    ASSERT_EQ(*testEl->root.get(p.getID()), p);
    auto& c = *m.create<TestPackage>();
    testEl->root.add(c);
    ASSERT_EQ(testEl->singleton.size(), 1);
    ASSERT_FALSE(testEl->singleton.empty());
    ASSERT_TRUE(testEl->singleton.get());
    ASSERT_EQ(*testEl->singleton.get(), p);
    ASSERT_EQ(testEl->root.size(), 2);
    ASSERT_TRUE(testEl->root.contains(p.getID()));
    ASSERT_TRUE(testEl->root.contains(c.getID()));
    ASSERT_EQ(*testEl->root.get(p.getID()), p);
    ASSERT_EQ(*testEl->root.get(c.getID()), c);
    testEl->singleton.set(0);
    ASSERT_FALSE(testEl->singleton.get());
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_TRUE(testEl->root.contains(c.getID()));
    ASSERT_FALSE(testEl->root.contains(p.getID()));
}

template <class ManagerPolicy>
class TestSharedSubsetEvenTreeElement : public ManagerPolicy {
    public:
        using Info = TypeInfo<TestSharedSubsetEvenTreeElement>;
        Set<TestPackage, TestSharedSubsetEvenTreeElement> root = Set<TestPackage, TestSharedSubsetEvenTreeElement>(this);
        Set<TestPackage, TestSharedSubsetEvenTreeElement> set1 = Set<TestPackage, TestSharedSubsetEvenTreeElement>(this);
        Set<TestPackage, TestSharedSubsetEvenTreeElement> set2 = Set<TestPackage, TestSharedSubsetEvenTreeElement>(this);
    private:
        void init() {
            set1.subsets(root);
            set2.subsets(root);
        }
    public:
        MANAGED_ELEMENT_CONSTRUCTOR(TestSharedSubsetEvenTreeElement);
};

template <>
struct ElementInfo<TestSharedSubsetEvenTreeElement> {
    static std::string name() { return "TestSharedSubsetsEvenTreeElement"; }
};

using SharedSubsetEvenTreeManager = Manager<TemplateTypeList<TestPackage, TestSharedSubsetEvenTreeElement>>;

TEST_F(SetTest, sharedSubsetEvenTreeTest) {
    SharedSubsetEvenTreeManager m;
    auto testEl = m.create<TestSharedSubsetEvenTreeElement>();
    auto& p1 = *m.create<TestPackage>();
    auto& p2 = *m.create<TestPackage>();
    testEl->set1.add(p1);
    testEl->set2.add(p2);
    std::cout << "test setup finished" << std::endl;
    ASSERT_FALSE(testEl->set1.contains(p2.getID()));
    std::cout << "set1 front: " << testEl->set1.front().id().string() << std::endl;
    ASSERT_EQ(*testEl->set1.front(), p1);
    std::cout << "set2 front: " << testEl->set2.front().id().string() << std::endl;
    ASSERT_EQ(*testEl->set2.front(), p2);
    std::cout << "root front: " << testEl->set2.front().id().string() << std::endl;
    std::cout << "root begin: " << testEl->root.begin()->getID().string() << std::endl;
    ASSERT_EQ(*testEl->root.front(), *testEl->root.begin());
    for (auto& el : testEl->root) {
        std::cout << "current iteration: " << el.getID().string() << std::endl;
        ASSERT_TRUE(testEl->root.contains(el.getID()));
    }
    std::cout << "done with regular iterator" << std::endl;
    for (const ID id : testEl->root.ids()) {
        std::cout << "current iteration: " << id.string() << std::endl;
        ASSERT_TRUE(testEl->root.contains(id));
    }
    std::cout << "done with id iterator" << std::endl;
    testEl->set1.remove(p1);
    ASSERT_EQ(testEl->set2.size(), 1);
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_EQ(*testEl->root.front(), p2);
    ASSERT_EQ(*testEl->set2.front(), p2);
    testEl->set1.add(p1);
    ASSERT_FALSE(testEl->set1.contains(p2.getID()));
    ASSERT_EQ(*testEl->set1.front(), p1);
    ASSERT_EQ(*testEl->set2.front(), p2);
    for (auto& el : testEl->root) {
        ASSERT_TRUE(testEl->root.contains(el.getID()));
    }
    for (const ID id : testEl->root.ids()) {
        ASSERT_TRUE(testEl->root.contains(id));
    }
    testEl->set2.remove(p2);
    ASSERT_EQ(testEl->set1.size(), 1);
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_EQ(*testEl->root.front(), p1);
    ASSERT_EQ(*testEl->set1.front(), p1);
}

template <class ManagerPolicy>
class TestTwoRootSubSetElement : public ManagerPolicy {
    public:
        using Info = TypeInfo<TestTwoRootSubSetElement>;
        Set<TestPackage, TestTwoRootSubSetElement> root1 = Set<TestPackage, TestTwoRootSubSetElement>(this);
        Set<TestPackage, TestTwoRootSubSetElement> root2 = Set<TestPackage, TestTwoRootSubSetElement>(this);
        Set<TestPackage, TestTwoRootSubSetElement> sub = Set<TestPackage, TestTwoRootSubSetElement>(this);
    private:
        void init() {
            sub.subsets(root1);
            sub.subsets(root2);
        }
    public:
        MANAGED_ELEMENT_CONSTRUCTOR(TestTwoRootSubSetElement);
};

template <>
struct ElementInfo<TestTwoRootSubSetElement> {
    static std::string name() { return "TestTwoRootSubSetElement"; }
};

using TestTwoRootSubSetManager = Manager<TemplateTypeList<TestPackage, TestTwoRootSubSetElement>>;

TEST_F(SetTest, multiRootWithinRootTest) {
    TestTwoRootSubSetManager m;
    auto testEl = m.create<TestTwoRootSubSetElement>();
    auto& c = *m.create<TestPackage>();
    auto& p = *m.create<TestPackage>();
    testEl->root1.add(c);
    testEl->sub.add(p);
    ASSERT_EQ(testEl->root1.size(), 2);
    ASSERT_EQ(testEl->root2.size(), 1);
    ASSERT_EQ(testEl->sub.size(), 1);
    ASSERT_TRUE(testEl->root1.contains(c.getID()));
    ASSERT_FALSE(testEl->root2.contains(c.getID()));
    ASSERT_FALSE(testEl->sub.contains(c.getID()));
    ASSERT_TRUE(testEl->root1.contains(p.getID()));
    ASSERT_TRUE(testEl->root2.contains(p.getID()));
    ASSERT_TRUE(testEl->sub.contains(p.getID()));
}

TEST_F(SetTest, multiSubsetsOneElement) {
    TestTwoRootSubSetManager m;
    auto testEl = m.create<TestTwoRootSubSetElement>();
    auto& p = *m.create<TestPackage>();
    testEl->sub.add(p);
    ASSERT_EQ(testEl->sub.size(), 1);
    ASSERT_EQ(testEl->root1.size(), 1);
    ASSERT_EQ(testEl->root2.size(), 1);
    ASSERT_TRUE(testEl->sub.contains(p.getID()));
    ASSERT_TRUE(testEl->root2.contains(p.getID()));
    ASSERT_TRUE(testEl->root1.contains(p.getID()));
}

TEST_F(SetTest, multiRootAddToSubsetTest) {
    TestTwoRootSubSetManager m;
    auto testEl = m.create<TestTwoRootSubSetElement>();
    auto pckg = m.create<TestPackage>();
    testEl->root1.add(pckg);
    ASSERT_EQ(testEl->sub.size(), 0);
    ASSERT_EQ(testEl->root1.size(), 1);
    ASSERT_EQ(testEl->root2.size(), 0);
    ASSERT_FALSE(testEl->sub.contains(pckg.id()));
    ASSERT_FALSE(testEl->root2.contains(pckg.id()));
    ASSERT_TRUE(testEl->root1.contains(pckg.id()));
    testEl->sub.add(pckg);
    ASSERT_EQ(testEl->sub.size(), 1);
    ASSERT_EQ(testEl->root1.size(), 1);
    ASSERT_EQ(testEl->root2.size(), 1);
    ASSERT_TRUE(testEl->sub.contains(pckg.id()));
    ASSERT_TRUE(testEl->root2.contains(pckg.id()));
    ASSERT_TRUE(testEl->root1.contains(pckg.id()));
}

template <class ManagerPolicy>
class TestComplexSubsetElement : public ManagerPolicy {
    public:
        using Info = TypeInfo<TestComplexSubsetElement>;
        Set<TestPackage, TestComplexSubsetElement> rootSet = Set<TestPackage, TestComplexSubsetElement>(this);
        Set<TestPackage, TestComplexSubsetElement> rightSet1 = Set<TestPackage, TestComplexSubsetElement>(this);
        Set<TestPackage, TestComplexSubsetElement> leftSet1 = Set<TestPackage, TestComplexSubsetElement>(this);
        Set<TestPackage, TestComplexSubsetElement> rightSet2 = Set<TestPackage, TestComplexSubsetElement>(this);
        Set<TestPackage, TestComplexSubsetElement> leftSet2 = Set<TestPackage, TestComplexSubsetElement>(this);
        Set<TestPackage, TestComplexSubsetElement> leftSet2a = Set<TestPackage, TestComplexSubsetElement>(this);
        Set<TestPackage, TestComplexSubsetElement> rightSet2a = Set<TestPackage, TestComplexSubsetElement>(this);
    private:
        void init() {
            rightSet1.subsets(rootSet);
            leftSet1.subsets(rootSet);
            rightSet2.subsets(rightSet1);
            leftSet2.subsets(leftSet1);
            rightSet2a.subsets(rightSet1);
            leftSet2a.subsets(leftSet1);
        }
    public:
        MANAGED_ELEMENT_CONSTRUCTOR(TestComplexSubsetElement);
};

template <>
struct ElementInfo<TestComplexSubsetElement> {
    static std::string name() { return "TestComplexSubsetElement"; }
};

using ComplexSubsetManager = Manager<TemplateTypeList<TestPackage, TestComplexSubsetElement>>;


TEST_F(SetTest, twoWayMultiSetSplitTest) {
    ComplexSubsetManager m;
    auto testEl = m.create<TestComplexSubsetElement>();
    auto& rightP = *m.create<TestPackage>();
    auto& leftP = *m.create<TestPackage>();
    testEl->rightSet2.add(rightP);
    testEl->leftSet2.add(leftP);
    ASSERT_EQ(testEl->rightSet2.size(), 1);
    ASSERT_EQ(testEl->rightSet1.size(), 1);
    ASSERT_EQ(testEl->leftSet2.size(), 1);
    ASSERT_EQ(testEl->leftSet1.size(), 1);
    ASSERT_EQ(testEl->rootSet.size(), 2);
    ASSERT_TRUE(testEl->rightSet2.contains(rightP.getID()));
    ASSERT_TRUE(testEl->rightSet1.contains(rightP.getID()));
    ASSERT_TRUE(testEl->rootSet.contains(rightP.getID()));
    ASSERT_TRUE(testEl->leftSet1.contains(leftP.getID()));
    ASSERT_TRUE(testEl->leftSet2.contains(leftP.getID()));
    ASSERT_TRUE(testEl->rootSet.contains(leftP.getID()));
    ASSERT_FALSE(testEl->rightSet2.contains(leftP.getID()));
    ASSERT_FALSE(testEl->rightSet1.contains(leftP.getID()));
    ASSERT_FALSE(testEl->leftSet2.contains(rightP.getID()));
    ASSERT_FALSE(testEl->leftSet1.contains(rightP.getID()));
    ASSERT_FALSE(testEl->rightSet2a.contains(rightP.getID()));
    ASSERT_FALSE(testEl->rightSet2a.contains(leftP.getID()));
    ASSERT_FALSE(testEl->leftSet2a.contains(rightP.getID()));
    ASSERT_FALSE(testEl->leftSet2a.contains(leftP.getID()));

    auto& rightP2 = *m.create<TestPackage>();
    auto& leftP2 = *m.create<TestPackage>();
    testEl->rightSet1.add(rightP2);
    ASSERT_EQ(testEl->rightSet2.size(), 1);
    ASSERT_EQ(testEl->rightSet1.size(), 2);
    ASSERT_EQ(testEl->leftSet2.size(), 1);
    ASSERT_EQ(testEl->leftSet1.size(), 1);
    ASSERT_EQ(testEl->rootSet.size(), 3);
    ASSERT_FALSE(testEl->rightSet2.contains(rightP2.getID()));
    ASSERT_TRUE(testEl->rightSet1.contains(rightP2.getID()));
    ASSERT_TRUE(testEl->rootSet.contains(rightP2.getID()));
    ASSERT_FALSE(testEl->rightSet2a.contains(rightP2.getID()));
    ASSERT_FALSE(testEl->leftSet1.contains(rightP2.getID()));
    ASSERT_FALSE(testEl->leftSet2.contains(rightP2.getID()));
    ASSERT_FALSE(testEl->leftSet2a.contains(rightP2.getID()));
    ASSERT_TRUE(testEl->rightSet2.contains(rightP.getID()));
    ASSERT_TRUE(testEl->rightSet1.contains(rightP.getID()));
    ASSERT_TRUE(testEl->rootSet.contains(rightP.getID()));
    ASSERT_TRUE(testEl->leftSet1.contains(leftP.getID()));
    ASSERT_TRUE(testEl->leftSet2.contains(leftP.getID()));
    ASSERT_TRUE(testEl->rootSet.contains(leftP.getID()));
    ASSERT_FALSE(testEl->rightSet2.contains(leftP.getID()));
    ASSERT_FALSE(testEl->rightSet1.contains(leftP.getID()));
    ASSERT_FALSE(testEl->leftSet2.contains(rightP.getID()));
    ASSERT_FALSE(testEl->leftSet1.contains(rightP.getID()));
    ASSERT_FALSE(testEl->rightSet2a.contains(rightP.getID()));
    ASSERT_FALSE(testEl->rightSet2a.contains(leftP.getID()));
    ASSERT_FALSE(testEl->leftSet2a.contains(rightP.getID()));
    ASSERT_FALSE(testEl->leftSet2a.contains(leftP.getID()));

    testEl->leftSet1.add(leftP2);
    ASSERT_EQ(testEl->rootSet.size(), 4);
    ASSERT_EQ(testEl->rightSet1.size(), 2);
    ASSERT_EQ(testEl->leftSet1.size(), 2);
    ASSERT_EQ(testEl->rightSet2.size(), 1);
    ASSERT_EQ(testEl->leftSet2.size(), 1);
    ASSERT_TRUE(testEl->rootSet.contains(rightP2));
    ASSERT_TRUE(testEl->rightSet1.contains(rightP2));
    ASSERT_FALSE(testEl->rightSet2.contains(rightP2));
    ASSERT_FALSE(testEl->leftSet1.contains(rightP2));
    ASSERT_FALSE(testEl->leftSet2.contains(rightP2));
    ASSERT_TRUE(testEl->rootSet.contains(leftP2));
    ASSERT_TRUE(testEl->leftSet1.contains(leftP2));
    ASSERT_FALSE(testEl->leftSet2.contains(leftP2));
    ASSERT_FALSE(testEl->rightSet1.contains(leftP2));
    ASSERT_FALSE(testEl->rightSet2.contains(leftP2));
}

TEST_F(SetTest, complexDividerNodeTest) {
    ComplexSubsetManager m;
    auto testEl = m.create<TestComplexSubsetElement>();
    auto a = m.create<TestPackage>();
    auto b = m.create<TestPackage>();
    auto c = m.create<TestPackage>();
    auto d = m.create<TestPackage>();
    
    testEl->rightSet2.add(a);
    ASSERT_EQ(testEl->rootSet.size(), 1);
    ASSERT_TRUE(testEl->rootSet.contains(a));
    ASSERT_FALSE(testEl->rootSet.contains(b));
    ASSERT_FALSE(testEl->rootSet.contains(c));
    ASSERT_FALSE(testEl->rootSet.contains(d));
    ASSERT_EQ(testEl->rightSet1.size(), 1);
    ASSERT_TRUE(testEl->rightSet1.contains(a));
    ASSERT_FALSE(testEl->rightSet1.contains(b));
    ASSERT_FALSE(testEl->rightSet1.contains(c));
    ASSERT_FALSE(testEl->rightSet1.contains(d));
    ASSERT_EQ(testEl->rightSet2.size(), 1);
    ASSERT_TRUE(testEl->rightSet2.contains(a));
    ASSERT_FALSE(testEl->rightSet2.contains(b));
    ASSERT_FALSE(testEl->rightSet2.contains(c));
    ASSERT_FALSE(testEl->rightSet2.contains(d));
    ASSERT_EQ(testEl->rightSet2a.size(), 0);
    ASSERT_FALSE(testEl->rightSet2a.contains(a));
    ASSERT_FALSE(testEl->rightSet2a.contains(b));
    ASSERT_FALSE(testEl->rightSet2a.contains(c));
    ASSERT_FALSE(testEl->rightSet2a.contains(d));
    ASSERT_EQ(testEl->leftSet1.size(), 0);
    ASSERT_FALSE(testEl->leftSet1.contains(a));
    ASSERT_FALSE(testEl->leftSet1.contains(b));
    ASSERT_FALSE(testEl->leftSet1.contains(c));
    ASSERT_FALSE(testEl->leftSet1.contains(d));
    ASSERT_EQ(testEl->leftSet2.size(), 0);
    ASSERT_FALSE(testEl->leftSet2.contains(a));
    ASSERT_FALSE(testEl->leftSet2.contains(b));
    ASSERT_FALSE(testEl->leftSet2.contains(c));
    ASSERT_FALSE(testEl->leftSet2.contains(d));
    ASSERT_EQ(testEl->leftSet2a.size(), 0);
    ASSERT_FALSE(testEl->leftSet2a.contains(a));
    ASSERT_FALSE(testEl->leftSet2a.contains(b));
    ASSERT_FALSE(testEl->leftSet2a.contains(c));
    ASSERT_FALSE(testEl->leftSet2a.contains(d));

    testEl->rightSet2a.add(b);
    ASSERT_EQ(testEl->rootSet.size(), 2);
    ASSERT_TRUE(testEl->rootSet.contains(a));
    ASSERT_TRUE(testEl->rootSet.contains(b));
    ASSERT_FALSE(testEl->rootSet.contains(c));
    ASSERT_FALSE(testEl->rootSet.contains(d));
    ASSERT_EQ(testEl->rightSet1.size(), 2);
    ASSERT_TRUE(testEl->rightSet1.contains(a));
    ASSERT_TRUE(testEl->rightSet1.contains(b));
    ASSERT_FALSE(testEl->rightSet1.contains(c));
    ASSERT_FALSE(testEl->rightSet1.contains(d));
    ASSERT_EQ(testEl->rightSet2.size(), 1);
    ASSERT_TRUE(testEl->rightSet2.contains(a));
    ASSERT_FALSE(testEl->rightSet2.contains(b));
    ASSERT_FALSE(testEl->rightSet2.contains(c));
    ASSERT_FALSE(testEl->rightSet2.contains(d));
    ASSERT_EQ(testEl->rightSet2a.size(), 1);
    ASSERT_FALSE(testEl->rightSet2a.contains(a));
    ASSERT_TRUE(testEl->rightSet2a.contains(b));
    ASSERT_FALSE(testEl->rightSet2a.contains(c));
    ASSERT_FALSE(testEl->rightSet2a.contains(d));
    ASSERT_EQ(testEl->leftSet1.size(), 0);
    ASSERT_FALSE(testEl->leftSet1.contains(a));
    ASSERT_FALSE(testEl->leftSet1.contains(b));
    ASSERT_FALSE(testEl->leftSet1.contains(c));
    ASSERT_FALSE(testEl->leftSet1.contains(d));
    ASSERT_EQ(testEl->leftSet2.size(), 0);
    ASSERT_FALSE(testEl->leftSet2.contains(a));
    ASSERT_FALSE(testEl->leftSet2.contains(b));
    ASSERT_FALSE(testEl->leftSet2.contains(c));
    ASSERT_FALSE(testEl->leftSet2.contains(d));
    ASSERT_EQ(testEl->leftSet2a.size(), 0);
    ASSERT_FALSE(testEl->leftSet2a.contains(a));
    ASSERT_FALSE(testEl->leftSet2a.contains(b));
    ASSERT_FALSE(testEl->leftSet2a.contains(c));
    ASSERT_FALSE(testEl->leftSet2a.contains(d));

    testEl->leftSet2.add(c);
    ASSERT_EQ(testEl->rootSet.size(), 3);
    ASSERT_TRUE(testEl->rootSet.contains(a));
    ASSERT_TRUE(testEl->rootSet.contains(b));
    ASSERT_TRUE(testEl->rootSet.contains(c));
    ASSERT_FALSE(testEl->rootSet.contains(d));
    ASSERT_EQ(testEl->rightSet1.size(), 2);
    ASSERT_TRUE(testEl->rightSet1.contains(a));
    ASSERT_TRUE(testEl->rightSet1.contains(b));
    ASSERT_FALSE(testEl->rightSet1.contains(c));
    ASSERT_FALSE(testEl->rightSet1.contains(d));
    ASSERT_EQ(testEl->rightSet2.size(), 1);
    ASSERT_TRUE(testEl->rightSet2.contains(a));
    ASSERT_FALSE(testEl->rightSet2.contains(b));
    ASSERT_FALSE(testEl->rightSet2.contains(c));
    ASSERT_FALSE(testEl->rightSet2.contains(d));
    ASSERT_EQ(testEl->rightSet2a.size(), 1);
    ASSERT_FALSE(testEl->rightSet2a.contains(a));
    ASSERT_TRUE(testEl->rightSet2a.contains(b));
    ASSERT_FALSE(testEl->rightSet2a.contains(c));
    ASSERT_FALSE(testEl->rightSet2a.contains(d));
    ASSERT_EQ(testEl->leftSet1.size(), 1);
    ASSERT_FALSE(testEl->leftSet1.contains(a));
    ASSERT_FALSE(testEl->leftSet1.contains(b));
    ASSERT_TRUE(testEl->leftSet1.contains(c));
    ASSERT_FALSE(testEl->leftSet1.contains(d));
    ASSERT_EQ(testEl->leftSet2.size(), 1);
    ASSERT_FALSE(testEl->leftSet2.contains(a));
    ASSERT_FALSE(testEl->leftSet2.contains(b));
    ASSERT_TRUE(testEl->leftSet2.contains(c));
    ASSERT_FALSE(testEl->leftSet2.contains(d));
    ASSERT_EQ(testEl->leftSet2a.size(), 0);
    ASSERT_FALSE(testEl->leftSet2a.contains(a));
    ASSERT_FALSE(testEl->leftSet2a.contains(b));
    ASSERT_FALSE(testEl->leftSet2a.contains(c));
    ASSERT_FALSE(testEl->leftSet2a.contains(d));

    testEl->leftSet2a.add(d);
    ASSERT_EQ(testEl->rootSet.size(), 4);
    ASSERT_TRUE(testEl->rootSet.contains(a));
    ASSERT_TRUE(testEl->rootSet.contains(b));
    ASSERT_TRUE(testEl->rootSet.contains(c));
    ASSERT_TRUE(testEl->rootSet.contains(d));
    ASSERT_EQ(testEl->rightSet1.size(), 2);
    ASSERT_TRUE(testEl->rightSet1.contains(a));
    ASSERT_TRUE(testEl->rightSet1.contains(b));
    ASSERT_FALSE(testEl->rightSet1.contains(c));
    ASSERT_FALSE(testEl->rightSet1.contains(d));
    ASSERT_EQ(testEl->rightSet2.size(), 1);
    ASSERT_TRUE(testEl->rightSet2.contains(a));
    ASSERT_FALSE(testEl->rightSet2.contains(b));
    ASSERT_FALSE(testEl->rightSet2.contains(c));
    ASSERT_FALSE(testEl->rightSet2.contains(d));
    ASSERT_EQ(testEl->rightSet2a.size(), 1);
    ASSERT_FALSE(testEl->rightSet2a.contains(a));
    ASSERT_TRUE(testEl->rightSet2a.contains(b));
    ASSERT_FALSE(testEl->rightSet2a.contains(c));
    ASSERT_FALSE(testEl->rightSet2a.contains(d));
    ASSERT_EQ(testEl->leftSet1.size(), 2);
    ASSERT_FALSE(testEl->leftSet1.contains(a));
    ASSERT_FALSE(testEl->leftSet1.contains(b));
    ASSERT_TRUE(testEl->leftSet1.contains(c));
    ASSERT_TRUE(testEl->leftSet1.contains(d));
    ASSERT_EQ(testEl->leftSet2.size(), 1);
    ASSERT_FALSE(testEl->leftSet2.contains(a));
    ASSERT_FALSE(testEl->leftSet2.contains(b));
    ASSERT_TRUE(testEl->leftSet2.contains(c));
    ASSERT_FALSE(testEl->leftSet2.contains(d));
    ASSERT_EQ(testEl->leftSet2a.size(), 1);
    ASSERT_FALSE(testEl->leftSet2a.contains(a));
    ASSERT_FALSE(testEl->leftSet2a.contains(b));
    ASSERT_FALSE(testEl->leftSet2a.contains(c));
    ASSERT_TRUE(testEl->leftSet2a.contains(d));
}

TEST_F(SetTest, AddElementThatIsInSuperSet) {
    TestSubsetsManager m;
    auto testEl = m.create<TestSubsetsElement>();
    auto& p = *m.create<TestPackage>();
    testEl->root.add(p);
    ASSERT_FALSE(testEl->root.empty());
    ASSERT_TRUE(testEl->sub.empty());
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_EQ(testEl->sub.size(), 0);
    ASSERT_TRUE(testEl->root.contains(p.getID()));
    ASSERT_FALSE(testEl->sub.contains(p.getID()));
    testEl->sub.add(p);
    ASSERT_FALSE(testEl->root.empty());
    ASSERT_FALSE(testEl->sub.empty());
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_EQ(testEl->sub.size(), 1);
    ASSERT_TRUE(testEl->root.contains(p.getID()));
    ASSERT_TRUE(testEl->sub.contains(p.getID()));
}

TEST_F(SetTest, removeFirstElementFromOrderedSetTest) {
    TestOrderedSetElementManager m;
    auto testEl = m.create<TestOrderedSetElement>();
    auto& p = *m.create<TestPackage>();
    testEl->set.add(p);
    testEl->set.remove(p);
    ASSERT_TRUE(testEl->set.empty());
    ASSERT_EQ(testEl->set.size(), 0);
    ASSERT_EQ(testEl->set.front(), decltype(testEl)());
}

TEST_F(SetTest, removeLastElementFromOrderedSetTest) {
    TestOrderedSetElementManager m;
    auto testEl = m.create<TestOrderedSetElement>();
    auto& p = *m.create<TestPackage>();
    testEl->set.add(p);
    auto& p2 = *m.create<TestPackage>();
    testEl->set.add(p2);
    testEl->set.remove(p2);
    ASSERT_EQ(*testEl->set.back(), p);
    ASSERT_FALSE(testEl->set.contains(p2.getID()));
    ASSERT_EQ(testEl->set.size(), 1);
}

TEST_F(SetTest, removeMiddleElementFromOrderedSetTest) {
    TestOrderedSetElementManager m;
    auto testEl = m.create<TestOrderedSetElement>();
    auto& p = *m.create<TestPackage>();
    testEl->set.add(p);
    auto& p2 = *m.create<TestPackage>();
    testEl->set.add(p2);
    auto& p3 = *m.create<TestPackage>();
    testEl->set.add(p3);
    testEl->set.remove(p2);
    auto it = testEl->set.begin();
    it++;
    ASSERT_EQ(*it, p3);
}

// uncomment?
// TEST_F(SetTest, removeFromSuperSetTest) {
//     UmlManager m;
//     UmlPtr<TestSubsetsElement> testEl = m.create<TestSubsetsElement>();
//     Package& pckg = *m.create<Package>();
//     testEl->sub.add(pckg);
//     testEl->sub.removeFromJustThisSet(pckg.getID());
//     ASSERT_FALSE(testEl->root.empty());
//     ASSERT_TRUE(testEl->sub.empty());
//     ASSERT_EQ(testEl->root.size(), 1);
//     ASSERT_EQ(testEl->sub.size(), 0);
//     ASSERT_TRUE(testEl->root.contains(pckg.getID()));
//     ASSERT_FALSE(testEl->sub.contains(pckg.getID()));
// }

template <class ManagerPolicy>
class TestTripleSuperSetElement : public ManagerPolicy {
    public:
        using Info = TypeInfo<TestTripleSuperSetElement>;
        Set<TestPackage, TestTripleSuperSetElement> root = Set<TestPackage, TestTripleSuperSetElement>(this);
        Set<TestPackage, TestTripleSuperSetElement> set1 = Set<TestPackage, TestTripleSuperSetElement>(this);
        Set<TestPackage, TestTripleSuperSetElement> set2 = Set<TestPackage, TestTripleSuperSetElement>(this);
        Set<TestPackage, TestTripleSuperSetElement> set3 = Set<TestPackage, TestTripleSuperSetElement>(this);
    private:
        void init() {
            set1.subsets(root);
            set2.subsets(root);
            set3.subsets(root);
        }
    public:
        MANAGED_ELEMENT_CONSTRUCTOR(TestTripleSuperSetElement);
};

template <>
struct ElementInfo<TestTripleSuperSetElement> {
    static std::string name() { return "TestTripleSuperSetElement"; }
};

using TripleSuperSetManager = Manager<TemplateTypeList<TestPackage, TestTripleSuperSetElement>>;


TEST_F(SetTest, tripleRemovePlacholder) {
    TripleSuperSetManager m;
    auto testEl = m.create<TestTripleSuperSetElement>();
    auto& pckg1 = *m.create<TestPackage>();
    auto& pckg2 = *m.create<TestPackage>();
    auto& pckg3 = *m.create<TestPackage>();
    pckg1.setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAAB"));
    pckg2.setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAAC"));
    pckg3.setID(ID::fromString("AAAAAAAAAAAAAAAAAAAAAAAAAAAD"));

    testEl->set1.add(pckg1);
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_EQ(testEl->set1.size(), 1);
    ASSERT_TRUE(testEl->root.contains(pckg1));
    ASSERT_TRUE(testEl->set1.contains(pckg1));
    testEl->set2.add(pckg2);
    ASSERT_EQ(testEl->root.size(), 2);
    ASSERT_EQ(testEl->set2.size(), 1);
    ASSERT_TRUE(testEl->root.contains(pckg2));
    ASSERT_TRUE(testEl->set2.contains(pckg2));
    testEl->set3.add(pckg3);

    ASSERT_EQ(testEl->root.size(), 3);
    ASSERT_EQ(testEl->set1.size(), 1);
    ASSERT_EQ(testEl->set2.size(), 1);
    ASSERT_EQ(testEl->set3.size(), 1);

    ASSERT_TRUE(testEl->root.contains(pckg1));
    ASSERT_TRUE(testEl->set1.contains(pckg1));
    ASSERT_FALSE(testEl->set2.contains(pckg1));
    ASSERT_FALSE(testEl->set3.contains(pckg1));

    ASSERT_TRUE(testEl->root.contains(pckg2));
    ASSERT_FALSE(testEl->set1.contains(pckg2));
    ASSERT_TRUE(testEl->set2.contains(pckg2));
    ASSERT_FALSE(testEl->set3.contains(pckg2));

    ASSERT_TRUE(testEl->root.contains(pckg3));
    ASSERT_FALSE(testEl->set1.contains(pckg3));
    ASSERT_FALSE(testEl->set2.contains(pckg3));
    ASSERT_TRUE(testEl->set3.contains(pckg3));

    testEl->set1.remove(pckg1);

    ASSERT_EQ(testEl->root.size(), 2);
    ASSERT_EQ(testEl->set1.size(), 0);
    ASSERT_EQ(testEl->set2.size(), 1);
    ASSERT_EQ(testEl->set3.size(), 1);

    ASSERT_FALSE(testEl->root.contains(pckg1));
    ASSERT_FALSE(testEl->set1.contains(pckg1));
    ASSERT_FALSE(testEl->set2.contains(pckg1));
    ASSERT_FALSE(testEl->set3.contains(pckg1));

    ASSERT_TRUE(testEl->root.contains(pckg2));
    ASSERT_FALSE(testEl->set1.contains(pckg2));
    ASSERT_TRUE(testEl->set2.contains(pckg2));
    ASSERT_FALSE(testEl->set3.contains(pckg2));

    ASSERT_TRUE(testEl->root.contains(pckg3));
    ASSERT_FALSE(testEl->set1.contains(pckg3));
    ASSERT_FALSE(testEl->set2.contains(pckg3));
    ASSERT_TRUE(testEl->set3.contains(pckg3));

    testEl->set2.remove(pckg2);

    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_EQ(testEl->set1.size(), 0);
    ASSERT_EQ(testEl->set2.size(), 0);
    ASSERT_EQ(testEl->set3.size(), 1);

    ASSERT_FALSE(testEl->root.contains(pckg1));
    ASSERT_FALSE(testEl->set1.contains(pckg1));
    ASSERT_FALSE(testEl->set2.contains(pckg1));
    ASSERT_FALSE(testEl->set3.contains(pckg1));

    ASSERT_FALSE(testEl->root.contains(pckg2));
    ASSERT_FALSE(testEl->set1.contains(pckg2));
    ASSERT_FALSE(testEl->set2.contains(pckg2));
    ASSERT_FALSE(testEl->set3.contains(pckg2));

    ASSERT_TRUE(testEl->root.contains(pckg3));
    ASSERT_FALSE(testEl->set1.contains(pckg3));
    ASSERT_FALSE(testEl->set2.contains(pckg3));
    ASSERT_TRUE(testEl->set3.contains(pckg3));

    testEl->set3.remove(pckg3);

    ASSERT_EQ(testEl->root.size(), 0);
    ASSERT_EQ(testEl->set1.size(), 0);
    ASSERT_EQ(testEl->set2.size(), 0);
    ASSERT_EQ(testEl->set3.size(), 0);

    ASSERT_FALSE(testEl->root.contains(pckg1));
    ASSERT_FALSE(testEl->set1.contains(pckg1));
    ASSERT_FALSE(testEl->set2.contains(pckg1));
    ASSERT_FALSE(testEl->set3.contains(pckg1));

    ASSERT_FALSE(testEl->root.contains(pckg2));
    ASSERT_FALSE(testEl->set1.contains(pckg2));
    ASSERT_FALSE(testEl->set2.contains(pckg2));
    ASSERT_FALSE(testEl->set3.contains(pckg2));

    ASSERT_FALSE(testEl->root.contains(pckg3));
    ASSERT_FALSE(testEl->set1.contains(pckg3));
    ASSERT_FALSE(testEl->set2.contains(pckg3));
    ASSERT_FALSE(testEl->set3.contains(pckg3));
}

template <class ManagerPolicy>
class TestDiamondSuperSetElement : public ManagerPolicy 
{
public:
    using Info = TypeInfo<TestDiamondSuperSetElement>;
    Set<TestPackage, TestDiamondSuperSetElement> root = Set<TestPackage, TestDiamondSuperSetElement>(this);
    Set<TestPackage, TestDiamondSuperSetElement> left = Set<TestPackage, TestDiamondSuperSetElement>(this);
    Set<TestPackage, TestDiamondSuperSetElement> right = Set<TestPackage, TestDiamondSuperSetElement>(this);
    Set<TestPackage, TestDiamondSuperSetElement> bottom = Set<TestPackage, TestDiamondSuperSetElement>(this);
private:
    void init()
    {
        left.subsets(root);
        right.subsets(root);
        bottom.subsets(left);
        bottom.subsets(right);
    }
public:
    MANAGED_ELEMENT_CONSTRUCTOR(TestDiamondSuperSetElement);
};

template <>
struct ElementInfo<TestDiamondSuperSetElement> {
    static std::string name() { return "TestDiamondSuperSetElement"; }
};

using DiamondManager = Manager<TemplateTypeList<TestPackage, TestDiamondSuperSetElement>>;

TEST_F(SetTest, VeryBasicDiamondSubsetTest)
{
    DiamondManager m;
    auto pckg1 = m.create<TestPackage>();
    auto testEl = m.create<TestDiamondSuperSetElement>();
    testEl->bottom.add(pckg1);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 1);
    ASSERT_EQ(testEl->right.size(), 1);
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));
}

TEST_F(SetTest, TwoElementsDiamondSubsetTest)
{
    DiamondManager m;
    auto pckg1 = m.create<TestPackage>();
    auto pckg2 = m.create<TestPackage>();
    auto testEl = m.create<TestDiamondSuperSetElement>();
    testEl->bottom.add(pckg1);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 1);
    ASSERT_EQ(testEl->right.size(), 1);
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));

    testEl->left.add(pckg2);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 2);
    ASSERT_EQ(testEl->right.size(), 1);
    ASSERT_EQ(testEl->root.size(), 2);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg2.id()));
    ASSERT_TRUE(testEl->left.contains(pckg2.id()));
    ASSERT_FALSE(testEl->right.contains(pckg2.id()));
    ASSERT_TRUE(testEl->root.contains(pckg2.id()));
}

TEST_F(SetTest, threeElementDiamondSubsetTest)
{
    DiamondManager m;
    auto pckg1 = m.create<TestPackage>();
    auto pckg2 = m.create<TestPackage>();
    auto pckg3 = m.create<TestPackage>();
    auto testEl = m.create<TestDiamondSuperSetElement>();
    testEl->bottom.add(pckg1);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 1);
    ASSERT_EQ(testEl->right.size(), 1);
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));

    testEl->left.add(pckg2);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 2);
    ASSERT_EQ(testEl->right.size(), 1);
    ASSERT_EQ(testEl->root.size(), 2);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg2.id()));
    ASSERT_TRUE(testEl->left.contains(pckg2.id()));
    ASSERT_FALSE(testEl->right.contains(pckg2.id()));
    ASSERT_TRUE(testEl->root.contains(pckg2.id()));

    testEl->right.add(pckg3);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 2);
    ASSERT_EQ(testEl->right.size(), 2);
    ASSERT_EQ(testEl->root.size(), 3);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg2.id()));
    ASSERT_TRUE(testEl->left.contains(pckg2.id()));
    ASSERT_FALSE(testEl->right.contains(pckg2.id()));
    ASSERT_TRUE(testEl->root.contains(pckg2.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg3.id()));
    ASSERT_FALSE(testEl->left.contains(pckg3.id()));
    ASSERT_TRUE(testEl->right.contains(pckg3.id()));
    ASSERT_TRUE(testEl->root.contains(pckg3.id()));
}

TEST_F(SetTest, fourElementDiamondSubsetTest)
{
    DiamondManager m;
    auto pckg1 = m.create<TestPackage>();
    auto pckg2 = m.create<TestPackage>();
    auto pckg3 = m.create<TestPackage>();
    auto pckg4 = m.create<TestPackage>();
    auto testEl = m.create<TestDiamondSuperSetElement>();
    testEl->bottom.add(pckg1);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 1);
    ASSERT_EQ(testEl->right.size(), 1);
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));

    testEl->left.add(pckg2);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 2);
    ASSERT_EQ(testEl->right.size(), 1);
    ASSERT_EQ(testEl->root.size(), 2);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg2.id()));
    ASSERT_TRUE(testEl->left.contains(pckg2.id()));
    ASSERT_FALSE(testEl->right.contains(pckg2.id()));
    ASSERT_TRUE(testEl->root.contains(pckg2.id()));

    testEl->right.add(pckg3);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 2);
    ASSERT_EQ(testEl->right.size(), 2);
    ASSERT_EQ(testEl->root.size(), 3);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg2.id()));
    ASSERT_TRUE(testEl->left.contains(pckg2.id()));
    ASSERT_FALSE(testEl->right.contains(pckg2.id()));
    ASSERT_TRUE(testEl->root.contains(pckg2.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg3.id()));
    ASSERT_FALSE(testEl->left.contains(pckg3.id()));
    ASSERT_TRUE(testEl->right.contains(pckg3.id()));
    ASSERT_TRUE(testEl->root.contains(pckg3.id()));

    testEl->root.add(pckg4);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 2);
    ASSERT_EQ(testEl->right.size(), 2);
    ASSERT_EQ(testEl->root.size(), 4);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg2.id()));
    ASSERT_TRUE(testEl->left.contains(pckg2.id()));
    ASSERT_FALSE(testEl->right.contains(pckg2.id()));
    ASSERT_TRUE(testEl->root.contains(pckg2.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg3.id()));
    ASSERT_FALSE(testEl->left.contains(pckg3.id()));
    ASSERT_TRUE(testEl->right.contains(pckg3.id()));
    ASSERT_TRUE(testEl->root.contains(pckg3.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg4.id()));
    ASSERT_FALSE(testEl->left.contains(pckg4.id()));
    ASSERT_FALSE(testEl->right.contains(pckg4.id()));
    ASSERT_TRUE(testEl->root.contains(pckg4.id()));
}

TEST_F(SetTest, simpleRemoveFromDiamondSubset) {
    DiamondManager m;
    auto pckg1 = m.create<TestPackage>();
    auto testEl = m.create<TestDiamondSuperSetElement>();
    testEl->bottom.add(pckg1);
    testEl->bottom.remove(pckg1);
    ASSERT_EQ(testEl->bottom.size(), 0);
    ASSERT_EQ(testEl->left.size(), 0);
    ASSERT_EQ(testEl->right.size(), 0);
    ASSERT_EQ(testEl->root.size(), 0);
    ASSERT_FALSE(testEl->bottom.contains(pckg1.id()));
    ASSERT_FALSE(testEl->left.contains(pckg1.id()));
    ASSERT_FALSE(testEl->right.contains(pckg1.id()));
    ASSERT_FALSE(testEl->root.contains(pckg1.id()));
}

TEST_F(SetTest, removefromDiamondSubset) {
    DiamondManager m;
    auto pckg1 = m.create<TestPackage>();
    auto pckg2 = m.create<TestPackage>();
    auto pckg3 = m.create<TestPackage>();
    auto testEl = m.create<TestDiamondSuperSetElement>();
    testEl->bottom.add(pckg1);
    testEl->right.add(pckg2);
    testEl->left.add(pckg3);
    testEl->bottom.remove(pckg1);
    ASSERT_EQ(testEl->bottom.size(), 0);
    ASSERT_EQ(testEl->left.size(), 1);
    ASSERT_EQ(testEl->right.size(), 1);
    ASSERT_EQ(testEl->root.size(), 2);
    ASSERT_FALSE(testEl->bottom.contains(pckg1.id()));
    ASSERT_FALSE(testEl->left.contains(pckg1.id()));
    ASSERT_FALSE(testEl->right.contains(pckg1.id()));
    ASSERT_FALSE(testEl->root.contains(pckg1.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg2.id()));
    ASSERT_FALSE(testEl->left.contains(pckg2.id()));
    ASSERT_TRUE(testEl->right.contains(pckg2.id()));
    ASSERT_TRUE(testEl->root.contains(pckg2.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg3.id()));
    ASSERT_TRUE(testEl->left.contains(pckg3.id()));
    ASSERT_FALSE(testEl->right.contains(pckg3.id()));
    ASSERT_TRUE(testEl->root.contains(pckg3.id()));

    testEl->bottom.add(pckg1);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 2);
    ASSERT_EQ(testEl->right.size(), 2);
    ASSERT_EQ(testEl->root.size(), 3);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg2.id()));
    ASSERT_FALSE(testEl->left.contains(pckg2.id()));
    ASSERT_TRUE(testEl->right.contains(pckg2.id()));
    ASSERT_TRUE(testEl->root.contains(pckg2.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg3.id()));
    ASSERT_TRUE(testEl->left.contains(pckg3.id()));
    ASSERT_FALSE(testEl->right.contains(pckg3.id()));
    ASSERT_TRUE(testEl->root.contains(pckg3.id()));

    testEl->right.remove(pckg2);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 2);
    ASSERT_EQ(testEl->right.size(), 1);
    ASSERT_EQ(testEl->root.size(), 2);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg2.id()));
    ASSERT_FALSE(testEl->left.contains(pckg2.id()));
    ASSERT_FALSE(testEl->right.contains(pckg2.id()));
    ASSERT_FALSE(testEl->root.contains(pckg2.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg3.id()));
    ASSERT_TRUE(testEl->left.contains(pckg3.id()));
    ASSERT_FALSE(testEl->right.contains(pckg3.id()));
    ASSERT_TRUE(testEl->root.contains(pckg3.id()));

    testEl->right.add(pckg2);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 2);
    ASSERT_EQ(testEl->right.size(), 2);
    ASSERT_EQ(testEl->root.size(), 3);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg2.id()));
    ASSERT_FALSE(testEl->left.contains(pckg2.id()));
    ASSERT_TRUE(testEl->right.contains(pckg2.id()));
    ASSERT_TRUE(testEl->root.contains(pckg2.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg3.id()));
    ASSERT_TRUE(testEl->left.contains(pckg3.id()));
    ASSERT_FALSE(testEl->right.contains(pckg3.id()));
    ASSERT_TRUE(testEl->root.contains(pckg3.id()));

    testEl->left.remove(pckg3);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 1);
    ASSERT_EQ(testEl->right.size(), 2);
    ASSERT_EQ(testEl->root.size(), 2);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg2.id()));
    ASSERT_FALSE(testEl->left.contains(pckg2.id()));
    ASSERT_TRUE(testEl->right.contains(pckg2.id()));
    ASSERT_TRUE(testEl->root.contains(pckg2.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg3.id()));
    ASSERT_FALSE(testEl->left.contains(pckg3.id()));
    ASSERT_FALSE(testEl->right.contains(pckg3.id()));
    ASSERT_FALSE(testEl->root.contains(pckg3.id()));

    testEl->left.add(pckg3);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_EQ(testEl->left.size(), 2);
    ASSERT_EQ(testEl->right.size(), 2);
    ASSERT_EQ(testEl->root.size(), 3);
    ASSERT_TRUE(testEl->bottom.contains(pckg1.id()));
    ASSERT_TRUE(testEl->left.contains(pckg1.id()));
    ASSERT_TRUE(testEl->right.contains(pckg1.id()));
    ASSERT_TRUE(testEl->root.contains(pckg1.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg2.id()));
    ASSERT_FALSE(testEl->left.contains(pckg2.id()));
    ASSERT_TRUE(testEl->right.contains(pckg2.id()));
    ASSERT_TRUE(testEl->root.contains(pckg2.id()));
    ASSERT_FALSE(testEl->bottom.contains(pckg3.id()));
    ASSERT_TRUE(testEl->left.contains(pckg3.id()));
    ASSERT_FALSE(testEl->right.contains(pckg3.id()));
    ASSERT_TRUE(testEl->root.contains(pckg3.id()));
}

TEST_F(SetTest, addToRightFirst) {
    DiamondManager m;
    auto pckg1 = m.create<TestPackage>();
    auto pckg2 = m.create<TestPackage>();
    auto testEl = m.create<TestDiamondSuperSetElement>();
    testEl->right.add(pckg1);
    ASSERT_EQ(testEl->root.size(), 1);
    ASSERT_TRUE(testEl->root.contains(pckg1));
    auto i = 0;
    for (__attribute__((unused)) auto& el : testEl->root) {
        i ++;
    }
    ASSERT_EQ(i, 1);
    ASSERT_EQ(testEl->right.size(), 1);
    ASSERT_TRUE(testEl->right.contains(pckg1));
    i = 0;
    for (__attribute__((unused)) auto& el : testEl->right) {
        i ++;
    }
    ASSERT_EQ(i, 1);
    ASSERT_EQ(testEl->left.size(), 0);
    ASSERT_FALSE(testEl->left.contains(pckg1));
    i = 0;
    for (__attribute__((unused)) auto& el : testEl->left) {
        i ++;
    }
    ASSERT_EQ(i, 0);
    ASSERT_EQ(testEl->bottom.size(), 0);
    ASSERT_FALSE(testEl->bottom.contains(pckg1));
    i = 0;
    for (__attribute__((unused)) auto& el : testEl->bottom) {
        i ++;
    }
    ASSERT_EQ(i, 0);

    testEl->bottom.add(pckg2);
    ASSERT_EQ(testEl->root.size(), 2);
    ASSERT_TRUE(testEl->root.contains(pckg2));
    i = 0;
    for (__attribute__((unused)) auto& el : testEl->root) {
        i ++;
    }
    ASSERT_EQ(i, 2);
    ASSERT_EQ(testEl->right.size(), 2);
    ASSERT_TRUE(testEl->right.contains(pckg2));
    i = 0;
    for (__attribute__((unused)) auto& el : testEl->right) {
        i ++;
    }
    ASSERT_EQ(i, 2);
    ASSERT_EQ(testEl->left.size(), 1);
    ASSERT_TRUE(testEl->left.contains(pckg2));
    i = 0;
    for (__attribute__((unused)) auto& el : testEl->left) {
        i ++;
    }
    ASSERT_EQ(i, 1);
    ASSERT_EQ(testEl->bottom.size(), 1);
    ASSERT_TRUE(testEl->bottom.contains(pckg2));
    i = 0;
    for (__attribute__((unused)) auto& el : testEl->bottom) {
        i ++;
    }
    ASSERT_EQ(i, 1);
}
}
// TODO more complex set
