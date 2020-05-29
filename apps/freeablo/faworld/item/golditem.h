#pragma once
#include "item.h"

namespace FAWorld
{
    class GoldItemBase;

    class GoldItem final : public Item2
    {
        using super = Item2;

    public:
        explicit GoldItem(const GoldItemBase* base);

        void save(FASaveGame::GameSaver& saver) const override { super::save(saver); }
        void load(FASaveGame::GameLoader& loader) override { super::load(loader); }

        GoldItem* getAsGoldItem() override { return this; }

        const GoldItemBase* getBase() const;

        std::string getFullDescription() const override;

        int32_t getCount() const { return mCount; }
        bool trySetCount(int32_t newCount);

    private:
        int32_t mCount = 1;
    };
}
