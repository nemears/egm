#pragma once

#include "abstractManager.h"
#include "egm/managedPtr.h"

namespace EGM {
    class DummyManager : public AbstractManager {
        public:
            struct BaseElement {
                using manager = DummyManager;
            };
            template <template <class> class Type>
            struct GenBaseHierarchy {};
            AbstractElementPtr create(std::size_t) override {
                throw ManagerStateException("ERROR DUMMY MANAGER!");
            }
            AbstractElementPtr createPtr(ID) override {
                throw ManagerStateException("ERROR DUMMY MANAGER!");
            }

            AbstractElementPtr abstractGet(ID) override {
                throw ManagerStateException("ERROR DUMMY MANAGER!");
            }

            void release (AbstractElement&) override {
                throw ManagerStateException("ERROR DUMMY MANAGER!");
            }
            AbstractElementPtr getAbstractRoot() const override {
                throw ManagerStateException("ERROR DUMMY MANAGER!");
            }
            void setRoot(AbstractElementPtr) override {
                throw ManagerStateException("ERROR DUMMY MANAGER!");
            }
            void erase(AbstractElement&) override {
                throw ManagerStateException("ERROR DUMMY MANAGER!");
            }
        protected:
            AbstractElementPtr reindex(ID , ID) override {
                throw ManagerStateException("ERROR DUMMY MANAGER!");
            }
            void destroy(ID) override {
                throw ManagerStateException("ERROR DUMMY MANAGER!");
            }
            void addToSet(AbstractSet&, AbstractElement&) const override {
                throw ManagerStateException("ERROR DUMMY MANAGER!");
            }
            void addToSet(AbstractSet&, ID) override {
                throw ManagerStateException("ERROR DUMMY MANAGER!");
            }
            void runAllAddPolicies(AbstractSet&, AbstractElement&) const override {
                throw ManagerStateException("ERROR DUMMY MANAGER!");
            }
    };

    static DummyManager dummyManager;
}
