#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>

/// Build project function
void buildProject() {
    std::cout << "Building project using build.amake\n";

    // Read build.amake
    std::ifstream build_file("build.amake");
    if (!build_file.is_open()) {
        std::cerr << "Error: Couldn't open build.amake\n";
        return;
    }

    // Parse custom build syntax
    std::string line;
    std::vector<std::string> buildLines;
    while (std::getline(build_file, line)) {
        buildLines.push_back(line);
    }

    // Extract build steps
    for (const auto& step : buildLines) {
        std::istringstream iss(step);
        std::vector<std::string> tokens(std::istream_iterator<std::string>{iss},
                                        std::istream_iterator<std::string>());

        if (tokens.empty()) {
            continue; // Ignore empty lines
        }

        std::string target = tokens[0];
        std::cout << "Building target: " << target << std::endl;

        std::string type = tokens[1];
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

        // Assuming the rest are source files and libraries
        for (size_t i = 2; i < tokens.size(); ++i) {
            buildCommand += " " + tokens[i];
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

void installProject() {
    std::ifstream installFile("install.amake");
    if (!installFile.is_open()) {
        std::cerr << "Error: install.amake not found\n";
        return;
    }

    // Parse custom install syntax
    std::string line;
    std::vector<std::string> installLines;
    while (std::getline(installFile, line)) {
        installLines.push_back(line);
    }

    // Extract install targets
    for (const auto& installTarget : installLines) {
        std::istringstream iss(installTarget);
        std::vector<std::string> tokens(std::istream_iterator<std::string>{iss},
                                        std::istream_iterator<std::string>());

        if (tokens.size() < 3 || tokens[0] != "install" || tokens[1] != "target:") {
            continue; // Ignore invalid lines
        }

        std::string target = tokens[2];
        std::string type = tokens[4];

        // Implement installation logic based on target type
        if (type == "executable") {
            // Install executable to /usr/bin/
            int installResult = system(("sudo cp build/target/" + target + " /usr/bin/").c_str());
            // Check installResult for errors
        } else if (type == "shared" || type == "static") {
            // Install library to /usr/lib/
            const char* extension = (type == "shared") ? ".so" : ".a";
            int installResult = system(("sudo cp build/target/" + target + extension + " /usr/bin/").c_str());
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
