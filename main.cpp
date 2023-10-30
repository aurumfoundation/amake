#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>

/// Build project function
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

        system("mkdir -p build/target");
        if (type == "executable") {
            buildCommand = "g++ -o build/target/" + target;
        } else if (type == "shared") {
            buildCommand = "g++ -shared -fPIC -o build/target/" + target + ".so";
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

        std::ofstream installFile("install.amake");
        if (!installFile.is_open()) {
            std::cerr << "Error: Couldn't create install.amake\n";
            return;
        }

        installFile << "install_targets:\n";

        for (const auto& step : steps) {
            std::string target = step["target"].as<std::string>();
            bool install = step["install"].as<bool>();

            if (install) {
                installFile << "  - target: " << target << "\n";
                installFile << "    type: " << step["type"].as<std::string>() << "\n";
            }
        }

        installFile.close();

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

void installProject()
{
    std::ifstream installFile("install.amake");
    if (!installFile.is_open()) {
        std::cerr << "Error: install.amake not found\n";
        return;
    }

    YAML::Node installConfig = YAML::Load(installFile);

    const YAML::Node& installTargets = installConfig["install_targets"];
    if (!installTargets.IsSequence()) {
        std::cerr << "Error: 'install_targets' must be a sequence in install.amake\n";
        return;
    }

    for (const auto& installTarget : installTargets) {
        std::string target = installTarget["target"].as<std::string>();
        std::string type = installTarget["type"].as<std::string>();

        // Implement installation logic based on target type
        if (type == "executable") {
            // Install executable to /usr/bin/
            int installResult = system(("sudo cp build/target/" + target + " /usr/bin/").c_str());
            // Check installResult for errors
        } else if (type == "shared" || type == "static") {
            // Install library to /usr/lib/
            const char* extension;
            if(type == "shared")
                int installResult = system(("sudo cp build/target/" + target + ".so /usr/bin/").c_str());
            else
                int installResult = system(("sudo cp build/target/" + target + ".a /usr/bin/").c_str());
            
            // Check installResult for errors
        } else {
            std::cerr << "Error: Unknown target type '" << type << "' for target '" << target << "'\n";
            return;
        }

        std::cout << "Installation successful for target: " << target << std::endl;
    }
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
    } else if (command == "install") {
        // Assuming install.amake is present in the same directory
        installProject();
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        return 1;
    }

    return 0;
}

