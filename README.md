# EGM

EGM which stands for EGM Generic Manager is a C++ header only library to aide in creating an object pool of predefined DataTypes. The library is intended to be used with user defined types compatible with the library (see [Usage](##Usage) section below) and then for those to be able to be used by your other programs. EGM is useful in substitution to a database with some more controls. There are predefined Managers with policies for storage, serialization and IO, but they can be tied in with existing databases or apis by providing your own policies (documentation soon, but examples in `include/egm/policies`). EGM is the simple, configurable and fast Object Pool Manager that can be used for so many applications.

## Usage

EGM is made to be used, there is no executable distibuted by EGM, as well as no library. Once you have EGM installed on your system all you have to do is set up your build to include our headers in `include`. If you want to use all of the features of egm including serialization you must also link your code with [yaml-cpp](https://github.com/jbeder/yaml-cpp). Below is a code snippet of a basic usage of EGM, you can find more complex examples in the test files `src/test`.

```
// include egm/egm.h for all headers in the library
// this would mean that you have to also link with yaml-cpp (see above)
#include "egm/egm.h"

// All Types needed for EGM must be a policy class like below
// A policy class is a template class that inherits its template argument
template <class ManagerPolicy>
struct BaseType : public ManagerPolicy {
    // an Info must be defined for proper use, this is just a basic DataType with no bases to
    // inherit from so we just need to make Info = TypeInfo<BaseType>
    // It is always safest to put this definition before any sets
    using Info = EGM::TypeInfo<BaseType>;

    // Defining a Set of other types, all references to other data must be kept in a set
    // Sets can define how they relate to their polices as well, this one will just make sure
    // that the instance it is referencing also has a reference to it
    using BaseSet = EGM::Set<BaseType, BaseType>;
    BaseSet references = BaseSet(this);
    BaseSet& getReferences() { return references; }

    // init needs to be defined for proper execution
    // it is best practice to make it private
    private:
    void init() {
        references.opposite(&BaseSet::ManagedType::getReferences());
    }
    public:
    

    // Use the macro to cover default constructors with call to init,
    // and the necessary constructor for creation by a manager
    MANAGED_ELEMENT_CONSTRUCTOR(BaseType);
};

// need to define an ElementInfo specialization for serialization implementations
namespace EGM {
    template <>
    struct ElementInfo<BaseType> {
        static std::string name() { return "BaseType"; }
        template <class ManagerPolicy>
        static SetList sets(BaseType<ManagerPolicy>& el) {
            return SetList { make_set_pair("references", el) };
        }
    };
}

// lets define a type that inherits from base type but has field with a string
template <class ManagerPolicy>
struct DerivedType : public ManagerPolicy {
    // Define Info this time with TemplateTypeList of just the BaseType, the 
    // type list allows you to provide as many bases as you want, and you
    // don't have to worry about the diamon inheritance problem
    using Info = EGM::TypeInfo<DerivedType, EGM::TemplateTypeList<BaseType>>;
    
    // a simple data field
    std::string field = "";

    // nothing we need to initialize in the constructor
    private:
    void init() {};
    public:

    MANAGED_ELEMENT_CONSTRUCTOR(DerivedType);
};

// serialization with data other than sets needs some special generic functionality
// defined within the ElementInfo
namespace EGM {
    template <>
    struct ElementInfo<DerivedType> {
        static std::string name() { return "DerivedType"; }
        
        // a policy needs to be provided to translate the field into a string
        template <class ManagerPolicy>
        struct FieldPolicy : public AbstractDataPolicy {
            ManagedPtr<DerivedType<ManagerPolicy>> el;
            FieldPolicy(DerivedType<ManagerPolicy>>& el) : el(el) {}
            std::string getData() {
                return el->field;
            }
            void setData(std::string data) {
                el->field = data;
            }
        };
        
        template <class ManagerPolicy>
        static DataList data(DerivedType<ManagerPolicy>& el) {
            return DataList {
                createDataPair<FieldPolicy<ManagerPolicy>>("field", el)
            };
        }
    };
}

// define a manager to control the types
using CustomManager = EGM::Manager<EGM::TemplateTypeList<BaseType, DerivedType>>;

#include <iostram>

using namespace std;

void example() {
    CustomManager m;

    // create some data, the manager returns a ManagedPtr to the type created
    auto base = m.create<BaseType>();
    auto derived = m.create<DerivedType>();

    // set the data
    base->references.add(derived);
    derived->field = "test";
    
    // object pool mechanics
    base.release();
    base.aquire();
    derived.release();
    
    // will aquire instances if they are release if accessed
    cout << "derived field: " << derived->field << endl;
    cout << "base refrences size: " << base->references.size() << endl;
    cout << "derived references size: " << derived-refrences.size() << endl;

    // should print out
    // derived field: test
    // base references size: 1
    // derived references size: 1
}
```
