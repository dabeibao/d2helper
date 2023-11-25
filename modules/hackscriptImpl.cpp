#include "FileScanner.hpp"
#include "HelpFunc.h"
#include "log.h"
#include "macros.hpp"
#include "strutils.hpp"
#include "loader.hpp"

enum TOKEN {
    TOKEN_STRING,
    TOKEN_END,
};

using bytes_t = std::vector<uint8_t>;

struct HackPatcher {
    std::string dllName;
    uint32_t    offset;
    bytes_t     origCode;
    bytes_t     newCode;
    bool        verify;

    int         patch()
    {
        DWORD   addr = GetDllOffset(dllName.c_str(), offset);

        if (addr == 0) {
            log_trace("Patch to %s offset 0x%x: cannot get offset\n",
                      dllName.c_str(), offset);
            return -1;
        }
        if (verify) {
            int ret = PatchCompare(addr, origCode.data(), origCode.size(),
                                   newCode.data(), newCode.size());
            if (ret < 0) {
                log_trace("Patch to %s 0x%lx: compare failed\n",
                          dllName.c_str(), addr, offset);
            }
            return ret;
        } else {
            WriteLocalBYTES(addr, newCode.data(), newCode.size());
            return 0;
        }
    }
};

static char * skipSpace(char * input)
{
    while (isspace(*input)) {
        input += 1;
    }
    return input;
}

static char * parseToken(char * input, char ** parsed)
{
    char *      token = input;
    bool        is_end = false;

    while (1) {
        char    c = *token;
        if (c == '#') {
            is_end = true;
            *token = '\0';
            break;
        }
        if (isspace(c)) {
            *token = '\0';
            break;
        }
        if (c == '\0') {
            is_end = true;
            break;
        }
        token += 1;
    }
    if (token == input) {
        *parsed = NULL;
    } else {
        *parsed = input;
    }
    if (!is_end) {
        return token + 1;
    }
    return NULL;
}

static int split(char * line, char * argv[], int size)
{
    char *      input = line;
    int         index = 0;

    while (input != NULL) {
        char *  next;
        char *  token;

        input = skipSpace(input);
        next = parseToken(input, &token);
        if (token == NULL) {
            break;
        }
        if (index >= size) {
            return index;
        }
        argv[index] = token;
        index += 1;
        input = next;
    }
    return index;
}

static int toHex(char c)
{
    c = tolower(c);
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }

    log_trace("'%c' is invalid\n", c);
    return -1;
}

static int getChar(const char *b, uint8_t * byte)
{
    int         l;
    int         h;

    h = toHex(b[0]);
    if (h < 0) {
        return -1;
    }
    l = toHex(b[1]);
    if (l < 0) {
        return -1;
    }

    *byte = h * 16 + l;
    return 0;
}

static int parseBytes(const char * input, bytes_t& bytes)
{
    int         len = strlen(input);
    if (len == 0 || len % 2 != 0) {
        log_trace("%s: len is invalid\n", input, len);
        return -3;
    }
    bytes.resize(len / 2);

    for (int i = 0; i < len / 2; i += 1) {
        int     ret;
        uint8_t byte;

        ret = getChar(&input[i * 2], &byte);
        if (ret < 0) {
            return ret;
        }
        bytes[i] = byte;
    }
    return 0;
}

static int parsePatcher(HackPatcher * patch, char * tokens[], int number)
{
    int                 ret;
    unsigned int        offset;
    bool ok;

    patch->dllName = tokens[0];
    if (patch->dllName.empty()) {
        return -1;
    }
    offset = helper::toInt(tokens[1], &ok);
    if (!ok) {
        return -2;
    }
    patch->offset = offset;
    ret = parseBytes(tokens[2], patch->origCode);
    if (ret < 0) {
        return -4;
    }
    ret = parseBytes(tokens[3], patch->newCode);
    if (ret < 0) {
        return -5;
    }
    if (number == 4) {
        patch->verify = 1;
    } else {
        int     index;
        if (number == 5) {
            index = 4;
            // our format
            // # d2game.dll offset orig new 1
        } else {
            // d2hack script format:
            // # d2game.dll offset orig new 0 1
            index = 5;
        }
        unsigned int verify = helper::toInt(tokens[index], &ok);
        if (!ok) {
            return -6;
        }
        patch->verify = verify;
    }

    return 1;
}

static int parseLine(char * line, HackPatcher * patcher)
{
    char *      tokens[5]; // dll name, offset, pattern, modified, [repeated]
    int         number;

    number = split(line, tokens, ARRAY_SIZE(tokens));
    //printf("Split to %d\n", number);
    if (number < 0) {
        return number;
    }
    if (number == 0) {
        return 0;
    }

    if (number < 4) {
        log_trace("    Require at least 4 fields\n");
        return -1;
    }

    return parsePatcher(patcher, tokens, number);
}

static bool loadFile(const char * fileName, std::vector<HackPatcher>& patches)
{
    char        path[512];
    char        realFileName[512 + 32];

    getAppDirectory(path, sizeof(path));
    snprintf(realFileName, sizeof(realFileName), "%s\\%s", path, fileName);
    log_verbose("Load script file from %s\n", realFileName);

    FileScanner scanner(realFileName);
    char *      line;
    int         lineNo = 0;

    if (!scanner.isOpen()) {
        log_trace("Load script file %s failed\n", realFileName);
        return false;
    }

    while ((line = scanner.getLine()) != nullptr) {
        int             ret;
        HackPatcher     patcher;

        lineNo += 1;
        ret = parseLine(line, &patcher);
        if (ret == 0) {
            continue;
        }
        if (ret < 0) {
            log_trace("Line: %d '%s' is invalid\n", lineNo, line);
            return false;
        }
        patches.push_back(patcher);
    }

    return true;
}

void hackScriptRunPatch(const std::string& file)
{
    std::vector<HackPatcher>    patches;
    if (!loadFile(file.c_str(), patches)) {
        return;
    }

    for (auto& patch: patches) {
        int ret = patch.patch();
        if (ret == 0) {
            log_verbose("    patched %s offset 0x%x\n", patch.dllName.c_str(), patch.offset);
        }
    }
}

#ifdef BUILD_HACKSCRIPT

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        hackScriptRunPatch("d2hack.script");
        return TRUE;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) PVOID __stdcall QueryInterface()
{
    return NULL;
}
#endif
