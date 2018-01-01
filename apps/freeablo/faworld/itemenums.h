#pragma once

#include <cstdint>

namespace FAWorld
{
    enum class ItemEquipType : uint8_t
    {
        none,
        oneHanded,
        twoHanded,
        chest,
        head,
        ring,
        amulet,
        unequipable,
        belt,
        invalid,
    };

    enum class ItemType : uint8_t
    {
        misc = 0,
        sword,
        axe,
        bow,
        mace,
        shield,
        lightArmor,
        helm,
        mediumArmor,
        heavyArmor,
        staff,
        gold,
        ring,
        amulet,
    };

    enum class ItemClass : uint8_t
    {
        none = 0,
        weapon,
        armor,
        jewelryAndConsumable,
        gold,
        quest,
    };

    // TODO: synchronize names with:
    // https://github.com/sanctuary/notes/blob/72a0772e0d187d29117c4ddd6e6265cafc774a50/enums.h#L128
    enum class ItemEffectType : uint8_t
    {
        IncPercentChanceToHit,
        DecPercentChanceToHit,
        IncPercentDamageDone,
        DecPercentDamageDone,
        IncPercentDamageDoneChanceToHit,
        DecPercentDamageDoneChanceToHit,
        IncPercentArmourClass,
        DecPercentArmourClass,
        IncPercentResistFire,
        IncPercentResistLightning,
        IncPercentResistMagic,
        IncPercentResistAll,
        Unknown6,
        Unknown7,
        ModSpellLevel,
        IncCharges,
        IncFireDamage,
        IncLightningDamage,
        Unknown8,
        IncStrength,
        DecStrength,
        IncMagic,
        DecMagic,
        IncDexterity,
        DecDexterity,
        IncVitality,
        DecVitality,
        IncAllBasicStats,
        DecAllBasicStats,
        IncDamageTaken,
        DecDamageTaken,
        IncHP,
        DecHP,
        IncMana,
        DecMana,
        IncPercentDurability,
        DecPercentDurability,
        Indestructible,
        IncLightRadius,
        DecLightRadius,
        Unknown0,
        MultipleArrows,
        IncPercentFireArrowDamage,
        IncPercentLightningArrowDamage,
        UniquePicture,
        Thorns,
        AllMana,
        PlayerNoHeal,
        Unknown1,
        Unknown2,
        Unknown3,
        Unknown4,
        HalfTrapDamage,
        Knockback,
        MonsterNoHeal,
        PercentManaSteal,
        PercentLifeSteal,
        ArmourPenetration,
        AttackSpeed0,
        HitRecovery,
        FastBlock,
        IncDamageDone,
        RandomArrowSpeed,
        UnusualDamage,
        AlteredDurability,
        NoStrengthRequirment,
        Spell,
        AttackSpeed1,
        OneHanded,
        AntiDemon,
        ZeroAllResist,
        Unknown5,
        ConstantLifeDrain,
        PercentFixedLifeSteal,
        Infravision,
        SpecifiedArmourClass,
        IncHPWithArmourClass,
        IncArmourClassWithMana,
        IncFireResistWithLevel,
        DecArmourClass
    };

    enum class ItemMiscId
    {
        none = 0,
        potionOfFullHealing = 2,
        potionOfHealing = 3,
        potionOfMana = 6,
        potionOfFullMana = 7,
        elixirOfStrength = 10,
        elixirOfMagic = 11,
        elixirOfDexterity = 12,
        elixirOfVitality = 13,
        potionOfRejuvenation = 18,
        potionOfFullRejuvenation = 19,
        scroll = 21,
        scrollWithTarget = 22,
        staff = 23,
        book = 24,
        ring = 25,
        amulet = 26,
        unique = 27,
        mapOfTheStars = 42,
        ear = 43,
        spectralElixir = 44,
        invalid = -1,
    };

    enum class ItemQuality
    {
        normal = 0,
        magic,
        unique
    };

    enum class ItemId
    {
        gold = 0,
        shortSword = 1,
        buckler = 2,
        club = 3,
        shortBow = 4,
        shortStaffOfChargedBolt = 5,
        cleaver = 6,
        theUndeadCrown = 7,
        empyreanBand = 8,
        magicRock = 9,
        opticAmulet = 10,
        ringOfTruth = 11,
        tavernSign = 12,
        harlequinCrest = 13,
        veilOfSteel = 14,
        goldenElixir = 15,
        anvilOfFury = 16,
        blackMushroom = 17,
        brain = 18,
        fungalTome = 19,
        spectralElixir = 20,
        bloodStone = 21,
        mapOfTheStars = 22,
        heart = 23,
        potionOfHealing = 24,
        potionOfMana = 25,
        scrollOfIdentify = 26,
        scrollOfTownPortal = 27,
        arkainesValor = 28,
        potionOfFullHealing = 29,
        potionOfFullMana = 30,
        griswoldsEdge = 31,
        lightforge = 32,
        staffOfLazarus = 33,
        scrollOfResurrect = 34,
        null1 = 35,
        null2 = 36,
        null3 = 37,
        null4 = 38,
        null5 = 39,
        null6 = 40,
        null7 = 41,
        null8 = 42,
        null9 = 43,
        null10 = 44,
        null11 = 45,
        null12 = 46,
        null13 = 47,
        baseCap = 48,
        baseSkullCap = 49,
        baseHelm = 50,
        baseFullHelm = 51,
        baseCrown = 52,
        baseGreatHelm = 53,
        baseCape = 54,
        baseRags = 55,
        baseCloak = 56,
        baseRobe = 57,
        baseQuiltedArmor = 58,
        baseLeatherArmor = 59,
        baseHardLeatherArmor = 60,
        baseStuddedLeatherArmor = 61,
        baseRingMail = 62,
        baseChainMail = 63,
        baseScaleMail = 64,
        baseBreastPlate = 65,
        baseSplintMail = 66,
        basePlateMail = 67,
        baseFieldPlate = 68,
        baseGothicPlate = 69,
        baseFullPlateMail = 70,
        baseBuckler = 71,
        baseSmallShield = 72,
        baseLargeShield = 73,
        baseKiteShield = 74,
        baseTowerShield = 75,
        baseGothicShield = 76,
        basePotionOfHealing = 77,
        basePotionOfFullHealing = 78,
        basePotionOfMana = 79,
        basePotionOfFullMana = 80,
        basePotionOfRejuvenation = 81,
        basePotionOfFullRejuvenation = 82,
        baseElixirOfStrength = 83,
        baseElixirOfMagic = 84,
        baseElixirOfDexterity = 85,
        baseElixirOfVitality = 86,
        baseScrollOfHealing = 87,
        baseScrollOfLightning = 88,
        baseScrollOfIdentify = 89,
        baseScrollOfResurrect = 90,
        baseScrollOfFireWall = 91,
        baseScrollOfInferno = 92,
        baseScrollOfTownPortal = 93,
        baseScrollOfFlash = 94,
        baseScrollOfInfravision = 95,
        baseScrollOfPhasing = 96,
        baseScrollOfManaShield = 97,
        baseScrollOfFlameWave = 98,
        baseScrollOfFireball = 99,
        baseScrollOfStoneCurse = 100,
        baseScrollOfChainLightning = 101,
        baseScrollOfGuardian = 102,
        baseNonItem = 103,
        baseScrollOfNova = 104,
        baseScrollOfGolem = 105,
        baseScrollOfNone = 106,
        baseScrollOfTeleport = 107,
        baseScrollOfApocalypse = 108,
        baseBookQlvl2 = 109,
        baseBookQlvl8 = 110,
        baseBookQlvl14 = 111,
        baseBookQlvl20 = 112,
        baseDagger = 113,
        baseShortSword = 114,
        baseFalchion = 115,
        baseScimitar = 116,
        baseClaymore = 117,
        baseBlade = 118,
        baseSabre = 119,
        baseLongSword = 120,
        baseBroadSword = 121,
        baseBastardSword = 122,
        baseTwoHandedSword = 123,
        baseGreatSword = 124,
        baseSmallAxe = 125,
        baseAxe = 126,
        baseLargeAxe = 127,
        baseBroadAxe = 128,
        baseBattleAxe = 129,
        baseGreatAxe = 130,
        baseMace = 131,
        baseMorningStar = 132,
        baseWarHammer = 133,
        baseSpikedClub = 134,
        baseClub = 135,
        baseFlail = 136,
        baseMaul = 137,
        baseShortBow = 138,
        baseHuntersBow = 139,
        baseLongBow = 140,
        baseCompositeBow = 141,
        baseShortBattleBow = 142,
        baseLongBattleBow = 143,
        baseShortWarBow = 144,
        baseLongWarBow = 145,
        baseShortStaff = 146,
        baseLongStaff = 147,
        baseCompositeStaff = 148,
        baseQuarterStaff = 149,
        baseWarStaff = 150,
        baseRingQlvl5 = 151,
        baseRingQlvl10 = 152,
        baseRingQlvl15 = 153,
        baseAmuletQlvl8 = 154,
        baseAmuletQlvl16 = 155,
        null14 = 156,
    };

    enum class UniqueItemId
    {
        theButchersCleaver = 0,
        theUndeadCrown,
        empyreanBand,
        opticAmulet,
        ringOfTruth,
        harlequinCrest,
        veilOfSteel,
        arkainesValor,
        griswoldsEdge,
        lightforge,
        theRiftBow,
        theNeedler,
        theCelestialBow,
        deadlyHunter,
        bowOfTheDead,
        theBlackoakBow,
        flamedart,
        fleshstinger,
        windforce,
        eaglehorn,
        gonnagalsDirk,
        theDefender,
        gryphonsClaw,
        blackRazor,
        gibbousMoon,
        iceShank,
        theExecutionersBlade,
        theBonesaw,
        shadowhawk,
        wizardspike,
        lightsabre,
        theFalconsTalon,
        inferno,
        doombringer,
        theGrizzly,
        theGrandfather,
        theMangler,
        sharpBeak,
        bloodslayer,
        theCelestialAxe,
        wickedAxe,
        stonecleaver,
        aguinarasHatchet,
        hellslayer,
        messerschmidtsReaver,
        crackrust,
        hammerOfJholm,
        civerbsCudgel,
        theCelestialStar,
        baranarsStar,
        gnarledRoot,
        theCraniumBasher,
        schaefersHammer,
        dreamflange,
        staffOfShadows,
        immolator,
        stormSpire,
        gleamsong,
        thundercall,
        theProtector,
        najsPuzzler,
        mindcry,
        rodOfOnan,
        helmOfSpirits,
        thinkingCap,
        overlordsHelm,
        foolsCrest,
        gotterdamerung,
        royalCirclet,
        tornFleshOfSouls,
        theGladiatorsBane,
        theRainbowCloak,
        leatherOfAut,
        wisdomsWrap,
        sparkingMail,
        scavengerCarapace,
        nightscape,
        najsLightPlate,
        demonspikeCoat,
        theDeflector,
        splitSkullShield,
        dragonsBreach,
        blackoakShield,
        holyDefender,
        stormshield,
        bramble,
        ringOfRegha,
        theBleeder,
        constrictingRing,
        ringOfEngagement,
        null,
    };
}
