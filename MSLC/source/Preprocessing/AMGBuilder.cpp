#include "..\..\include\Preprocessing\AMGBuilder.hpp"
#include "..\..\include\Common\Diagnostics\Diagnostics.hpp"
#include "..\..\include\Syntax\StringConstants.hpp"
#include <fstream>
#include <queue>

namespace MSLC
{
	namespace Preprocessing
	{
		std::string AMGBuilder::ResolvePath(const std::string& base_path, const std::string& relative_path)
        {
            if (std::filesystem::path(relative_path).is_absolute()) {
                return relative_path;
            }

            //TODO Add checking global paths

            // For relative paths by base_path
            std::filesystem::path base_dir = std::filesystem::path(base_path).parent_path();
            std::filesystem::path resolved = base_dir / relative_path;

            // Path normalising путь (delete .., . and etc)
            return std::filesystem::canonical(resolved).string();
        }
        void AMGBuilder::AnalyzeRecursive(HandlingOrder& HO, const std::string& file_path, size_t current_layer)
        {
            // Cycle dependencies checking
            if (HO.currently_processing.count(file_path)) {
                Diagnostics::Logger::Get().PrintWithFormat(Diagnostics::InformationMessage("Cyclic dependency detected involving file: %s.", Diagnostics::MessageType::FileError),
                    file_path
                );
                return;
            }

            // If file is already handled -  check: need we update layer?
            if (HO.path_id.count(file_path)) {
                file_identificator id = HO.path_id[file_path];
                FileInfo& info = HO.files[id];

                //  If current layer is deeper - update
                if (current_layer > info.layer) {
                    info.layer = current_layer;
                    // Also we need update dependencies of this file
                    for (auto dep_id : info.dependencies) {
                        const FileInfo& dep_info = HO.files[dep_id];
                        AnalyzeRecursive(HO, dep_info.path, current_layer + 1);
                    }
                }
                return;
            }

            // Начинаем обработку этого файла Start handling of this file
            HO.currently_processing.insert(file_path);

            // Load file
            std::stringstream text = LoadFile(file_path);
            if (text.str().empty()) {
                HO.currently_processing.erase(file_path);
                return;
            }

            // Create writting of this file
            file_identificator id = HO.global_id++;
            HO.files[id] = FileInfo{ file_path, current_layer, id, {} };
            HO.path_id[file_path] = id;
            FileInfo& current_info = HO.files[id];

            // Imports parsing
            std::string line;
            while (std::getline(text, line))
            {
                // Delete leading spaces
                size_t start = line.find_first_not_of(" \t");
                if (start == std::string::npos) continue;

                if (line.substr(start)._Starts_with("#" + std::string(Directives::w_import.data())))
                {
                    std::string import_path = GetImportDirectiveArgument(line);

                    if (import_path.empty())
                    {
                        Diagnostics::Logger::Get().PrintWithFormat(
                            Diagnostics::InformationMessage(
                                Diagnostics::ErrorTexts::import_directive_argument.data(),
                                Diagnostics::MessageType::FileError,
                                Diagnostics::SourceType::SourceCode,
                                0
                            ),
                            import_path,
                            file_path
                        );
                        continue;
                    }

                    // Resolve import's path
                    std::string resolved_path = ResolvePath(file_path, import_path);

                    // Recursive analyze dependencies
                    AnalyzeRecursive(HO, resolved_path, current_layer + 1);

                    // Add dependce if file has loaded successfully
                    if (HO.path_id.count(resolved_path)) {
                        file_identificator dep_id = HO.path_id[resolved_path];
                        current_info.dependencies.insert(dep_id);

                        // Update dependence's layer if it's needed
                        FileInfo& dep_info = HO.files[dep_id];
                        if (current_layer + 1 > dep_info.layer) {
                            dep_info.layer = current_layer + 1;
                        }
                    }
                }
            }

            // End handling of this file
            HO.currently_processing.erase(file_path);
            HO.processed_files.insert(file_path);
        }
        std::vector<file_identificator> AMGBuilder::BuildProcessingOrder(const HandlingOrder& HO)
        {
            std::vector<file_identificator> order;
            std::unordered_map<file_identificator, size_t> in_degree;
            std::queue<file_identificator> zero_incoming;

            // Calculate enter degrees
            for (const auto& [id, info] : HO.files) {
                in_degree[id] = 0;
            }

            for (const auto& [id, info] : HO.files) {
                for (file_identificator dep_id : info.dependencies) {
                    in_degree[dep_id]++;
                }
            }

            // Find nodes without enter dependencies
            for (const auto& [id, degree] : in_degree) {
                if (degree == 0) {
                    zero_incoming.push(id);
                }
            }

            // Topologic sort (algorythm of Cana)
            while (!zero_incoming.empty()) {
                file_identificator id = zero_incoming.front();
                zero_incoming.pop();
                order.push_back(id);

                for (file_identificator dep_id : HO.files.at(id).dependencies) {
                    if (--in_degree[dep_id] == 0) {
                        zero_incoming.push(dep_id);
                    }
                }
            }

            // Check handling's state of nodes
            if (order.size() != HO.files.size()) {
                Diagnostics::Logger::Get().Print(Diagnostics::InformationMessage("Circular dependencies detected in import graph.", Diagnostics::MessageType::FileError));
                return {};
            }

            // Sort by layers (deep dependencies are first)
            std::sort(order.begin(), order.end(),
                [&HO](file_identificator a, file_identificator b) {
                    return HO.files.at(a).layer > HO.files.at(b).layer;
                });

            return order;
        }
        HandlingOrder AMGBuilder::Analyze(const std::string& main_file)
        {
            HandlingOrder HO;


            // Start recursive analysis with root file
            AnalyzeRecursive(HO, main_file, 0);

            // Build handling order
            std::vector<file_identificator> processing_order = BuildProcessingOrder(HO);

            HO.processing_order = processing_order;

            return HO;
        }
        std::string AMGBuilder::GetImportDirectiveArgument(const std::string& directive)
        {
            size_t start = directive.find('"');
            if (start == std::string::npos) return "";

            size_t end = directive.find('"', start + 1);
            if (end == std::string::npos) return "";

            return directive.substr(start + 1, end - start - 1);
        }
        std::stringstream AMGBuilder::LoadFile(const std::string& path)
        {
            std::ifstream file(path);
            if (!file.is_open())
            {
                Diagnostics::Logger::Get().PrintWithFormat(
                    Diagnostics::InformationMessage(
                        Diagnostics::ErrorTexts::file_loading_bad_path.data(),
                        Diagnostics::MessageType::FileError,
                        Diagnostics::SourceType::SourceCode,
                        0
                    ),
                    path
                );
                return std::stringstream();
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            return buffer;
        }
	}
}