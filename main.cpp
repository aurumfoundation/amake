#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>

void buildProject() {
    std::cout << "Building project using build.amake\n";

    // Read build.amake
    std::ifstream build_file("build.amake");
    if (!build_file.is_open()) {
        std::cerr << "Error: Couldn't open build.amake\n";
        return;
    }

    // Parse YAML
    YAML::Node buildConfig = YAML::Load(build_file);


    // Extract build steps
    const YAML::Node& steps = buildConfig["steps"];
    if (!steps.IsSequence()) {
        std::cerr << "Error: 'steps' must be a sequence in build.amake\n";
        return;
    }

    for (const auto& step : steps) {
        std::string target = step["target"].as<std::string>();
        std::cout << "Building target: " << target << std::endl;

        std::string type = step["type"].as<std::string>();
        std::cout << "Type: " << type << std::endl;

        std::string buildCommand;

        if (type == "executable") {
            buildCommand = "g++ -o " + target;
        } else if (type == "shared") {
            buildCommand = "g++ -shared -fPIC -o " + target + ".so";
        } else if (type == "static") {
            buildCommand = "g++ -c " + target + ".o";
            // Assuming ar command is available
            buildCommand += " && ar rcs lib" + target + ".a " + target + ".o";
        } else if (type == "aurum_application") {
            buildCommand = "g++ -o " + target + " -e appmain";
        } else if (type == "custom") {
            // Replace this with the actual custom build script/command
            buildCommand = "./" + target + "_custom.sh";
        } else {
            std::cerr << "Error: Unknown build type '" << type << "' for target '" << target << "'\n";
            return;
        }

        // Assuming "sources" is a sequence of strings
        const YAML::Node& sources = step["sources"];
        if (!sources.IsSequence()) {
            std::cerr << "Error: 'sources' must be a sequence in build.amake\n";
            return;
        }

        const YAML::Node& linkWithTargets = step["link_with"];
        if (linkWithTargets) {
            for (const auto& linkTarget : linkWithTargets) {
                std::string linkTargetName = linkTarget.as<std::string>();
                std::cout << "Linking with: " << linkTargetName << std::endl;

                // Assuming the target exists in the steps
                const YAML::Node& linkedStep = buildConfig[linkTargetName];
                if (!linkedStep) {
                    std::cerr << "Error: Target '" << linkTargetName << "' not found for linking\n";
                    return;
                }

                std::string linkCommand = "g++ -o " + target;

                const YAML::Node& linkedSources = linkedStep["sources"];
                for (const auto& linkedSource : linkedSources) {
                    linkCommand += " " + linkedSource.as<std::string>();
                }

                const YAML::Node& linkedLibraries = linkedStep["libraries"];
                for (const auto& linkedLibrary : linkedLibraries) {
                    linkCommand += " -l" + linkedLibrary.as<std::string>();
                }

                // Execute the link command
                int linkResult = system(linkCommand.c_str());

                if (linkResult != 0) {
                    std::cerr << "Error: Failed to link with target '" << linkTargetName << "'\n";
                    return;
                }

                std::cout << "Linking successful with target: " << linkTargetName << std::endl;
            }
        }

        // Collecting source files
        std::vector<std::string> sourceFiles;
        for (const auto& source : sources) {
            sourceFiles.push_back(source.as<std::string>());
            std::cout << "Source: " << source.as<std::string>() << std::endl;
        }

        // Assuming "libraries" is a sequence of strings
        const YAML::Node& libraries = step["libraries"];

        // Collecting libraries
        std::vector<std::string> libraryFiles;
        for (const auto& library : libraries) {
            libraryFiles.push_back(library.as<std::string>());
            std::cout << "Libraries: " << library.as<std::string>() << std::endl;
        }

        // Add source files and libraries to the build command
        for (const auto& source : sourceFiles) {
            buildCommand += " " + source;
        }

        for (const auto& library : libraryFiles) {
            buildCommand += " -l" + library;
        }

        // Execute the build command
        int result = system(buildCommand.c_str());

        if (result != 0) {
            std::cerr << "Error: Failed to build target '" << target << "'\n";
            return;
        }

        std::cout << "Build successful for target: " << target << std::endl;
    }

    // Add overall build logic if needed
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <command>\n";
        return 1;
    }

    std::string command = argv[1];

    if (command == "build") {
        buildProject();
    } else if (command == "version") {
        std::cout << "amake version 1.0\n";
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        return 1;
    }

    return 0;
}

