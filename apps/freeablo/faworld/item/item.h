#pragma once
#include <cstdint>
#include <fagui/menuentry.h>
#include <misc/misc.h>
#include <string>
#include <vector>

namespace FASaveGame
{
    class GameSaver;
    class GameLoader;
}

namespace FAWorld
{
    class EquipmentItem;
    class UsableItem;
    class GoldItem;

    class ItemBase;

    class Item
    {
    public:
        explicit Item(const ItemBase* base) : mBase(base) {}
        virtual ~Item() = default;

        virtual void init() {}

        virtual void save(FASaveGame::GameSaver& saver) const { UNUSED_PARAM(saver); }
        virtual void load(FASaveGame::GameLoader& loader) { UNUSED_PARAM(loader); }

        virtual EquipmentItem* getAsEquipmentItem() { return nullptr; }
        const EquipmentItem* getAsEquipmentItem() const { return const_cast<Item*>(this)->getAsEquipmentItem(); }
        virtual UsableItem* getAsUsableItem() { return nullptr; }
        const UsableItem* getAsMiscItem() const { return const_cast<Item*>(this)->getAsUsableItem(); }
        virtual GoldItem* getAsGoldItem() { return nullptr; }
        const GoldItem* getAsGoldItem() const { return const_cast<Item*>(this)->getAsGoldItem(); }

        int32_t getPrice() const;
        virtual std::string getFullDescription() const;
        virtual std::vector<FAGui::MenuEntry> descriptionForMerchants() const;

        const ItemBase* getBase() const { return mBase; }

    protected:
        const ItemBase* mBase = nullptr;
    };
}