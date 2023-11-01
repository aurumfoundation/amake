#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// Function to split a string by space
std::vector<std::string> split(const std::string& s) {
    std::istringstream iss(s);
    return {std::istream_iterator<std::string>{iss},
            std::istream_iterator<std::string>{}};
}

// Function to build project using custom syntax
void buildProject() {
    std::cout << "Building project using amakefile\n";

    // Read amakefile
    std::ifstream build_file("amakefile");
    if (!build_file.is_open()) {
        std::cerr << "Error: Couldn't open amakefile\n";
        return;
    }

    // Parse amakefile
    std::vector<std::string> buildConfig;
    for (std::string line; getline(build_file, line); ) {
        buildConfig.push_back(line);
    }

    for (const auto& step : buildConfig) {
        // Split the step by space
        std::vector<std::string> tokens = split(step);

        if (tokens.empty()) {
            continue;
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

        // Assuming the rest of the tokens are sources and libraries
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
}

// Function to install project using custom syntax
void installProject() {
    std::ifstream installFile("install.amake");
    if (!installFile.is_open()) {
        std::cerr << "Error: install.amake not found\n";
        return;
    }

    std::vector<std::string> installConfig;
    for (std::string line; getline(installFile, line); ) {
        installConfig.push_back(line);
    }

    for (const auto& installTarget : installConfig) {
        // Split the install target by space
        std::vector<std::string> tokens = split(installTarget);

        if (tokens.size() < 3) {
            std::cerr << "Error: Invalid install target format\n";
            return;
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
            const char* extension;
            if (type == "shared") {
                int installResult = system(("sudo cp build/target/" + target + ".so /usr/bin/").c_str());
            } else {
                int installResult = system(("sudo cp build/target/" + target + ".a /usr/bin/").c_str());
            }

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

