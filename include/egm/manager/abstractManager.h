#pragma once

#include "egm/id.h"

namespace EGM {

    template <class T>
    class ManagedPtr;
    class AbstractElement;
    using AbstractElementPtr = ManagedPtr<AbstractElement>;
    struct ManagerNode;
    class AbstractSet;

    class  AbstractManager {

        friend struct ManagerNode;
        friend class AbstractElement;
        template <template <class> class, class, class, class>
        friend class PrivateSet;
        
        public:
            virtual AbstractElementPtr create(std::size_t elementType) = 0;
            virtual AbstractElementPtr createPtr(ID id) = 0;
            virtual AbstractElementPtr abstractGet(ID id) = 0;
            virtual void release (AbstractElement& el) = 0;
            virtual AbstractElementPtr getAbstractRoot() const = 0;
            virtual void setRoot(AbstractElementPtr root) = 0;
            virtual void erase(AbstractElement& el) = 0;
        protected:
            virtual AbstractElementPtr reindex(ID oldID, ID newID) = 0;
            virtual void destroy(ID id) = 0;
            virtual void addToSet(AbstractSet& set, AbstractElement& el) const = 0;
            virtual void addToSet(AbstractSet& set, ID id) = 0;
            virtual void runAllAddPolicies(AbstractSet& set, AbstractElement& el) const = 0;

            bool m_runPolicies = true;

            void enablePolicies() { m_runPolicies = true; }
            void disablePolicies() { m_runPolicies = false; }
            bool policiesEnabled() const { return m_runPolicies; }
            
            // TODO rest of funcionality interface
    };

    class ManagerStateException : public std::exception {
        std::string m_msg;
        public:
            ManagerStateException(){};
            ManagerStateException(std::string msg) : m_msg("Uml Manager bad state! " + msg) {};
            const char* what() const throw() override {
                return m_msg.c_str();
            };
    };
}
