#include "GameplayDefinitionLoader.h"
#include "GameplayDefinitionJson.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace Gameplay {
DefinitionOptions ParseDefinitionOptions(int argc, char* argv[]) {
    DefinitionOptions result;
    bool selected = false;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        DefinitionCommand command;
        if (argument == "--gameplay-config") command = DefinitionCommand::Run;
        else if (argument == "--validate-gameplay-config") command = DefinitionCommand::Validate;
        else if (argument == "--export-gameplay-config") command = DefinitionCommand::Export;
        else if (argument == "--help") command = DefinitionCommand::Help;
        // macOS Finder may supply its process serial number.
        else if (argument.compare(0, 5, "-psn_") == 0) continue;
        else throw std::invalid_argument("Unknown argument: " + argument);
        if (selected) throw std::invalid_argument("Gameplay configuration commands are mutually exclusive");
        selected = true;
        result.command = command;
        if (command != DefinitionCommand::Help) {
            if (++i >= argc || std::string(argv[i]).empty() || std::string(argv[i]).compare(0, 2, "--") == 0)
                throw std::invalid_argument(argument + " requires a file path");
            result.path = argv[i];
        }
    }
    return result;
}

GameplayDefinitions LoadDefinitionFile(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error(path + ": cannot open configuration file");
    std::string text;
    char buffer[4096];
    while (input) {
        input.read(buffer, sizeof(buffer));
        text.append(buffer, static_cast<std::size_t>(input.gcount()));
        if (text.size() > 1024 * 1024) throw std::invalid_argument(path + ": document exceeds 1 MiB");
    }
    if (!input.eof()) throw std::runtime_error(path + ": failed reading configuration file");
    try { return ParseDefinitionJson(text); }
    catch (const std::exception& e) { throw std::invalid_argument(path + ": " + e.what()); }
}

void ExportBuiltInDefinitionFile(const std::string& path) {
    const std::string text = SerializeDefinitionJson(BuildBuiltInDefinitions());
    // Exclusive creation prevents accidental replacement of an authored file.
    FILE* output = std::fopen(path.c_str(), "wx");
    if (!output) throw std::runtime_error(path + ": cannot create export: " + std::strerror(errno));
    const bool written = std::fwrite(text.data(), 1, text.size(), output) == text.size();
    const bool closed = std::fclose(output) == 0;
    if (!written || !closed) {
        std::remove(path.c_str());
        throw std::runtime_error(path + ": failed writing export");
    }
}

GameplayDefinitions LoadStartupDefinitions(const DefinitionOptions& options, std::string* diagnostic) {
    const std::string path = options.path.empty() ? "gameplay.json" : options.path;
    try {
        auto definitions = LoadDefinitionFile(path);
        if (diagnostic) *diagnostic = "Loaded " + path + " (schema version 1)";
        return definitions;
    } catch (const std::exception& e) {
        if (diagnostic) *diagnostic = "Could not load " + path + ": " + e.what() +
            "; using built-in defaults";
    } catch (...) {
        if (diagnostic) *diagnostic = "Could not load " + path +
            ": unknown loading error; using built-in defaults";
    }
    return BuildBuiltInDefinitions();
}

const char* DefinitionCommandHelp() {
    return "Usage: tankgame [--gameplay-config <path>]\n"
           "       tankgame --validate-gameplay-config <path>\n"
           "       tankgame --export-gameplay-config <new-path>\n"
           "       tankgame --help\n"
           "Defaults to gameplay.json; loading failures are logged and use built-in defaults.\n"
           "Paths are relative to the launch working directory. Exit codes: 0 success, 1 failure.\n";
}
}
