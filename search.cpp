#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

namespace fs = std::filesystem;

struct FoundLine {
    std::string filepath;
    int line_num;
    std::string line;
};

std::vector<FoundLine> find_files_with_content(const std::string& directory, const std::string& content) {
    std::vector<FoundLine> found_lines;
    
    try {
		for (const auto& entry : fs::recursive_directory_iterator(directory)) {
			if (entry.is_regular_file()) {
				std::ifstream file(entry.path());
				
				if (!file.is_open()) {
					std::cerr << "Error reading file " << entry.path() << std::endl;
					continue;
				}
				
				std::string line;
				int line_num = 0;
				
				while (std::getline(file, line)) {
					++line_num;
					if (line.find(content) != std::string::npos) found_lines.push_back({ entry.path().string(), line_num, line });
				}
			}
		}
	} catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }

    return found_lines;
}

void write_to_log(const std::vector<FoundLine>& found_lines, const std::string& log_filename, const std::string& content) {
    std::ofstream log_file(log_filename);
    
    if (log_file.is_open()) {
        log_file << "Files containing `" << content << "`:\n";
        log_file << std::string(50, '-') << '\n';
        
        for (const auto& entry : found_lines) {
            log_file << "File: " << entry.filepath << '\n';
            log_file << "Line " << entry.line_num << ": " << entry.line << '\n';
            log_file << std::string(50, '-') << '\n';
        }
    }
}

std::pair<std::string, std::string> parse_input(const std::string& input) {
    size_t delimiter_pos  = input.find('%');
    if (delimiter_pos == std::string::npos) throw std::invalid_argument("Invalid input format. Use: search <directory>%<content>");
    
    std::string directory = input.substr(0, delimiter_pos);
    std::string content   = input.substr(delimiter_pos + 1);
    
    return {directory, content};
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory>%<content>" << std::endl;
        return 1;
    }

    std::string input = argv[1];
    std::pair<std::string, std::string> parsed_input;

    try {
        parsed_input = parse_input(input);
    } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::string directory       = parsed_input.first;
    std::string content_to_find = parsed_input.second;
    
    if (
		!fs::exists(directory) ||
		!fs::is_directory(directory)
	) {
        std::cerr << "The specified directory does not exist: " << directory << std::endl;
        return 1;
    }
    
    auto found_lines = find_files_with_content(directory, content_to_find);

    if (found_lines.empty()) {
		std::cout << "No files containing `" << content_to_find << "` found." << std::endl;
		return 1;
	}
    
    std::cout << "Files containing `" << content_to_find << "` found:" << std::endl;
        
    for (const auto& entry : found_lines) {
        std::cout << "File: " << entry.filepath << std::endl;
        std::cout << "Line "  << entry.line_num << ": " << entry.line << std::endl;
        std::cout << std::string(50, '-') << std::endl;
    }
        
    std::string log_filename = directory + "found_content.log";
    std::cout << "Found content saved to " << log_filename << " [" << found_lines.size() << "]" << std::endl;
        
    write_to_log(found_lines, log_filename, content_to_find);
    return 0;
}
