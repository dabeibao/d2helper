#include "D2Item.hpp"

int Bag::TxtFileNo = -1;         // TGBN bag: 1071
DWORD Bag::Row = 3;
DWORD Bag::Col = 3;

Storage D2Item::storage()
{
    auto        itemData = mItem->pItemData;

    if (itemData == nullptr) {
        return StorageNull;
    }

    switch (itemData->nLocation) {
    case NodeEquip:
        return StorageEquip;
    case NodeBeltSlot:
        return StorageBelt;
    default:
        break;
    }

    switch (mItem->pItemData->nItemLocation) {
    case StorageInventory:
        return StorageInventory;
    case StorageTrade:
        return StorageTrade;
    case StorageStash:
        return StorageStash;
    case StorageCube:
        return StorageCube;
    default:
        return StorageNull;
    }
}

bool D2Item::isEquipBelt()
{
    return storage() == StorageEquip && mItem->pItemData->nBodyLocation == 8;
}

int D2Item::beltRows()
{
    CODE(lbl); // sash
    CODE(vbl); // light belt
    CODE(mbl); // belt
    CODE(tbl); // heavy belt
    CODE(hbl); // plated belt
    CODE(zlb); // demonhide sash
    CODE(zvb); // sharkskin belt
    CODE(zmb); // mesh belt
    CODE(ztb); // battle belt
    CODE(zhb); // war belt
    CODE(ulc); // Spiderweb Sash
    CODE(uvc); // Vampriefang Belt
    CODE(umc); // Mithril Coil
    CODE(utc); // Troll Belt
    CODE(uhc); // Colossus Girdle

    DWORD       itemCode;

    itemCode = code();

    switch (itemCode) {
    case lbl:
    case vbl:
        return 2;
    case mbl:
    case tbl:
        return 3;
    case hbl:
    case zlb:
    case zvb:
    case zmb:
    case ztb:
    case zhb:
    case ulc:
    case uvc:
    case umc:
    case utc:
    case uhc:
        return 4;
    default:
        break;
    }

    auto txt = itemTxt();
    if (txt == nullptr) {
        debug(L"no txt");
        return 1;
    }

    switch (txt->Belt) {
    case 1: // 轻带
    case 4: // 轻束带
        return  2;

    case 5: // 重腰带
        return 3;

    case 3: //  束带
    case 6: // 高级束带
        return 4;
    default:
        debug(L"unknown belt %d code 0x%08x", txt->Belt, code());
        return 1;
    }
}

CODE(rvl);
Potion::Type Potion::getType(UnitAny *pItem)
{
    CODE(rvs);          // Full Rejuvenation Potion
    CODE(hp1);
    CODE(hp2);
    CODE(hp3);
    CODE(hp4);
    CODE(hp5);

    CODE(mp1);
    CODE(mp2);
    CODE(mp3);
    CODE(mp4);
    CODE(mp5);

    CODE(yps);
    CODE(vps);
    CODE(wms);

    D2Item      item(pItem);
    auto        code = item.code();

    switch (code) {
    case rvl:
    case rvs:
        return Purple;
    case hp1:
    case hp2:
    case hp3:
    case hp4:
    case hp5:
        return Red;
    case mp1:
    case mp2:
    case mp3:
    case mp4:
    case mp5:
        return Blue;
    case yps:
    case vps:
    case wms:
        return Junk;
    default:
        return Unknown;
    }
}

bool Potion::isBigPurple(UnitAny *pItem)
{
    D2Item      item(pItem);
    auto        code = item.code();

    return code == rvl;
}
