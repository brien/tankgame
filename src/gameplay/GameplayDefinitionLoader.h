#pragma once

#include "GameplayDefinitions.h"
#include <string>

namespace Gameplay {
enum class DefinitionCommand { Run, Validate, Export, Help };
struct DefinitionOptions {
    DefinitionCommand command = DefinitionCommand::Run;
    std::string path;
};
DefinitionOptions ParseDefinitionOptions(int argc, char* argv[]);
GameplayDefinitions LoadDefinitionFile(const std::string& path);
void ExportBuiltInDefinitionFile(const std::string& path);
// Tries the selected path, or gameplay.json in the working directory.
// On failure returns built-ins and supplies a diagnostic for startup logging.
GameplayDefinitions LoadStartupDefinitions(const DefinitionOptions& options, std::string* diagnostic = nullptr);
const char* DefinitionCommandHelp();
}
