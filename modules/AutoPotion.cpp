#include "Event.hpp"
#include "d2h_module.hpp"
#include "d2ptrs.h"
#include "cConfigLoader.hpp"
#include "hotkey.hpp"
#include "d2vars.h"
#include "Define.h"
#include "Inventory.hpp"
#include "D2Item.hpp"
#include "KeyModule.hpp"

class PotionStorage {
public:
    PotionStorage()
    {
    }

    bool                    add(UnitAny * ptItem)
    {
        D2Item  item(ptItem);
        auto    storage = item.storage();

        switch (storage) {
        case StorageStash:
            mStashPotions.push_back(ptItem);
            return true;

        case StorageCube:
            mCubePotions.push_back(ptItem);
            break;
        case StorageInventory:
            mInvPotions.push_back(ptItem);
            break;
        case StorageBelt:
            mBeltPotions.push_back(ptItem);
            break;
        default:
            break;
        }
        return false;
    }

    UnitAny *               get(Storage * storage)
    {
        if (!mStashPotions.empty()) {
            *storage = StorageStash;
            return mStashPotions[0];
        }
        if (!mCubePotions.empty()) {
            *storage = StorageCube;
            return mCubePotions[0];
        }
        if (!mInvPotions.empty()) {
            *storage = StorageInventory;
            return mInvPotions[0];
        }
        if (!mBeltPotions.empty()) {
            *storage = StorageBelt;
            return mBeltPotions[0];
        }
        *storage = StorageNull;
        return nullptr;
    }

private:
    std::vector<UnitAny *>  mStashPotions;
    std::vector<UnitAny *>  mCubePotions;
    std::vector<UnitAny *>  mInvPotions;
    std::vector<UnitAny *>  mBeltPotions;
};

static void quickEatRVL()
{
    DWORD       dwLife = D2GetUnitStat(PLAYER, STAT_HP, 0)>>8;
    DWORD       dwMaxlife = D2GetUnitStat(PLAYER, STAT_MAXHP, 0)>>8;
    DWORD       mana = D2GetUnitStat(PLAYER, STAT_MANA, 0) >> 8;
    DWORD       maxMana = D2GetUnitStat(PLAYER, STAT_MAXMANA, 0) >> 8;

    if (mana == maxMana && dwLife == dwMaxlife) {
        return;
    }

    bool                useRVS;

    useRVS = dwLife * 100 >= dwMaxlife * (100 - 30);

    PotionStorage       rvs;
    PotionStorage       rvl;

    Inventory::forEach([&](UnitAny * ptItem) {
        D2Item  item(ptItem);
        if (useRVS) {
            if (item.code() == D2Item::rvsCode()) {
                bool done = rvs.add(ptItem);
                return done;
            }

            if (item.code() == D2Item::rvlCode()) {
                rvl.add(ptItem);
            }

            return false;
        }

        // use RVL only
        if (item.code() == D2Item::rvlCode()) {
            return rvl.add(ptItem);
        }
        return false;
    });

    UnitAny *   potion = nullptr;
    Storage     storage;

    if (useRVS) {
        potion = rvs.get(&storage);
        if (potion == nullptr) {
            potion = rvl.get(&storage);
        }
    } else {
        potion = rvl.get(&storage);
    }

    if (potion == nullptr) {
        return;
    }

    if (storage == StorageBelt) {
        BYTE    packet[13] = {0x26,};
        *(DWORD *)&packet[1] = potion->dwUnitId;
        D2SendPacket(sizeof(packet), 0, packet);
    } else {
        D2Util::activeItem(potion, PLAYER->pMonPath->wTargetX, PLAYER->pMonPath->wTargetY);
    }
}

static bool quickEatPotion(struct HotkeyConfig * config)
{
    if (D2Util::uiIsSet(UIVAR_CUBE)) {
        return false;
    }

    quickEatRVL();

    return false;
}

static bool quickPotionEnabled = true;
static uint32_t quickEatPotionKey = Hotkey::Invalid;

static void quickEatPotionLoadConfig()
{
    auto section = CfgLoad::section("helper.quickPotion");
    quickPotionEnabled = section.loadBool("enable", true);
    auto keyString = section.loadString("eatKey");
    quickEatPotionKey = Hotkey::parseKey(keyString);
}

static int quickPotionInstall()
{
    quickEatPotionLoadConfig();
    if (!quickPotionEnabled) {
        return 0;
    }
    if (quickEatPotionKey != Hotkey::Invalid) {
        keyRegisterHotkey(quickEatPotionKey, quickEatPotion, nullptr);
    }
    return 0;
}

static void quickPotionUninstall()
{
}

static void quickPotionOnLoad()
{
}

static void quickPotionReload()
{
}

D2HModule quickPotionModule = {
    "Quick Potin",
    quickPotionInstall,
    quickPotionUninstall,
    quickPotionOnLoad,
    quickPotionReload,
};
