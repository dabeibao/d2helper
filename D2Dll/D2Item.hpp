#pragma once

#include <stdint.h>
#include <stdio.h>
#include "d2ptrs.h"
#include "d2structs.h"
#include "D2Utils.hpp"
#include "d2vars.h"

enum Storage {
    StorageInventory    = 0,
    StorageEquip        = 1,
    StorageTrade        = 2,
    StorageCube         = 3,
    StorageStash        = 4,
    StorageBelt         = 5,
    StorageNull         = 255,
};

enum NodeLocation {
    NodeGround          = 0,
    NodeStorage         = 1,
    NodeBeltSlot        = 2,
    NodeEquip           = 3,
};

#define CODE(x) static constexpr DWORD x = D2Item::toTxtCode(#x)

#define DEFINE_CODE(x)                          \
    static inline constexpr DWORD x ## Code() { return D2Item::toTxtCode(#x); }

class D2Item {
public:
    static constexpr DWORD      rvlTxt = 516;

public:
    template <int Size = 4>
    static inline constexpr DWORD toTxtCode(const char * c)
    {
        DWORD   v = 0;
        int     i;

        for (i = 0; i < 4; i += 1) {
            if (c[i] == 0) {
                break;
            }
            v += ((DWORD)c[i]) << (i * 8);
        }
        for (; i < Size; i += 1) {
            v += (DWORD)0x20 << (i * 8);
        }
        return v;
    }

    template<typename T>
    static inline void txtCodeToString(DWORD code, T txt[5])
    {
        union {
            uint32_t    code;
            uint8_t     c[4];
        } word;
        word.code = code;

        int i = 0;
        for (; i < 4; i += 1) {
            if (word.c[i] == '\0') {
                break;
            }
            txt[i] = (T)word.c[i];
        }
        txt[i] = T('\0');
    }

    template <typename Size>
    static inline constexpr DWORD clipCode(DWORD code, Size sz)
    {
        uint8_t *   p = (uint8_t *)&code;
        for (int i = sz; i < 4; i += 1) {
            p[i] = '\0';
        }
        return code;
    }

    DEFINE_CODE(box);
    DEFINE_CODE(rvs);
    DEFINE_CODE(rvl);

public:
    D2Item(UnitAny * item = nullptr): mItem(item) { }

    Storage     storage();
    bool        isEquipBelt();
    int         beltRows();

    int         x()
    {
        if (mItem->pItemPath == nullptr) {
            return -1;
        }
        return mItem->pItemPath->dwPosX;
    }

    DWORD       id()
    {
        return mItem->dwUnitId;
    }

    int         y()
    {
        if (mItem->pItemPath == nullptr) {
            return -1;
        }
        return mItem->pItemPath->dwPosY;
    }

    DWORD       txtFileNo()
    {
        return mItem->dwTxtFileNo;
    }

    DWORD       code()
    {
        auto    txt = itemTxt();
        if (txt == nullptr) {
            return 0;
        }
        return txt->dwCode[0];
    }

    const wchar_t *     name()
    {
        ItemTxt *       txt = itemTxt();
        if (txt == nullptr) {
            return L"";
        }
        auto            name = D2GetLocaleText(txt->wLocaleTxtNo);
        if (name == nullptr) {
            return L"";
        }
        return name;
    }

    ItemTxt *           itemTxt()
    {
        return D2GetItemTxt(mItem->dwTxtFileNo);
    }

    inline void         show(LPCWSTR fmt, auto&&... args)
    {
        wchar_t     info[512];
        _snwprintf(info, sizeof(info) / sizeof(info[0]),
                   fmt, std::forward<decltype(args)>(args)...);
        D2Util::showInfo(L"%s(%d): %s", name(), txtFileNo(), info);
    }

    inline void         verbose(LPCWSTR fmt, auto&&... args)
    {
        if (D2Util::gVerbose) {
            show(fmt, std::forward<decltype(args)>(args)...);
        }
    }

    inline void         debug(LPCWSTR fmt, auto&&... args)
    {
        if (D2Util::gDebug) {
            show(fmt, std::forward<decltype(args)>(args)...);
        }
    }

    bool                stackable()
    {
        if (mItem->pItemData->dwQuality != ITEM_QUALITY_NORMAL) {
            return false;
        }
        auto txt = itemTxt();
        if (txt == nullptr) {
            return false;
        }
        return txt->Stackable != 0;
    }

    DWORD               quantity()
    {
        DWORD stat = D2GetUnitStat(mItem, STAT_AMMOQUANTITY, 0);
        return stat;
    }

    DWORD               maxStack()
    {
        auto    txt = itemTxt();
        if (txt == nullptr) {
            return 0;
        }
        return txt->dwMaxStack;
    }

    bool                stackablesEqual(UnitAny * pItem)
    {
        //show(L"qual %d stackable %d", mItem->pItemData->dwQuality, itemTxt()->Stackable);
        if (!stackable()) {
            return false;
        }
        if (txtFileNo() != pItem->dwTxtFileNo) {
            //show(L"Txt: %d item %d", txtFileNo(), pItem->dwTxtFileNo);
            return false;
        }
        if (mItem->pItemData->dwQuality != pItem->pItemData->dwQuality) {
            //show(L"qual: %d item %d", mItem->pItemData->dwQuality, mItem->pItemData->dwQuality);
            return false;
        }
        if (mItem->pItemData->dwFileIndex != pItem->pItemData->dwFileIndex) {
            //show(L"file idx: %d item %d", mItem->pItemData->dwFileIndex, mItem->pItemData->dwFileIndex);
            return false;
        }

        if (quantity() >= maxStack()) {
            //show(L"Full, quantity %d max %d", quantity(), maxStack());
            return false;
        }

        if (0) {
            show(L"stackable, id %d, %d(%d) !!!", mItem->dwUnitId, quantity(), maxStack());
        }

        return true;
    }


protected:
    UnitAny *           mItem;
};

class Bag: public D2Item {
public:
    static int          TxtFileNo;
    static DWORD        Row;
    static DWORD        Col;
    static bool         avaliable()
    {
        return TxtFileNo > 0;
    }
    static void         setBagNumber(int number, DWORD row=3, DWORD col=3)
    {
        TxtFileNo = number;
        Row = row;
        Col = col;
    }

    static int          checkGoodBag(UnitAny * ptItem, DWORD txtNeedImport)
    {
        if (!avaliable()) {
            return -1;
        }

        D2Item  item(ptItem);

        if (!isBag(item)) {
            item.verbose(L"Not a bag");
            return -1;
        }
        Bag     bag(ptItem);
        int     ret = bag.checkItem(txtNeedImport);
        if (ret < 0) {
            bag.verbose(L"has no %u", txtNeedImport);
            return -1;
        }
        item.verbose(L"Bag found");
        return ret;
    }

    static bool         isBag(D2Item item)
    {
        return item.txtFileNo() == TxtFileNo;
    }

    Bag(UnitAny * item): D2Item(item) { }

    int                 checkItem(DWORD itemTxt)
    {
        StatList *      statList = D2GetItemStatList(mItem, 0x40);
        if (statList == nullptr) {
            return -1;
        }
        auto            ptr = (const StatEx *)statList->sBaseStat.pStat;
        for (int i = 0; i < statList->sBaseStat.wStats; i += 1) {
            DWORD       txt;
            if (ptr[i].wParam != 0) {
                txt = ptr[i].wParam;
            } else {
                txt = ptr[i].dwStatValue & 0xffff;
            }

            debug(L"  %d: %d %d %d txt %d\n", i, ptr[i].wParam, ptr[i].wStatId, ptr[i].dwStatValue, txt);
            if (txt == itemTxt) {
                debug(L"Bag has %u at %i slot", itemTxt, i);
                return i;
            }
        }
        return -1;
    }

    void                toCoord(int pos, int& x, int& y)
    {
        x = pos % Row;
        y = pos / Row;
    }

    void                activate()
    {
        int         posX;
        int         posY;

        if (PLAYER == nullptr) {
            return;
        }
        posX = PLAYER->pMonPath->wPosX + 2;
        posY = PLAYER->pMonPath->wPosY + 2;

        BYTE aPacket[13] = { 0 };
        aPacket[0] = 0x20;
        *(DWORD *)(&aPacket[1]) = id();
        *(DWORD *)(&aPacket[5]) = posX;
        *(DWORD *)(&aPacket[9]) = posY;
        D2SendPacket(13, 0, aPacket);
    }
};

class Potion: public D2Item {
public:
    enum Type {
        Unknown = 0,
        Red,
        Blue,
        Purple,
        Junk,

    };
    class TypeMask {
    public:
        static constexpr int    AllPotion = ((1 << (int)Red)) | (1 << (int)Blue) | (1 << (int)Purple);

        explicit TypeMask(int mask = 0): mMask(mask) { }
        explicit TypeMask(Type type): mMask(1 << (int)type) { }

        TypeMask&       add(Type type)
        {
            mMask |= (1 << (int)type);
            return *this;
        }

        int             mask() const
        {
            return mMask;
        }

        bool            isEmpty() const
        {
            return mMask == 0;
        }

        bool            hasType(Type type) const
        {
            return (mMask & (1 << (int)type)) != 0;
        }

        void            setPotionOnly()
        {
            mMask &= AllPotion;
        }

        void            setAllPotion()
        {
            mMask = AllPotion;
        }


    private:
        int     mMask;
    };

    static Type         getType(UnitAny * item);
    static bool         isPotionType(Type type)
    {
        return type == Red || type == Blue || type == Purple;
    }
    static bool         isPotion(UnitAny * item)
    {
        return isPotionType(getType(item));
    }
    static bool         isBigPurple(UnitAny * item);

public:
    Potion(UnitAny * item, Type type) : D2Item(item), mType(type) { }

    bool                isHpPotion() { return mType == Red || mType == Purple; }
    bool                isMpPotion() { return mType == Blue || mType == Purple; }
    Type                type() { return mType; }

private:
    Type                mType;
};
