#pragma once

#include <filesystem>
#include <fstream>
#include "egm/manager/manager.h"

namespace EGM {

    struct FilePersistenceError : public ManagerStateException {
        using ManagerStateException::ManagerStateException;
    };

    class FilePersistencePolicy {
        private:
            std::string m_projectPath = "project.yml";
            std::string m_mountPath = "mount";
        protected:
            std::string loadElementData(ID id) {
                std::string dataPath = m_mountPath + "/mount/" + id.string() + ".yml"; // TODO extension from serialization policy?
                std::ifstream file(dataPath);
                if (!file.good()) {
                    throw FilePersistenceError("No such file exists! " + dataPath);
                }
                return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()); 
            }
            void saveElementData(std::string data, ID id) {
                std::string dataPath = m_mountPath + "/mount/" + id.string() + ".yml"; // TODO extension from serialization policy?
                std::ofstream file(dataPath);
                file << data;
                file.close();
            }
            std::string getProjectData() {
                std::ifstream file(m_projectPath);
                if (!file.good()) {
                    throw FilePersistenceError("No such file exists! " + m_projectPath);
                }
                return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());         
            }
            std::string getProjectData(std::string path) {
                m_projectPath = path;
                return getProjectData(); 
            }
            void saveProjectData(std::string data) {
                std::ofstream file(m_projectPath);
                file << data;
                file.close();
            }
            void saveProjectData(std::string data, std::string path) {
                m_projectPath = path;
                saveProjectData(data);
            }
            void eraseEl(ID id) {
                std::string dataPath = m_mountPath + "/mount/" + id.string() + ".yml"; // TODO extension from serialization policy?
                std::filesystem::remove(dataPath); 
            }
            void reindex(ID oldID, ID newID) {
                std::string oldDataPath = m_mountPath + "/mount/" + oldID.string() + ".yml"; // TODO extension from serialization policy?
                std::string newDataPath = m_mountPath + "/mount/" + newID.string() + ".yml"; // TODO extension from serialization policy?
                if (std::filesystem::exists(oldDataPath))
                    std::filesystem::rename(newDataPath, oldDataPath);
            }
        public:
            void mount(std::string mountPath) {
                m_mountPath = mountPath;
                std::filesystem::create_directories(m_mountPath / std::filesystem::path("mount"));
            }
            virtual ~FilePersistencePolicy() {
                // delete mount
                if (!m_mountPath.empty()) {
                    try {
                        std::filesystem::remove_all(std::filesystem::path(m_mountPath) / "mount");
                    } catch (std::exception& e) {
                        // TODO don't know what to do
                        // std::cout << "Persistence policy encountered error deleting mount: " << e.what() << std::endl;
                    }
                }
            }
    };
}
