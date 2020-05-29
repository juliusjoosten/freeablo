#include "inventory.h"
#include "../fagui/guimanager.h"
#include "../fasavegame/gameloader.h"
#include "../faworld/actorstats.h"
#include "actorstats.h"
#include "equiptarget.h"
#include "item/equipmentitem.h"
#include "item/equipmentitembase.h"
#include "item/itembase.h"
#include "item/usableitem.h"
#include "item/usableitembase.h"
#include "itemenums.h"
#include "itemfactory.h"
#include "player.h"
#include <algorithm>
#include <cstdint>
#include <engine/enginemain.h>
#include <faworld/item/golditem.h>
#include <faworld/item/golditembase.h>
#include <iostream>
#include <sstream>
#include <string>

namespace FAWorld
{
    BasicInventory::BasicInventory(int32_t width, int32_t height, bool treatAllIItemsAs1by1)
        : mInventoryBox(width, height), mTreatAllItemsAs1by1(treatAllIItemsAs1by1)
    {
        for (int32_t y = 0; y < mInventoryBox.height(); y++)
        {
            for (int32_t x = 0; x < mInventoryBox.width(); x++)
                mInventoryBox.get(x, y).position = {x, y};
        }
    }

    void BasicInventory::save(FASaveGame::GameSaver& saver) const
    {
        Serial::ScopedCategorySaver cat("BasicInventory", saver);

        saver.save(mInventoryBox.width());
        saver.save(mInventoryBox.height());

        for (const auto& box : *this)
        {
            debug_assert(box.item && box.isReal && box.topLeft == box.position);

            saver.save(true);
            box.position.save(saver);
            Engine::EngineMain::get()->mWorld->getItemFactory().saveItem(*box.item, saver);
        }
        saver.save(false);

        saver.save(mTreatAllItemsAs1by1);
    }

    void BasicInventory::load(FASaveGame::GameLoader& loader)
    {
        int32_t width = loader.load<int32_t>();
        int32_t height = loader.load<int32_t>();

        mInventoryBox = Misc::Array2D<BasicInventoryBox>(width, height);
        for (int32_t y = 0; y < mInventoryBox.height(); y++)
        {
            for (int32_t x = 0; x < mInventoryBox.width(); x++)
                mInventoryBox.get(x, y).position = {x, y};
        }

        while (loader.load<bool>())
        {
            Vec2i position(loader);

            std::unique_ptr<Item2> item = Engine::EngineMain::get()->mWorld->getItemFactory().loadItem(loader);
            PlaceItemResult result = placeItem(item, position.x, position.y);

            if (!result.succeeded())
                printf("Couldn't fit loaded item %s", item->getBase()->mId.c_str());
        }

        mTreatAllItemsAs1by1 = loader.load<bool>();
    }

    bool BasicInventory::canFitItem(const Item2& item) const
    {
        Vec2i itemSize = item.getBase()->mSize;
        if (mTreatAllItemsAs1by1)
            itemSize = Vec2i(1, 1);

        for (int32_t y = 0; y < mInventoryBox.height() - itemSize.y; y++)
        {
            for (int32_t x = 0; x < mInventoryBox.width() - itemSize.x; x++)
            {
                bool success = true;

                for (int32_t yy = y; yy < y + itemSize.y; yy++)
                {
                    for (int32_t xx = x; xx < x + itemSize.x; xx++)
                    {
                        if (mInventoryBox.get(xx, yy).item)
                        {
                            success = false;
                            break;
                        }
                    }
                }

                if (success)
                    return true;
            }
        }

        return false;
    }

    bool BasicInventory::autoPlaceItem(std::unique_ptr<Item2>& item, PlacementCheckOrder order)
    {
        if (order == PlacementCheckOrder::Automatic)
        {
            order = PlacementCheckOrder::FromLeftTop;
            if (item->getBase()->mSize == Vec2i{1, 1})
                order = PlacementCheckOrder::FromLeftBottom;
            else if (item->getBase()->mSize == Vec2i{2, 2})
                order = PlacementCheckOrder::SpecialFor2x2;
            else if (item->getBase()->mSize == Vec2i{1, 2})
                order = PlacementCheckOrder::SpecialFor1x2;
        }

        switch (order)
        {
            case PlacementCheckOrder::FromLeftBottom:
                for (int32_t y = mInventoryBox.height() - 1; y != -1; y--)
                    for (int32_t x = 0; x != mInventoryBox.width(); x++)
                        if (placeItem(item, x, y).succeeded())
                            return true;
                break;
            case PlacementCheckOrder::FromLeftTop:
                for (int32_t y = 0; y != mInventoryBox.height(); y++)
                    for (int32_t x = 0; x != mInventoryBox.width(); x++)
                        if (placeItem(item, x, y).succeeded())
                            return true;
                break;
            case PlacementCheckOrder::FromRightBottom:
                for (int32_t y = mInventoryBox.height() - 1; y != -1; y--)
                    for (int32_t x = mInventoryBox.width() - 1; x != -1; x--)
                        if (placeItem(item, x, y).succeeded())
                            return true;
                break;
            case PlacementCheckOrder::SpecialFor1x2:
                for (int32_t y = mInventoryBox.height() - 2; y != -1; y -= 2)
                    for (int32_t x = mInventoryBox.width() - 1; x != -1; x--)
                        if (placeItem(item, x, y).succeeded())
                            return true;
                for (int32_t y = mInventoryBox.height() - 3; y != -1; y -= 2)
                    for (int32_t x = mInventoryBox.width() - 1; x != -1; x--)
                        if (placeItem(item, x, y).succeeded())
                            return true;
                break;
            case PlacementCheckOrder::SpecialFor2x2:
                // this way lies madness
                for (int32_t x = mInventoryBox.width() - 2; x != -1; x -= 2)
                    for (int32_t y = 0; y != mInventoryBox.height(); x += 2)
                        if (placeItem(item, x, y).succeeded())
                            return true;
                for (int32_t y = mInventoryBox.height() - 2; y != -1; y -= 2)
                    for (int32_t x = 1; x != mInventoryBox.width(); x += 2)
                        if (placeItem(item, x, y).succeeded())
                            return true;
                for (int32_t y = 1; y != mInventoryBox.height(); y += 2)
                    for (int32_t x = 0; x != mInventoryBox.width(); x++)
                        if (placeItem(item, x, y).succeeded())
                            return true;
                break;
            default:
                invalid_enum(PlacementCheckOrder, order);
        }
        return false;
    }

    PlaceItemResult BasicInventory::placeItem(std::unique_ptr<Item2>& item, int32_t x, int32_t y)
    {
        Vec2i itemSize = item->getBase()->mSize;
        if (mTreatAllItemsAs1by1)
            itemSize = Vec2i(1, 1);

        // Shift item slot to the closest available that will fit the item.
        // This removes the need for "out of bounds" placement and gives a more intuitive feel
        // when placing an item near the edge of the inventory.
        x = std::max(x, 0);
        y = std::max(y, 0);
        x = std::min(x, mInventoryBox.width() - itemSize.x);
        y = std::min(y, mInventoryBox.height() - itemSize.y);

        std::set<BasicInventoryBox*> blockingItems;

        for (int32_t yy = y; yy < y + itemSize.y; yy++)
        {
            for (int32_t xx = x; xx < x + itemSize.x; xx++)
            {
                BasicInventoryBox& cell = mInventoryBox.get(xx, yy);

                if (cell.item)
                {
                    BasicInventoryBox& topLeftItemCell = mInventoryBox.get(cell.topLeft.x, cell.topLeft.y);
                    blockingItems.insert(&topLeftItemCell);
                }
            }
        }

        if (!blockingItems.empty())
            return PlaceItemResult{PlaceItemResult::Type::BlockedByItems, blockingItems};

        Item2* itemReleased = item.release();
        for (int32_t yy = y; yy < y + itemSize.y; yy++)
        {
            for (int32_t xx = x; xx < x + itemSize.x; xx++)
            {
                BasicInventoryBox& cell = mInventoryBox.get(xx, yy);

                cell.topLeft = {x, y};
                cell.item = itemReleased;
                cell.isReal = false;
            }
        }

        mInventoryBox.get(x, y).isReal = true;

        if (mInventoryChanged)
            mInventoryChanged(nullptr, itemReleased);

        return PlaceItemResult{PlaceItemResult::Type::Success, {}};
    }

    bool BasicInventory::swapItem(std::unique_ptr<Item2>& item, int32_t x, int32_t y)
    {
        if (!item)
        {
            item = remove(x, y);
            return item == nullptr;
        }

        PlaceItemResult result = placeItem(item, x, y);

        switch (result.type)
        {
            case PlaceItemResult::Type::Success:
            {
                return true;
            }
            case PlaceItemResult::Type::BlockedByItems:
            {
                if (result.blockingItems.size() == 1)
                {
                    std::unique_ptr<Item2> tmp = std::move(item);
                    Vec2i removeFrom = (*result.blockingItems.begin())->topLeft;
                    item = remove(removeFrom.x, removeFrom.y);
                    auto finalPlaceResult = placeItem(tmp, x, y);
                    release_assert(finalPlaceResult.succeeded());
                    return true;
                }

                return false;
            }
            case PlaceItemResult::Type::OutOfBounds:
            {
                return false;
            }
        }

        invalid_enum(PlaceItemResult, result.type);
    }

    std::unique_ptr<Item2> BasicInventory::remove(int32_t x, int32_t y)
    {
        BasicInventoryBox result = mInventoryBox.get(x, y);

        if (!result.item)
            return nullptr;

        Vec2i itemSize = result.item->getBase()->mSize;
        if (mTreatAllItemsAs1by1)
            itemSize = Vec2i(1, 1);

        for (int32_t yLocal = result.topLeft.y; yLocal < result.topLeft.y + itemSize.y; ++yLocal)
            for (int32_t xLocal = result.topLeft.x; xLocal < result.topLeft.x + itemSize.x; ++xLocal)
            {
                BasicInventoryBox& box = mInventoryBox.get(xLocal, yLocal);
                box.item = nullptr;
                box.isReal = false;
                box.topLeft = Vec2i::invalid();
            }

        if (mInventoryChanged)
            mInventoryChanged(result.item, nullptr);

        return std::unique_ptr<Item2>(result.item);
    }

    struct ExchangeResult
    {
        std::set<EquipTarget> NeedsToBeReplaced;
        std::set<EquipTarget> NeedsToBeReturned; // used only for equipping 2-handed weapon while wearing 1h weapon + shield
        std::optional<EquipTarget> newTarget;    // sometimes target changes during exchange
        explicit ExchangeResult(std::set<EquipTarget> NeedsToBeReplacedArg = {},
                                std::set<EquipTarget> NeedsToBeReturnedArg = {},
                                const std::optional<EquipTarget>& newTargetArg = {});
    };

    ExchangeResult::ExchangeResult(std::set<EquipTarget> NeedsToBeReplacedArg,
                                   std::set<EquipTarget> NeedsToBeReturnedArg,
                                   const std::optional<EquipTarget>& newTargetArg)
        : NeedsToBeReplaced(std::move(NeedsToBeReplacedArg)), NeedsToBeReturned(std::move(NeedsToBeReturnedArg)), newTarget(newTargetArg)
    {
    }

    CharacterInventory::CharacterInventory()
    {
        for (const auto& pair : mInventoryTypes)
        {
            EquipTargetType type = pair.first;
            pair.second.mInventoryChanged = [this, type](const Item2* removed, const Item2* added) {
                if (mInventoryChanged)
                    mInventoryChanged(type, removed, added);
            };
        }
    }

    void CharacterInventory::save(FASaveGame::GameSaver& saver) const
    {
        mMainInventory.save(saver);
        mBelt.save(saver);
        mHead.save(saver);
        mBody.save(saver);
        mLeftRing.save(saver);
        mRightRing.save(saver);
        mAmulet.save(saver);
        mLeftHand.save(saver);
        mRightHand.save(saver);
        mCursorHeld.save(saver);
    }

    void CharacterInventory::load(FASaveGame::GameLoader& loader)
    {
        mMainInventory.load(loader);
        mBelt.load(loader);
        mHead.load(loader);
        mBody.load(loader);
        mLeftRing.load(loader);
        mRightRing.load(loader);
        mAmulet.load(loader);
        mLeftHand.load(loader);
        mRightHand.load(loader);
        mCursorHeld.load(loader);
    }

    bool CharacterInventory::autoPlaceItem(std::unique_ptr<Item2>&& item) { return autoPlaceItem(item, PlacementCheckOrder::Automatic); }

    bool CharacterInventory::autoPlaceItem(std::unique_ptr<Item2>& item, PlacementCheckOrder order)
    {
        // auto-placing in belt
        if (item->getAsUsableItem() && item->getAsUsableItem()->getBase()->isBeltEquippable() && mBelt.autoPlaceItem(item, order))
            return true;

        if (EquipmentItem* equipmentItem = item->getAsEquipmentItem())
        {
            // auto-equipping two handed weapons
            const Item2* leftHand = mLeftHand.getItem(0, 0);
            const Item2* rightHand = mRightHand.getItem(0, 0);
            if (equipmentItem->getBase()->mEquipSlot == ItemEquipType::twoHanded && !leftHand && !rightHand)
            {
                release_assert(mLeftHand.autoPlaceItem(item));
                return true;
            }

            // auto equip one handed weapons
            if (equipmentItem->getBase()->mEquipSlot == ItemEquipType::oneHanded && equipmentItem->getBase()->mClass == ItemClass::weapon && !leftHand)
            {
                mLeftHand.autoPlaceItem(item);
                return true;
            }
        }

        if (GoldItem* goldItem = item->getAsGoldItem())
        {
            int32_t count = goldItem->getCount();
            count = placeGold(count, Engine::EngineMain::get()->mWorld->getItemFactory());

            if (count)
                release_assert(goldItem->trySetCount(count));
            else
                item.reset();
        }

        return mMainInventory.autoPlaceItem(item, order);
    }

    bool CharacterInventory::forcePlaceItem(std::unique_ptr<Item2>& item, const EquipTarget& target)
    {
        BasicInventory& inv = getInvMutable(target.type);
        PlaceItemResult result = inv.placeItem(item, target.posX, target.posY);

        switch (result.type)
        {
            case PlaceItemResult::Type::Success:
                return true;

            case PlaceItemResult::Type::OutOfBounds:
                return false;

            case PlaceItemResult::Type::BlockedByItems:
            {
                for (const BasicInventoryBox* block : result.blockingItems)
                    inv.remove(block->topLeft.x, block->topLeft.y);

                result = inv.placeItem(item, target.posX, target.posY);
                release_assert(result.succeeded());
                return true;
            }
        }

        invalid_enum(PlaceItemResult, result.type);
    }

    const Item2* CharacterInventory::getItemAt(const EquipTarget& target) const { return getInv(target.type).getItem(target.posX, target.posY); }

    std::unique_ptr<Item2> CharacterInventory::remove(const EquipTarget& target) { return getInvMutable(target.type).remove(target.posX, target.posY); }

    void CharacterInventory::setCursorHeld(std::unique_ptr<Item2>&& item)
    {
        mCursorHeld.remove(0, 0);
        if (item)
            release_assert(mCursorHeld.placeItem(item, 0, 0).succeeded());
    }

    const BasicInventory& CharacterInventory::getInv(EquipTargetType type) const { return mInventoryTypes.at(type); }

    BasicInventory& CharacterInventory::getInvMutable(EquipTargetType type) { return mInventoryTypes.at(type); }

    void CharacterInventory::slotClicked(const EquipTarget& slot)
    {
        const Item2* cursorItem = getCursorHeld();

        if (cursorItem)
        {
            bool ok = true;

            switch (slot.type)
            {
                case EquipTargetType::inventory:
                    break;
                case EquipTargetType::belt:
                    ok = cursorItem->getAsMiscItem() && cursorItem->getAsMiscItem()->getBase()->isBeltEquippable();
                    break;
                case EquipTargetType::head:
                    ok = cursorItem->getBase()->getEquipType() == ItemEquipType::head;
                    break;
                case EquipTargetType::body:
                    ok = cursorItem->getBase()->getEquipType() == ItemEquipType::chest;
                    break;
                case EquipTargetType::leftRing:
                case EquipTargetType::rightRing:
                    ok = cursorItem->getBase()->getEquipType() == ItemEquipType::ring;
                    break;
                case EquipTargetType::leftHand:
                case EquipTargetType::rightHand:
                    ok = cursorItem->getBase()->getEquipType() == ItemEquipType::oneHanded || cursorItem->getBase()->getEquipType() == ItemEquipType::twoHanded;
                    break;
                case EquipTargetType::amulet:
                    ok = cursorItem->getBase()->getEquipType() == ItemEquipType::amulet;
                    break;
                default:
                    invalid_enum(EquipTargetType, slot.type);
            }

            if (!ok)
                return;

            // Handle weapons and shields.
            // Weapons always go in left hand, shields always in right.
            // 2 handed weapons need both hands free.
            if (slot.type == EquipTargetType::leftHand || slot.type == EquipTargetType::rightHand)
            {
                if (cursorItem->getBase()->getEquipType() == ItemEquipType::twoHanded)
                {
                    if (!getRightHand() || !getLeftHand() || getLeftHand()->getBase()->getEquipType() == ItemEquipType::twoHanded)
                    {
                        std::unique_ptr<Item2> removed;
                        if (getRightHand())
                            removed = mRightHand.remove(0, 0);
                        if (getLeftHand())
                            removed = mLeftHand.remove(0, 0);

                        std::unique_ptr<Item2> cursorOld = mCursorHeld.remove(0, 0);
                        release_assert(mLeftHand.placeItem(cursorOld, 0, 0).succeeded());
                        setCursorHeld(std::move(removed));
                    }
                }
                else if (cursorItem->getBase()->getEquipType() == ItemEquipType::oneHanded)
                {
                    std::unique_ptr<Item2> removed;
                    if (getLeftHand() && getLeftHand()->getBase()->getEquipType() == ItemEquipType::twoHanded)
                        removed = mLeftHand.remove(0, 0);
                    else if (cursorItem->getBase()->mClass == ItemClass::weapon)
                        removed = mLeftHand.remove(0, 0);
                    else if (cursorItem->getBase()->mType == ItemType::shield)
                        removed = mRightHand.remove(0, 0);

                    std::unique_ptr<Item2> cursorOld = mCursorHeld.remove(0, 0);

                    if (cursorItem->getBase()->mClass == ItemClass::weapon)
                        release_assert(mLeftHand.placeItem(cursorOld, 0, 0).succeeded());
                    else if (cursorItem->getBase()->mType == ItemType::shield)
                        release_assert(mRightHand.placeItem(cursorOld, 0, 0).succeeded());

                    setCursorHeld(std::move(removed));
                }

                return;
            }
        }

        BasicInventory& inv = getInvMutable(slot.type);

        std::unique_ptr<Item2> tmp = mCursorHeld.remove(0, 0);
        inv.swapItem(tmp, slot.posX, slot.posY);

        setCursorHeld(std::move(tmp));
    }

    void CharacterInventory::calculateItemBonuses(ItemStats& stats) const
    {
        EquipTarget hands[] = {MakeEquipTarget<EquipTargetType::leftHand>(), MakeEquipTarget<EquipTargetType::rightHand>()};
        for (auto& slot : hands)
        {
            const EquipmentItem* item = getItemAt(slot) ? getItemAt(slot)->getAsEquipmentItem() : nullptr;
            if (!item)
                continue;

            if (Item::isItemAMeleeWeapon(item->getBase()->mType))
                stats.meleeDamageBonusRange += item->getBase()->mDamageBonusRange;
            else if (Item::isItemARangedWeapon(item->getBase()->mType))
                stats.rangedDamageBonusRange += item->getBase()->mDamageBonusRange;

            // TODO: other stats
        }
    }

    bool CharacterInventory::isRangedWeaponEquipped() const
    {
        EquipTarget hands[] = {MakeEquipTarget<EquipTargetType::leftHand>(), MakeEquipTarget<EquipTargetType::rightHand>()};
        for (auto& slot : hands)
        {
            const Item2* item = getItemAt(slot);
            if (item && Item::isItemARangedWeapon(item->getBase()->mType))
                return true;
        }

        return false;
    }

    bool CharacterInventory::isShieldEquipped() const
    {
        return (getLeftHand() && getLeftHand()->getBase()->mType == ItemType::shield) ||
               (getRightHand() && getRightHand()->getBase()->mType == ItemType::shield);
    }

    EquippedInHandsItems CharacterInventory::getItemsInHands() const
    {
        EquippedInHandsItems retval = {};

        EquipTarget hands[] = {MakeEquipTarget<EquipTargetType::leftHand>(), MakeEquipTarget<EquipTargetType::rightHand>()};
        for (auto& slot : hands)
        {
            const Item2* item = getItemAt(slot);
            if (!item)
                continue;

            if (Item::isItemARangedWeapon(item->getBase()->mType))
            {
                retval.rangedWeapon = EquippedInHandsItems::TypeData{item->getAsEquipmentItem(), slot.type};
                retval.weapon = retval.rangedWeapon;
            }
            else if (Item::isItemAMeleeWeapon(item->getBase()->mType))
            {
                retval.meleeWeapon = EquippedInHandsItems::TypeData{item->getAsEquipmentItem(), slot.type};
                retval.weapon = retval.meleeWeapon;
            }
            else if (item->getBase()->mType == ItemType::shield)
            {
                retval.shield = EquippedInHandsItems::TypeData{item->getAsEquipmentItem(), slot.type};
            }
        }

        return retval;
    }

    int32_t CharacterInventory::placeGold(int32_t quantity, const ItemFactory& itemFactory)
    {
        if (quantity == 0)
            return 0;

        // first part - filling existing gold piles
        for (const auto& item : mMainInventory)
        {
            GoldItem* goldItem = item.item->getAsGoldItem();
            if (!goldItem)
                continue;

            int32_t room = goldItem->getBase()->mMaxCount - goldItem->getCount();
            if (room > 0)
            {
                int32_t toPlace = std::min(quantity, room);
                release_assert(goldItem->trySetCount(goldItem->getCount() + toPlace));

                quantity -= toPlace;
                if (quantity == 0)
                    return 0;
            }
        }
        // second part - filling the empty slots with gold
        for (int32_t x = 0; x != mMainInventory.width(); x++)
        {
            for (int32_t y = 0; y != mMainInventory.height(); x++)
            {
                if (!mMainInventory.getItem(x, y))
                {
                    std::unique_ptr<Item2> newItem = itemFactory.generateBaseItem(ItemId::gold);
                    GoldItem* goldItem = newItem->getAsGoldItem();

                    int32_t toPlace = std::min(quantity, goldItem->getBase()->mMaxCount);
                    release_assert(goldItem->trySetCount(toPlace));
                    release_assert(mMainInventory.placeItem(newItem, x, y).succeeded());

                    quantity -= toPlace;
                    if (quantity == 0)
                        return 0;
                }
            }
        }

        return quantity;
    }

    bool CharacterInventory::canFitGold(int32_t quantity) const
    {
        if (quantity == 0)
            return true;

        // first part - filling existing gold piles
        for (const auto& item : mMainInventory)
        {
            GoldItem* goldItem = item.item->getAsGoldItem();
            if (!goldItem)
                continue;

            int32_t room = goldItem->getBase()->mMaxCount - goldItem->getCount();
            quantity -= room;
            if (quantity <= 0)
                return true;
        }

        const GoldItemBase* goldItemBase =
            safe_downcast<const GoldItemBase*>(Engine::EngineMain::get()->mWorld->getItemFactory().getItemBaseHolder().get("gold"));

        // second part - filling the empty slots with gold
        for (int32_t x = 0; x != mMainInventory.width(); x++)
        {
            for (int32_t y = 0; y != mMainInventory.height(); x++)
            {
                if (!mMainInventory.getItem(x, y))
                {
                    quantity -= goldItemBase->mMaxCount;
                    if (quantity <= 0)
                        return true;
                }
            }
        }

        return false;
    }

    void CharacterInventory::takeOutGold(int32_t quantity)
    {
        for (const auto& item : mMainInventory)
        {
            GoldItem* goldItem = item.item->getAsGoldItem();
            if (!goldItem)
                continue;

            int32_t toTake = std::min(quantity, goldItem->getCount());
            int32_t newPileCount = goldItem->getCount() - toTake;

            release_assert(newPileCount >= 0);

            if (newPileCount == 0)
                mMainInventory.remove(item.position.x, item.position.y);
            else
                release_assert(goldItem->trySetCount(newPileCount));

            quantity -= toTake;
            if (quantity == 0)
                return;
        }

        message_and_abort("Not enough gold");
    }

    void CharacterInventory::splitGoldIntoCursor(int32_t x, int32_t y, int32_t amountToTransferToCursor, const ItemFactory& itemFactory)
    {
        std::unique_ptr<Item2> goldFromInventory = mMainInventory.remove(x, y);
        GoldItem* goldFromInventoryGoldItem = goldFromInventory->getAsGoldItem();
        release_assert(goldFromInventoryGoldItem);

        amountToTransferToCursor = std::min(goldFromInventoryGoldItem->getCount(), amountToTransferToCursor);
        int32_t newPileAmount = goldFromInventoryGoldItem->getCount() - amountToTransferToCursor;

        release_assert(newPileAmount >= 0);

        if (newPileAmount > 0)
        {
            release_assert(goldFromInventoryGoldItem->trySetCount(newPileAmount));
            mMainInventory.placeItem(goldFromInventory, x, y);
        }

        std::unique_ptr<Item2> cursorGold = itemFactory.generateBaseItem(ItemId::gold);
        release_assert(cursorGold->getAsGoldItem()->trySetCount(amountToTransferToCursor));

        setCursorHeld(std::move(cursorGold));
    }

    int32_t CharacterInventory::getTotalGold() const
    {
        int32_t total = 0;
        for (const auto& slot : mMainInventory)
        {
            if (slot.item->getAsGoldItem())
                total += slot.item->getAsGoldItem()->getCount();
        }
        return total;
    }
}
