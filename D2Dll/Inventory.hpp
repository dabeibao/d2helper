#pragma once

#include <stdint.h>
#include "d2ptrs.h"
#include "d2structs.h"
#include "D2Item.hpp"
#include "D2Utils.hpp"
#include "Define.h"

struct Inventory {
    template<typename Func>
    static void         forEach(Func&& f)
    {
        for (UnitAny *ptItem = D2GetFirstItemInInv(PLAYER->pInventory); ptItem; ptItem = D2GetNextItemInInv(ptItem)) {
            bool        done = f(ptItem);
            if (done) {
                break;
            }
        }
    }

    template<typename Func>
    static void         forEachInInv(Func&& f)
    {
        for (UnitAny *ptItem = D2GetFirstItemInInv(PLAYER->pInventory); ptItem; ptItem = D2GetNextItemInInv(ptItem)) {
            D2Item      item(ptItem);

            if (item.storage() == StorageInventory) {
                bool    done = f(ptItem);
                if (done) {
                    break;
                }
            }
        }
    }
};

class Belt {
public:
    static UnitAny *            GetBelt()
    {
        UnitAny *   belt = nullptr;

        Inventory::forEach([&](UnitAny * ptItem) -> bool {
            D2Item          item(ptItem);

            if (item.isEquipBelt()) {
                belt = ptItem;
                return true;
            }
            return false;
        });
        return belt;
    }

    static unsigned int         GetBeltRows()
    {
        UnitAny * belt = GetBelt();
        if (belt == nullptr) {
            return 1;
        }

        D2Item      beltItem(belt);

        int         rows = beltItem.beltRows();
        return rows;
    }

    Belt(bool isFill = true)
    {
        reset(GetBeltRows());
        if (isFill) {
            fill();
        }
    }

    int                 rows() const
    {
        return mRowNumber;
    }

    int                 columns() const
    {
        return 4;
    }

    bool                isFull() const
    {
        return mItemNumber >= rows() * columns();
    }


    void                reset(int rows = 1)
    {
        mRowNumber = rows <= 4? rows : 4;
        memset(&mRows, 0, sizeof(mRows));
        mItemNumber = 0;
    }

    int                 fill()
    {
        int             items = 0;

        Inventory::forEach([&](UnitAny * ptItem) -> bool {
            bool        ok = addItem(ptItem);
            if (ok) {
                items += 1;
            }
            return false;
        });
        mItemNumber = items;
        return items;
    }

    bool                addItem(UnitAny * ptItem)
    {
        D2Item  item(ptItem);
        if (item.storage() != StorageBelt) {
            return false;
        }
        int     pos = item.x();
        if (pos < 0 || pos >= 16) {
            return false;
        }
        int     row = pos / 4;
        int     col = pos % 4;

        auto&   element = mRows[row].elements[col];
        element.code = item.code();
        element.type = Potion::getType(ptItem);

        return true;
    }

    Potion::TypeMask    getFreePotions()
    {
        Potion::TypeMask        mask;

        if (isFull()) {
            return mask;
        }

        for (int col = 0; col < 4; col += 1) {
            if (mRows[0].elements[col].isEmpty()) {
                D2Util::showDebug(L"OK: Col %d is empty", col);
                // Empty column
                mask.setAllPotion();
                break;
            }
            auto        type = mRows[0].elements[col].type;
            for (int row = 1; row < mRowNumber; row += 1) {
                if (mRows[row].elements[col].isEmpty()) {
                    D2Util::showDebug(L"OK: Col %d has free slot at row %d", col, row);
                    mask.add(type);
                    break;
                }
            }
        }
        mask.setPotionOnly();
        return mask;
    }

    bool                hasSlot(Potion::Type potion)
    {
        if (isFull()) {
            return false;
        }

        for (int col = 0; col < 4; col += 1) {
            if (mRows[0].elements[col].isEmpty()) {
                D2Util::showDebug(L"OK: Col %d is empty", col);
                // Empty column
                return true;
            }
            auto        type = mRows[0].elements[col].type;
            if (type != potion) {
                continue;
            }
            for (int row = 1; row < mRowNumber; row += 1) {
                if (mRows[row].elements[col].isEmpty()) {
                    D2Util::showDebug(L"OK: Col %d has free slot at row %d", col, row);
                    return true;
                }
            }
        }

        return false;
    }

    bool                canPutToBelt(UnitAny * ptItem)
    {
        D2Item          item(ptItem);
        Potion::Type    potion = Potion::getType(ptItem);

        if (!Potion::isPotionType(potion)) {
            return false;
        }
        bool    ok = hasSlot(potion);

        item.debug(L"Potion %d can be put %d", potion, ok);

        return ok;
    }

private:
    struct Element {
        Potion::Type    type;
        DWORD           code;

        bool isEmpty() { return code == 0; }
    };

    struct Row {
        Element elements[4];
    };

    int         mRowNumber;
    Row         mRows[4];
    int         mItemNumber;
};
