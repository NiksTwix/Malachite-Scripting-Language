#pragma once
#include "..\Definitions\Common.hpp"
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <sstream>



namespace MSLC 
{
	namespace Preprocessing 
	{

		using file_identificator = Definitions::ModuleId;

		struct FileInfo 
		{
			std::string path;
			size_t layer = 0;//0-main

			file_identificator file_id = 0;	

			std::unordered_set<file_identificator> dependencies;
		};

		struct HandlingOrder
		{
			std::unordered_map<std::string, file_identificator> path_id;
			std::unordered_map<file_identificator, FileInfo> files;
			std::vector<file_identificator> processing_order; 

			file_identificator global_id = 0;

			// ƒл€ отслеживани€ уже обработанных файлов и обнаружени€ циклов
			std::unordered_set<std::string> processed_files;
			std::unordered_set<std::string> currently_processing;

			const FileInfo& GetFileInfo(file_identificator id) const {
				return files.at(id);
			}

			const FileInfo& GetFileInfo(const std::string& path) const {
				return files.at(path_id.at(path));
			}

			std::vector<std::string> GetFilePathsInOrder() const {
				std::vector<std::string> result;
				for (auto id : processing_order) {
					result.push_back(files.at(id).path);
				}
				return result;
			}
		};


        class AMGBuilder
        {
        private:
            // –азрешение путей
            std::string ResolvePath(const std::string& base_path,
                const std::string& relative_path);

            void AnalyzeRecursive(HandlingOrder& HO,
                const std::string& file_path,
                size_t current_layer);

            // ѕостроение упор€доченного списка дл€ обработки (топологическа€ сортировка)
            std::vector<file_identificator> BuildProcessingOrder(const HandlingOrder& HO);
        public:
            HandlingOrder Analyze(const std::string& main_file);

            // ќстальные методы остаютс€ аналогичными...
            std::string GetImportDirectiveArgument(const std::string& directive);

			std::stringstream LoadFile(const std::string& path);
        };
    }
}
