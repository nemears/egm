#pragma once

#include <memory>
#include <unordered_map>
#include "yaml-cpp/yaml.h"
#include "egm/manager/abstractElement.h"
#include "egm/manager/abstractManager.h"
#include "egm/manager/typeInfo.h"
#include "egm/manager/managerTypes.h"
#include "egm/managedPtr.h"
#include "egm/set/abstractSet.h"
#include "egm/set/privateSet.h"
#include "egm/util/templateTypeList.h"
#include "egm/util/visitor.h"

namespace EGM {
    class SerializationError : public std::exception {
        private:
            std::string m_msg;
        public:
            SerializationError(std::string msg) : m_msg(msg) {};
            virtual const char* what() const throw() {
                return m_msg.c_str();
            }
    };



    template <class Tlist>
    class YamlSerializationPolicy : virtual public ManagerTypes<Tlist> {
        private:
            using TypedManager = ManagerTypes<Tlist>;
            using BaseElement = typename TypedManager::BaseElement;
            struct AbstractSerializationPolicy {
                YamlSerializationPolicy& m_manager;
                AbstractSerializationPolicy(YamlSerializationPolicy& manager) : m_manager(manager) {}
                virtual ~AbstractSerializationPolicy() {}
                virtual AbstractElementPtr create() = 0;
                virtual void parseBody(YAML::Node, AbstractElementPtr) const = 0;
                virtual void parseScope(YAML::Node, AbstractElementPtr) const = 0;
                virtual void parseComposite(YAML::Node, AbstractElementPtr) const = 0;
                virtual std::string emit(AbstractElementPtr) const = 0;
                virtual void emitComposite(YAML::Emitter& emitter, AbstractElementPtr el) const = 0;
            };

            template <template <class> class Type>
            static void emitData(YAML::Emitter& emitter, Type<typename TypedManager::template GenBaseHierarchy<Type>>& el) {
                if constexpr (HasData<Type>{}) {
                    for (auto& dataPair : Type<BaseElement>::Info::data(el)) {
                        std::string data = dataPair.second->getData();
                        if (!data.empty()) {
                            emitter << YAML::Key << dataPair.first << YAML::Value << data;
                        }
                    }
                }
            }

            struct EmitVisitor {
                ManagedPtr<BaseElement> el;
                YAML::Emitter& emitter;
                template <template <class> class Type>
                void visit() {
                    emitData<Type>(emitter, el->template as<Type>());
                    if constexpr (HasSets<Type>{}) {
                        for (auto& setPair : Type<BaseElement>::Info::sets(el->template as<Type>())) {
                            auto set = setPair.second;
                            if (set->empty() || set->getComposition() == CompositionType::ANTI_COMPOSITE || !set->rootSet()) {
                                continue;
                            }

                            // check if subsets have any of our elements
                            std::size_t numElsInSet = set->size();
                            for (auto id : set->ids()) {
                                auto subSetWithEl = set->subSetContains(id);
                                if (subSetWithEl) {
                                    numElsInSet--;
                                }
                            }

                            // all in subsets continue
                            if (numElsInSet == 0) {
                                continue;
                            }


                            emitter << YAML::Key << setPair.first;
                            switch (set->setType()) {
                                case SetType::SET:
                                case SetType::ORDERED_SET:
                                {
                                    emitter << YAML::BeginSeq;
                                    for (auto id : set->ids()) {
                                        auto subSetWithEl = set->subSetContains(id);
                                        if (subSetWithEl && !subSetWithEl->readonly()) {
                                            continue;
                                        }
                                        emitter << id.string();
                                    }
                                    emitter << YAML::EndSeq;
                                    break;
                                }
                                case SetType::SINGLETON : {
                                    emitter << YAML::Value << set->ids().front().string();
                                    break;
                                }
                                default:
                                    throw SerializationError("Could not emit, cannot handle set type!");
                            }
                        }
                    }
                }
            };

            struct EmitCompositeVisitor {
                ManagedPtr<BaseElement> el;
                YAML::Emitter& emitter;
                YamlSerializationPolicy& manager;
                template <template <class> class Type>
                void visit() {
                    emitData<Type>(emitter, el->template as<Type>());
                    if constexpr (HasSets<Type>{}) {
                        for (auto& setPair : Type<BaseElement>::Info::sets(el->template as<Type>())) {
                            auto set = setPair.second;
                            if (set->empty() || set->getComposition() == CompositionType::ANTI_COMPOSITE || !set->rootSet()) {
                                continue;
                            }

                            // check if subsets have any of our elements
                            std::size_t numElsInSet = set->size();
                            for (auto id : set->ids()) {
                                auto subSetWithEl = set->subSetContains(id);
                                if (subSetWithEl) {
                                    numElsInSet--;
                                }
                            }

                            // all in subsets continue
                            if (numElsInSet == 0) {
                                continue;
                            }

                            emitter << YAML::Key << setPair.first;
                            switch (set->setType()) {
                                case SetType::SET:
                                case SetType::ORDERED_SET:
                                {
                                    emitter << YAML::BeginSeq;
                                    if (set->getComposition() == CompositionType::COMPOSITE) {
                                        for (auto it = set->beginPtr(); *it != *set->endPtr(); it->next()) {
                                            auto el = it->getCurr();
                                            auto subsetWithEl = set->subSetContains(el.id());
                                            if (subsetWithEl && !subsetWithEl->readonly()) {
                                                continue;
                                            }
                                            
                                            manager.m_serializationByType.at(el->getElementType())->emitComposite(emitter, el);
                                        }
                                    } else if (set->getComposition() == CompositionType::NONE) {
                                        for (auto id : set->ids()) {
                                            auto subSetWithEl = set->subSetContains(id);
                                            if (subSetWithEl && !subSetWithEl->readonly()) {
                                                continue;
                                            }
                                            emitter << id.string();
                                        }
                                    }
                                    emitter << YAML::EndSeq;
                                    break;
                                }
                                case SetType::SINGLETON : {
                                    emitter << YAML::Value << set->ids().front().string();
                                    break;
                                }
                                default:
                                    throw SerializationError("Could not emit, cannot handle set type!");
                            }
                        } 
                    } 
                }
            };

            struct FindValidScopeVisitor {
                ManagedPtr<BaseElement> el; 
                std::unique_ptr<std::pair<std::string, AbstractSet*>> validMatch = 0;
                bool matchedAndNotReadOnly = false;
                template <template <class> class Type>
                void visit() {
                    if (matchedAndNotReadOnly) {
                        return;
                    }
                    if constexpr (HasSets<Type>{}) {
                        for (auto& setPair : Type<BaseElement>::Info::sets(el->template as<Type>())) {
                            if (setPair.second->getComposition() == CompositionType::ANTI_COMPOSITE && setPair.second->size() > 0) {
                                if (validMatch && setPair.second->readonly()) {
                                    continue;
                                }
                                validMatch = std::make_unique<std::pair<std::string, AbstractSet*>>(setPair);
                                matchedAndNotReadOnly = !setPair.second->readonly();
                            }
                        }
                    }
                }
            };

            template <template <class> class Type>
            static void parseData(YAML::Node node, Type<typename TypedManager::template GenBaseHierarchy<Type>>& el) {
                if constexpr (HasData<Type>{}) {
                    for (auto& dataPair : Type<BaseElement>::Info::data(el)) {
                        if (node[dataPair.first]) {
                            auto dataNode = node[dataPair.first];
                            if (dataNode.IsScalar()) {
                                dataPair.second->setData(dataNode.template as<std::string>());
                            }
                        }
                    }
                }
            }

            struct ParseCompositeVisitor {
                ManagedPtr<BaseElement> el;
                YAML::Node node;
                YamlSerializationPolicy& manager;
                template <template <class> class Type>
                void visit() {
                    parseData<Type>(node, el->template as<Type>());
                    if constexpr (HasSets<Type>{}) {
                        for (auto& setPair : Type<BaseElement>::Info::sets(el-> template as<Type>())) {
                            if (!setPair.second->rootSet()) {
                                continue;
                            }
                            auto set = setPair.second;
                            auto createAndParse = [this](YAML::Node node) -> AbstractElementPtr {
                                auto it = node.begin();
                                const auto keyNode = it->first;
                                const auto valNode = it->second;
                                auto serialization_policy = manager.m_serializationByName.at(keyNode.as<std::string>());
                                auto elToParse = serialization_policy->create();
                                serialization_policy->parseComposite(valNode, elToParse); 
                                return elToParse;
                            };
                            if (node[setPair.first]) {
                                auto setNode = node[setPair.first];
                                if (setNode.IsScalar()) {
                                    if (set->setType() != SetType::SINGLETON) {
                                        throw SerializationError("bad format for " + setPair.first + ", line number " + std::to_string(setNode.Mark().line));
                                    }
                                    manager.addToSet(*set, ID::fromString(setNode.template as<std::string>()));
                                } else if (setNode.IsSequence()) {
                                    for (const auto valNode : setNode) {
                                        if (valNode.IsMap()) {
                                            auto setEl = createAndParse(valNode);
                                            manager.addToSet(*set, *setEl);
                                        } else if (valNode.IsScalar()) {
                                            manager.addToSet(*set, ID::fromString(valNode.template as<std::string>()));
                                        }
                                    }
                                } else if (setNode.IsMap()) {
                                    if (set->setType() != SetType::SINGLETON) {
                                        throw SerializationError("bad format for " + setPair.first + ", line number " + std::to_string(setNode.Mark().line));
                                    }
                                    auto setEl = createAndParse(setNode);
                                    manager.addToSet(*set, *setEl);
                                }
                            } 
                        }
                    }
                }
            };

            struct ParseBodyVisitor {
                ManagedPtr<BaseElement> el;
                YAML::Node node;
                YamlSerializationPolicy& manager;
                template <template <class> class Type>
                void visit() {
                    parseData<Type>(node, el->template as<Type>());
                    if constexpr (HasSets<Type>{}) {
                        for (auto& setPair : Type<DummyManager::BaseElement>::Info::sets(el->template as<Type>())) {
                            if (!setPair.second->rootSet()) {
                                continue;
                            }
                            auto set = setPair.second;
                            if (node[setPair.first]) {
                                auto setNode = node[setPair.first];
                                if (setNode.IsScalar()) {
                                    if (set->setType() != SetType::SINGLETON) {
                                        throw SerializationError("bad format for " + setPair.first + ", line number " + std::to_string(setNode.Mark().line));
                                    }
                                    manager.addToSet(*set, ID::fromString(setNode.template as<std::string>()));
                                } else if (setNode.IsSequence()) {
                                    for (const auto& valNode : setNode) {
                                        manager.addToSet(*set, ID::fromString(valNode.template as<std::string>()));
                                    }
                                } else {
                                    throw SetStateException("Invalid set formatting for individual parsing! line number " + std::to_string(setNode.Mark().line));
                                }
                            }
                        }
                    }
                }
            };

            struct ParseScopeVisitor {
                ManagedPtr<BaseElement> el;
                YAML::Node node;
                YamlSerializationPolicy& manager;
                bool found = false;
                template <template <class> class Type>
                void visit() {
                    if (found) {
                        return;
                    }
                    if constexpr (HasSets<Type>{}) {
                        for (auto& setPair : Type<BaseElement>::Info::sets(el->template as<Type>())) {
                            if (setPair.second->readonly()) {
                                continue;
                            }
                            auto set = dynamic_cast<AbstractReadableSet*>(setPair.second);
                            if (set->getComposition() != CompositionType::ANTI_COMPOSITE || !set->rootSet()) {
                                continue;
                            }
                            if (node[setPair.first]) {
                                auto setNode = node[setPair.first];
                                if (setNode.IsScalar()) {
                                    if (set->setType() != SetType::SINGLETON) {
                                        throw SerializationError("bad format for " + setPair.first + ", line number " + std::to_string(setNode.Mark().line));
                                    }
                                    manager.addToSet(*set, ID::fromString(setNode.template as<std::string>()));
                                    found = true;
                                }
                            }
                        }
                    }
                }
            };

            template <template <class> class Type>
            struct SerializationPolicy : public AbstractSerializationPolicy {
                AbstractElementPtr create() override {
                    return this->m_manager.template create<Type>();
                }
                SerializationPolicy(YamlSerializationPolicy& manager) : AbstractSerializationPolicy(manager) {}
                void parseBody(YAML::Node bodyNode, AbstractElementPtr el) const override {
                    ParseBodyVisitor visitor {el, bodyNode, this->m_manager};
                    visitBasesBFS<ParseBodyVisitor, Type, Tlist>(visitor);
                }
                void parseScope(YAML::Node elNode, AbstractElementPtr el) const override {
                    ParseScopeVisitor visitor {el, elNode, this->m_manager};
                    visitBasesBFS<ParseScopeVisitor, Type, Tlist>(visitor);
                }
                void parseComposite(YAML::Node node, AbstractElementPtr el) const override {
                    if (node["id"]) {
                        if (!node["id"].IsScalar()) {
                            throw SerializationError("invalid node for id, must be a scalar");
                        }
                        el->setID(ID::fromString(node["id"].as<std::string>()));
                    }
                    ParseCompositeVisitor visitor { el, node, this->m_manager};
                    visitBasesDFS<ParseCompositeVisitor, Type, Tlist>(visitor);
                }
                std::string emit(AbstractElementPtr el) const override {
                    if constexpr (typename TypedManager::template IsAbstract<Type>{}) {
                        throw ManagerStateException("Error Tried to emit abstract type!");
                    } else {
                        YAML::Emitter emitter;
                        emitter << YAML::BeginMap;

                        // emit scope
                        FindValidScopeVisitor findValidScopeVisitor {el};
                        visitBasesBFS<FindValidScopeVisitor, Type, Tlist>(findValidScopeVisitor);
                        if (findValidScopeVisitor.validMatch) {
                            emitter << YAML::Key << findValidScopeVisitor.validMatch->first << YAML::Value << findValidScopeVisitor.validMatch->second->ids().front().string();
                        }

                        // emit body
                        std::string elementName = Type<DummyManager::BaseElement>::Info::name();
                        emitter << YAML::Key << elementName << YAML::Value << YAML::BeginMap;
                        emitter << YAML::Key << "id" << YAML::Value << el.id().string();
                        EmitVisitor visitor {el, emitter};
                        visitBasesDFS<EmitVisitor, Type, Tlist>(visitor);
                        emitter << YAML::EndMap;
                        emitter << YAML::EndMap;

                        return emitter.c_str();
                    }
                }
                void emitComposite(YAML::Emitter& emitter, AbstractElementPtr el) const override {
                    if constexpr (typename TypedManager::template IsAbstract<Type>{}) {
                        throw ManagerStateException("Error tried to emit abstract type!");
                    } else {
                        emitter << YAML::BeginMap;
                        emitter << YAML::Key << Type<BaseElement>::Info::name();
                        emitter << YAML::Value << YAML::BeginMap;
                        emitter << YAML::Key << "id";
                        emitter << YAML::Value << el.id().string();
                        EmitCompositeVisitor visitor = { el, emitter, this->m_manager };
                        visitBasesDFS<EmitCompositeVisitor, Type, Tlist>(visitor);
                        emitter << YAML::EndMap;
                        emitter << YAML::EndMap;
                    }
                }
            };
            std::unordered_map<std::string, std::shared_ptr<AbstractSerializationPolicy>> m_serializationByName;
            std::unordered_map<std::size_t, std::shared_ptr<AbstractSerializationPolicy>> m_serializationByType;

            template <class TypeList, class Dummy = void>
            struct PopulatePolicies;

            template <template <class> class First, template <class> class ... Rest>
            struct PopulatePolicies<TemplateTypeList<First, Rest...>> {
                static void populate(YamlSerializationPolicy& manager) {
                    auto serialization_policy = std::make_shared<SerializationPolicy<First>>(manager);
                    if constexpr ( !typename TypedManager::template IsAbstract<First> {} ) {
                        manager.m_serializationByName.emplace(First<BaseElement>::Info::name(), serialization_policy);
                    }
                    constexpr int type_id = TemplateTypeListIndex<First, Tlist>::result;
                    manager.m_serializationByType.emplace(type_id, serialization_policy);
                    PopulatePolicies<TemplateTypeList<Rest...>>::populate(manager); 
                }
            };

            template <class Dummy>
            struct PopulatePolicies<TemplateTypeList<>, Dummy> {
                static void populate(__attribute__((unused)) YamlSerializationPolicy& manager) {}
            };
        public:
            YamlSerializationPolicy() {
                PopulatePolicies<Tlist>::populate(*this);
            }
        protected:
            AbstractElementPtr parseIndividual(std::string data) {
                std::vector<YAML::Node> rootNodes = YAML::LoadAll(data);
                if (rootNodes.empty()) {
                    throw SerializationError("could not parse data supplied to manager! Is it JSON or YAML?");
                }
                auto node = rootNodes[0];
                auto it = node.begin();
                while (it != node.end()) {
                    const auto keyNode = it->first;
                    const auto valNode = it->second;
                    if (valNode.IsMap()) {
                        // look up key
                        try {
                            auto serialization_policy = m_serializationByName.at(keyNode.as<std::string>());
                            AbstractElementPtr el = serialization_policy->create();
                            if (valNode["id"] && valNode["id"].IsScalar()) {
                                el->setID(ID::fromString(valNode["id"].template as<std::string>()));
                            }
                            serialization_policy->parseBody(valNode, el);
                            serialization_policy->parseScope(node, el);
                            return el;
                        } catch (std::exception& e) {
                            throw SerializationError("Could not find proper type to parse! line number " + std::to_string(keyNode.Mark().line));
                        }
                    }
                    it++;
                } 
                throw SerializationError("Could not identify type to parse relevant to this manager!");
            }
            std::vector<ManagedPtr<AbstractElement>> parseWhole(std::string data) {
                std::vector<YAML::Node> rootNodes = YAML::LoadAll(data);
                if (rootNodes.empty()) {
                    throw SerializationError("could not parse data supplied to manager! Is it JSON or YAML?");
                }
                std::vector<AbstractElementPtr> ret;
                ret.reserve(rootNodes.size());
                for (auto node : rootNodes) {
                    auto it = node.begin();
                    const auto keyNode = it->first;
                    const auto valNode = it->second;
                    if (valNode.IsMap()) {
                        try {
                            auto serialization_policy = m_serializationByName.at(keyNode.as<std::string>());
                            AbstractElementPtr el = serialization_policy->create();
                            serialization_policy->parseComposite(valNode, el);
                            ret.push_back(el);
                        } catch (std::exception& e) {
                            throw SerializationError("Could not find proper type to parse! line number " + std::to_string(keyNode.Mark().line));
                        }
                    }
                }
                return ret;
            }
            std::string emitIndividual(AbstractElement& el) {
                return m_serializationByType.at(el.getElementType())->emit(&el);
            }
            std::string emitWhole(AbstractElement& el) {
                YAML::Emitter emitter;
                primeEmitter(emitter);
                m_serializationByType.at(el.getElementType())->emitComposite(emitter, AbstractElementPtr(&el));
                return emitter.c_str();
            }
            virtual void primeEmitter(__attribute__((unused)) YAML::Emitter& emitter) {}
        public:
            std::string dump() {
                return  this->emitWhole(*this->getAbstractRoot());
            }
            std::string dump(AbstractElement& el) {
                return this->emitWhole(el);
            }
    };

    template <class Tlist>
    class JsonSerializationPolicy : public YamlSerializationPolicy<Tlist> {
        private:
            bool emitYaml = false;
        protected:
            void primeEmitter(YAML::Emitter& emitter) override {
                if (!emitYaml) {
                    emitter << YAML::DoubleQuoted << YAML::Flow;
                }
            }
            std::string dumpYaml() {
                emitYaml = true;
                auto ret = this->emitWhole(*this->getAbstractRoot());
                emitYaml = false;
                return ret;
            }
            std::string dumpYaml(AbstractElement& el) {
                emitYaml = true;
                auto ret = this->emitWhole(el);
                emitYaml = false;
                return ret;
            }
    };
}
