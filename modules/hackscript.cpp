#include "d2h_module.hpp"
#include "cConfigLoader.hpp"
#include "log.h"
#include "hackscriptImpl.hpp"

#define DEFAULT_FILE_NAME       "d2hack.script"

static bool hackScriptEnabled;
static std::string hackScriptFile;

static int hackScriptModuleInstall()
{
    auto section = CfgLoad::section("helper.hackscript");
    hackScriptEnabled = section.loadBool("enable", false);

    if (!hackScriptEnabled) {
        log_verbose("hackscript is disabled\n");
        return 0;
    }
    hackScriptFile = section.loadString("file", DEFAULT_FILE_NAME);
    if (hackScriptFile.empty()) {
        log_verbose("hackscript: no script name\n");
        return 0;
    }
    hackScriptRunPatch(hackScriptFile);

    return 0;
}

static void hackScriptModuleUninstall()
{
    return;
}

D2HModule hackScriptModule = {
    "Hack Script",
    hackScriptModuleInstall,
    hackScriptModuleUninstall,
    nullptr,
    nullptr,
};
