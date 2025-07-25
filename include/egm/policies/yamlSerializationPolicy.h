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
        protected:
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
                virtual void emitScope(YAML::Emitter&, AbstractElementPtr) const = 0;
                virtual void emitBody(YAML::Emitter&, AbstractElementPtr) const = 0;
                virtual void emit(YAML::Emitter&, AbstractElementPtr) const = 0;
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

            bool set_valid_to_emit(AbstractSet& set) {
                if (set.empty() || set.getComposition() == CompositionType::ANTI_COMPOSITE || !set.rootSet()) {
                    return false;
                }

                // check if subsets have any of our elements
                std::size_t numElsInSet = set.size();
                for (auto id : set.ids()) {
                    auto subSetWithEl = set.subSetContains(id);
                    if (subSetWithEl) {
                        numElsInSet--;
                    }
                }

                // all in subsets continue
                if (numElsInSet == 0) {
                    return false;
                }

                return true;
            }

            virtual void emit_set(YAML::Emitter& emitter, std::string set_name, AbstractSet& set) {
                if (!set_valid_to_emit(set)) {
                    return;
                }

                emitter << YAML::Key << set_name;
                switch (set.setType()) {
                    case SetType::SET:
                    case SetType::ORDERED_SET:
                    {
                        emitter << YAML::BeginSeq;
                        for (auto id : set.ids()) {
                            auto subSetWithEl = set.subSetContains(id);
                            if (subSetWithEl && !subSetWithEl->readonly()) {
                                continue;
                            }
                            emitter << id.string();
                        }
                        emitter << YAML::EndSeq;
                        break;
                    }
                    case SetType::SINGLETON : {
                        emitter << YAML::Value << set.ids().front().string();
                        break;
                    }
                    default:
                        throw SerializationError("Could not emit, cannot handle set type!");
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
                            // dispatch 
                            dynamic_cast<YamlSerializationPolicy&>(el->getManager()).emit_set(emitter, setPair.first, *setPair.second);
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
                                    if (set->getComposition() == CompositionType::COMPOSITE) {
                                        emitter << YAML::Value;
                                        auto el = set->beginPtr()->getCurr();
                                        manager.m_serializationByType.at(el->getElementType())->emitComposite(emitter, el);
                                    } else {
                                        emitter << YAML::Value << set->ids().front().string();
                                    }
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
                if constexpr (requires{ElementInfo<Type>::data(el);}) {
                // if constexpr (HasData<Type>{}) {
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
                    if constexpr (requires{ Type<BaseElement>::Info::sets(el-> template as<Type>()); }) {
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

            virtual void parse_set(YAML::Node node, std::string set_name, AbstractSet& set) {
                if (!set.rootSet()) {
                    return;
                }

                if (node[set_name]) {
                    auto setNode = node[set_name];
                    if (setNode.IsScalar()) {
                        if (set.setType() != SetType::SINGLETON) {
                            throw SerializationError("bad format for " + set_name + ", line number " + std::to_string(setNode.Mark().line));
                        }
                        this->addToSet(set, ID::fromString(setNode.template as<std::string>()));
                    } else if (setNode.IsSequence()) {
                        for (const auto& valNode : setNode) {
                            this->addToSet(set, ID::fromString(valNode.template as<std::string>()));
                        }
                    } else {
                        throw SetStateException("Invalid set formatting for individual parsing! line number " + std::to_string(setNode.Mark().line));
                    }
                } 
            }

            struct ParseBodyVisitor {
                ManagedPtr<BaseElement> el;
                YAML::Node node;
                YamlSerializationPolicy& manager;
                template <template <class> class Type>
                void visit() {
                    parseData<Type>(node, el->template as<Type>());
                    if constexpr (HasSets<Type>{}) {
                        for (auto& setPair : Type<DummyManager::BaseElement>::Info::sets(el->template as<Type>())) {
                            manager.parse_set(node, setPair.first, *setPair.second);
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
                            auto set = setPair.second;
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
                    visitBasesBFS<ParseCompositeVisitor, Type, Tlist>(visitor);
                }
                void emitScope(YAML::Emitter& emitter, AbstractElementPtr el) const override {
                    // emit scope
                    FindValidScopeVisitor findValidScopeVisitor {el};
                    visitBasesBFS<FindValidScopeVisitor, Type, Tlist>(findValidScopeVisitor);
                    if (findValidScopeVisitor.validMatch) {
                        emitter << YAML::Key << findValidScopeVisitor.validMatch->first << YAML::Value << findValidScopeVisitor.validMatch->second->ids().front().string();
                    } 
                }
                void emitBody(YAML::Emitter& emitter, AbstractElementPtr el) const override {
                    EmitVisitor visitor {el, emitter};
                    visitBasesReverseBFS<EmitVisitor, Tlist, Type>(visitor); 
                }
                void emit(YAML::Emitter& emitter, AbstractElementPtr el) const override {
                    if constexpr (typename TypedManager::template IsAbstract<Type>{}) {
                        throw ManagerStateException("Error Tried to emit abstract type!");
                    } else {
                        emitScope(emitter, el);
                        std::string elementName = Type<DummyManager::BaseElement>::Info::name();
                        emitter << YAML::Key << elementName << YAML::Value << YAML::BeginMap;
                        emitter << YAML::Key << "id" << YAML::Value << el.id().string();
                        emitBody(emitter, el);
                        emitter << YAML::EndMap;
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
                        visitBasesReverseBFS<EmitCompositeVisitor, Tlist, Type>(visitor);
                        emitter << YAML::EndMap;
                        emitter << YAML::EndMap;
                    }
                }
            };

            std::unordered_map<std::string, std::shared_ptr<AbstractSerializationPolicy>> m_serializationByName;
            std::unordered_map<std::size_t, std::shared_ptr<AbstractSerializationPolicy>> m_serializationByType;
        private:

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
            virtual AbstractElementPtr parseNode(YAML::Node node) {
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
            AbstractElementPtr parseIndividual(std::string data) {
                std::vector<YAML::Node> rootNodes = YAML::LoadAll(data);
                if (rootNodes.empty()) {
                    throw SerializationError("could not parse data supplied to manager! Is it JSON or YAML?");
                }
                return parseNode(rootNodes[0]);
            }
            // parses nodes compositely
            // returns root node parsed, or null if couldn't pars
            virtual ManagedPtr<AbstractElement> parse_composite_node(YAML::Node node) {
                auto it = node.begin();
                const auto keyNode = it->first;
                const auto valNode = it->second;
                if (valNode.IsMap()) {
                    try {
                        auto serialization_policy = m_serializationByName.at(keyNode.as<std::string>());
                        AbstractElementPtr el = serialization_policy->create();
                        serialization_policy->parseComposite(valNode, el);
                        return el;
                    } catch (std::out_of_range& e) {
                        this->enablePolicies();
                        throw SerializationError("Could not find proper type to parse! line number " + std::to_string(keyNode.Mark().line));
                    } catch (std::exception& e) {
                        this->enablePolicies();
                        throw e;
                    }     
                } 
                return AbstractElementPtr();
            }
            virtual std::vector<ManagedPtr<AbstractElement>> parseWhole(std::string data) {
                // policies are supposed to be run once all elements are available 
                // they should be run after parsing by the manager or the caller
                this->disablePolicies(); 
                std::vector<YAML::Node> rootNodes = YAML::LoadAll(data);
                if (rootNodes.empty()) {
                    throw SerializationError("could not parse data supplied to manager! Is it JSON or YAML?");
                }
                std::vector<AbstractElementPtr> ret;
                ret.reserve(rootNodes.size());
                for (auto node : rootNodes) {
                    ret.push_back(parse_composite_node(node));
                }

                this->enablePolicies();
                return ret;
            }
            virtual void emitIndividual(YAML::Emitter& emitter, AbstractElement& el) {
                m_serializationByType.at(el.getElementType())->emit(emitter, &el);
            }
            virtual std::string emitIndividual(AbstractElement& el) {
                YAML::Emitter emitter;
                primeEmitter(emitter);
                emitter << YAML::BeginMap;
                emitIndividual(emitter, el);
                emitter << YAML::EndMap;
                return emitter.c_str();
            }
            virtual std::string emitWhole(AbstractElement& el) {
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
            AbstractElementPtr parse(std::string data) {
                return this->parseWhole(data)[0];
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
