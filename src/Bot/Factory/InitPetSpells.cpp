/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PlayerbotFactory.h"

void PlayerbotFactory::InitPetSpells()
{
    Map* map = bot->GetMap();
    if (!map)
        return;

    Pet* pet = bot->GetPet();
    if (!pet)
    {
        // LOG_INFO("playerbots", "{} init pet talents failed with no pet", bot->GetName().c_str());
        return;
    }

    if (bot->GetClass() == CLASS_HUNTER)
    {
        enum HunterPetType
        {
            PET_WOLF,
            PET_CAT,
            PET_SPIDER,
            PET_BEAR,
            PET_BOAR,
            PET_CROCOLISK,
            PET_CARRION_BIRD,
            PET_CRAB,
            PET_GORILLA,
            PET_RAPTOR,
            PET_TALLSTRIDER,
            PET_SCORPID,
            PET_TURTLE,
            PET_BAT,
            PET_HYENA,
            PET_OWL,
            PET_WIND_SERPENT,
            PET_UNKNOWN
        };

        std::map<HunterPetType, std::vector<std::pair<uint32, uint32>>> hunterPetSpells;

        hunterPetSpells[PET_BAT] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697},
            // Dive
            {30, 23145},
            {40, 23146},
            {50, 23147},
            // Screech
            {8,  24423},
            {24, 24577},
            {40, 24578},
            {56, 27051}
        };

        hunterPetSpells[PET_BEAR] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Claw
            {1,  16827},
            {8,  16828},
            {15, 16829},
            {22, 16830},
            {29, 16831},
            {36, 16832},
            {48, 3010 },
            {56, 3009 },
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697}
        };

        hunterPetSpells[PET_BOAR] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Charge
            {1,  7371 },
            {12, 26177},
            {24, 26178},
            {36, 26179},
            {48, 26201},
            {60, 27685},
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697},
            // Dash
            {30, 23099},
            {40, 23109},
            {50, 23110}
        };

        hunterPetSpells[PET_CARRION_BIRD] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Claw
            {1,  16827},
            {8,  16828},
            {15, 16829},
            {22, 16830},
            {29, 16831},
            {36, 16832},
            {48, 3010 },
            {56, 3009 },
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697},
            // Dive
            {30, 23145},
            {40, 23146},
            {50, 23147},
            // Screech
            {8,  24423},
            {24, 24577},
            {40, 24578},
            {56, 27051}
        };

        hunterPetSpells[PET_CAT] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Claw
            {1,  16827},
            {8,  16828},
            {15, 16829},
            {22, 16830},
            {29, 16831},
            {36, 16832},
            {48, 3010 },
            {56, 3009 },
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697},
            // Dash
            {30, 23099},
            {40, 23109},
            {50, 23110},
            // Prowl
            {30, 24450},
            {40, 24452},
            {50, 24453}
        };

        hunterPetSpells[PET_CRAB] = {
            // Claw
            {1,  16827},
            {8,  16828},
            {15, 16829},
            {22, 16830},
            {29, 16831},
            {36, 16832},
            {48, 3010 },
            {56, 3009 },
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697}
        };

        hunterPetSpells[PET_CROCOLISK] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697}
        };

        hunterPetSpells[PET_GORILLA] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697},
            // Thunderstomp
            {30, 26090},
            {40, 26187},
            {50, 26188}
        };

        hunterPetSpells[PET_HYENA] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697},
            // Dash
            {30, 23099},
            {40, 23109},
            {50, 23110}
        };

        hunterPetSpells[PET_OWL] = {
            // Claw
            {1,  16827},
            {8,  16828},
            {15, 16829},
            {22, 16830},
            {29, 16831},
            {36, 16832},
            {48, 3010 },
            {56, 3009 },
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697},
            // Dive
            {30, 23145},
            {40, 23146},
            {50, 23147},
            // Screech
            {8,  24423},
            {24, 24577},
            {40, 24578},
            {56, 24579}
        };

        hunterPetSpells[PET_RAPTOR] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Claw
            {1,  16827},
            {8,  16828},
            {15, 16829},
            {22, 16830},
            {29, 16831},
            {36, 16832},
            {48, 3010 },
            {56, 3009 },
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697}
        };

        hunterPetSpells[PET_SCORPID] = {
            // Claw
            {1,  16827},
            {8,  16828},
            {15, 16829},
            {22, 16830},
            {29, 16831},
            {36, 16832},
            {48, 3010 },
            {56, 3009 },
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697},
            // Scorpid Poison
            {8,  24640},
            {24, 24583},
            {40, 24586},
            {56, 24587}
        };

        hunterPetSpells[PET_SPIDER] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697}
        };

        hunterPetSpells[PET_TALLSTRIDER] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697},
            // Dash
            {30, 23099},
            {40, 23109},
            {50, 23110}
        };

        hunterPetSpells[PET_TURTLE] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697},
            // Shell Shield
            {20, 26064}
        };

        hunterPetSpells[PET_WIND_SERPENT] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697},
            // Dive
            {30, 23145},
            {40, 23146},
            {50, 23147},
            // Lightning Breath
            {1,  24844},
            {12, 25008},
            {24, 25009},
            {36, 25010},
            {48, 25011},
            {60, 25012}
        };

        hunterPetSpells[PET_WOLF] = {
            // Bite
            {1,  17253},
            {8,  17255},
            {16, 17256},
            {24, 17257},
            {32, 17258},
            {40, 17259},
            {48, 17260},
            {56, 17261},
            // Cower
            {5,  1742 },
            {15, 1753 },
            {25, 1754 },
            {35, 1755 },
            {45, 1756 },
            {55, 16697},
            // Dash
            {30, 23099},
            {40, 23109},
            {50, 23110},
            // Furious Howl
            {10, 24604},
            {20, 24605},
            {30, 24603},
            {40, 24597}
        };

        // Determine petType from creature template family
        auto GetHunterPetTypeFromEntry = [](uint32 entry) -> HunterPetType {
            CreatureInfo const* ci = sObjectMgr.GetCreatureTemplate(entry);
            if (!ci)
                return PET_UNKNOWN;

            switch (ci->pet_family)
            {
                case 1: return PET_WOLF;
                case 2: return PET_CAT;
                case 3: return PET_SPIDER;
                case 4: return PET_BEAR;
                case 5: return PET_BOAR;
                case 6: return PET_CROCOLISK;
                case 7: return PET_CARRION_BIRD;
                case 8: return PET_CRAB;
                case 9: return PET_GORILLA;
                case 11: return PET_RAPTOR;
                case 12: return PET_TALLSTRIDER;
                case 20: return PET_SCORPID;
                case 21: return PET_TURTLE;
                case 24: return PET_BAT;
                case 25: return PET_HYENA;
                case 26: return PET_OWL;
                case 27: return PET_WIND_SERPENT;
                default: return PET_UNKNOWN;
            }
        };

        HunterPetType petType = GetHunterPetTypeFromEntry(pet->GetEntry());

        auto it = hunterPetSpells.find(petType);
        if (it != hunterPetSpells.end())
        {
            std::vector<uint32> activeSpellsToRemove;
            static const std::unordered_set<uint32> disabledAutocastSpellIds = {
                1742, 1753, 1754, 1755, 1756, 16697,
                24450, 24452, 24453
            };

            std::vector<uint32> expectedActiveSpells;
            for (const auto& pair : it->second)
            {
                if (pet->GetLevel() < pair.first)
                    continue;

                uint32 firstId = sSpellMgr.GetFirstSpellInChain(pair.second);
                std::vector<uint32>::iterator expected = std::find_if(
                    expectedActiveSpells.begin(), expectedActiveSpells.end(),
                    [firstId](uint32 spellId)
                    {
                        return sSpellMgr.GetFirstSpellInChain(spellId) == firstId;
                    });

                if (expected == expectedActiveSpells.end())
                    expectedActiveSpells.push_back(pair.second);
                else if (sSpellMgr.IsHighRankOfSpell(pair.second, *expected))
                    *expected = pair.second;
            }

            static const std::pair<uint32, uint32> growlRanks[] = {
                {1, 2649}, {10, 14916}, {20, 14917}, {30, 14918},
                {40, 14919}, {50, 14920}, {60, 14921}
            };
            uint32 expectedGrowlSpellId = 0;
            for (const auto& rank : growlRanks)
            {
                if (pet->GetLevel() >= rank.first)
                    expectedGrowlSpellId = rank.second;
            }
            if (expectedGrowlSpellId)
                expectedActiveSpells.push_back(expectedGrowlSpellId);

            std::vector<uint32> currentActiveSpells;
            for (const auto& spell : pet->m_petSpells)
            {
                if (spell.second.state == PETSPELL_REMOVED || Spells::IsPassiveSpell(spell.first))
                    continue;

                currentActiveSpells.push_back(spell.first);
            }

            bool spellsCorrect = currentActiveSpells.size() == expectedActiveSpells.size();
            if (spellsCorrect)
            {
                for (uint32 spellId : expectedActiveSpells)
                {
                    if (std::find(currentActiveSpells.begin(), currentActiveSpells.end(), spellId) == currentActiveSpells.end())
                    {
                        spellsCorrect = false;
                        break;
                    }
                }
            }

            if (spellsCorrect)
            {
                bool autocastChanged = false;
                for (uint32 spellId : disabledAutocastSpellIds)
                {
                    PetSpellMap::iterator spell = pet->m_petSpells.find(spellId);
                    if (spell != pet->m_petSpells.end() &&
                        spell->second.state != PETSPELL_REMOVED &&
                        spell->second.active == ACT_ENABLED)
                    {
                        pet->ToggleAutocast(spellId, false);
                        autocastChanged = true;
                    }
                }

                if (autocastChanged)
                    bot->PetSpellInitialize();

                return;
            }

            for (PetSpellMap::const_iterator spellItr = pet->m_petSpells.begin(); spellItr != pet->m_petSpells.end(); ++spellItr)
            {
                if (spellItr->second.state == PETSPELL_REMOVED)
                    continue;

                uint32 spellId = spellItr->first;

                if (Spells::IsPassiveSpell(spellId))
                    continue;

                activeSpellsToRemove.push_back(spellId);
            }

            for (uint32 spellId : activeSpellsToRemove)
            {
                pet->ToggleAutocast(spellId, false);
                pet->RemoveSpell(spellId, false, false);
            }

            pet->CleanupActionBar();

            // Cower

            for (const auto& pair : it->second)
            {
                const uint32& levelRequired = pair.first;
                const uint32& spellID = pair.second;

                if (pet->GetLevel() >= levelRequired)
                {
                    if (!pet->HasSpell(spellID))
                    {
                        pet->AddSpell(spellID);
                    }

                    if (!Spells::IsPassiveSpell(spellID))
                    {
                        // Cower should be available, but not autocast.
                        const bool autocast = (disabledAutocastSpellIds.find(spellID) == disabledAutocastSpellIds.end());

                        if (pet->HasSpell(spellID))
                        {
                            pet->ToggleAutocast(spellID, autocast);
                        }
                    }
                }
            }
        }

        // Growl
        struct GrowlRank
        {
            uint32 minLevel;
            uint32 spellId;
        };
        static const GrowlRank growlRanks[] = {
            {1,  2649 }, // Growl Rank 1
            {10, 14916}, // Growl Rank 2
            {20, 14917}, // Growl Rank 3
            {30, 14918}, // Growl Rank 4
            {40, 14919}, // Growl Rank 5
            {50, 14920}, // Growl Rank 6
            {60, 14921}, // Growl Rank 7
        };
        uint32 growlSpellId = 0;
        for (const auto& rank : growlRanks)
        {
            if (pet->GetLevel() >= rank.minLevel)
                growlSpellId = rank.spellId;
        }
        if (growlSpellId)
        {
            if (!pet->HasSpell(growlSpellId))
            {
                pet->AddSpell(growlSpellId);
            }

            if (pet->HasSpell(growlSpellId))
            {
                pet->ToggleAutocast(growlSpellId, true);
            }
        }

        // Natural Armor
        struct NaturalArmorRank
        {
            uint32 minLevel;
            uint32 spellId;
        };
        static const NaturalArmorRank naturalArmorRanks[] = {
            {1,  24545},
            {12, 24549},
            {18, 24550},
            {24, 24551}
        };
        uint32 naturalArmorSpellId = 0;
        for (const auto& rank : naturalArmorRanks)
        {
            if (pet->GetLevel() >= rank.minLevel)
                naturalArmorSpellId = rank.spellId;
        }
        if (naturalArmorSpellId && !pet->HasSpell(naturalArmorSpellId))
        {
            pet->AddSpell(naturalArmorSpellId);
        }

        // Great Stamina
        struct GreatStaminaRank
        {
            uint32 minLevel;
            uint32 spellId;
        };
        static const GreatStaminaRank greatStaminaRanks[] = {
            {1,  4187},
            {12, 4188},
            {18, 4189},
            {24, 4190},
            {30, 4191},
            {36, 4192},
            {42, 4193},
            {48, 4194},
            {54, 5041},
            {60, 5042}
        };
        uint32 greatStaminaSpellId = 0;
        for (const auto& rank : greatStaminaRanks)
        {
            if (pet->GetLevel() >= rank.minLevel)
                greatStaminaSpellId = rank.spellId;
        }
        if (greatStaminaSpellId && !pet->HasSpell(greatStaminaSpellId))
        {
            pet->AddSpell(greatStaminaSpellId);
        }

                // Resistances
        if (pet->GetLevel() >= 20)
        {
            struct ResistanceSpell
            {
                uint32 spellId;
            };

            static const ResistanceSpell resistances[] = {
                {24493}, // Arcane
                {23992}, // Fire
                {24446}, // Frost
                {24492}, // Nature
                {24488} // Shadow
            };

            for (const auto& res : resistances)
            {
                if (!pet->HasSpell(res.spellId))
                    pet->AddSpell(res.spellId);
            }
        }

        pet->CleanupActionBar();
        bot->PetSpellInitialize();
    }

    if (bot->GetClass() == CLASS_WARLOCK)
    {
        constexpr uint32 PET_IMP = 416;
        constexpr uint32 PET_FELHUNTER = 417;
        constexpr uint32 PET_VOIDWALKER = 1860;
        constexpr uint32 PET_SUCCUBUS = 1863;
        constexpr uint32 PET_FELGUARD = 17252;

        //      pet type                    pet level  pet spell id
        std::map<uint32, std::vector<std::pair<uint32, uint32>>> spellList;

        // Imp spells
        {
            // Blood Pact
            spellList[PET_IMP].push_back(std::pair(4, 6307));
            spellList[PET_IMP].push_back(std::pair(14, 7804));
            spellList[PET_IMP].push_back(std::pair(26, 7805));
            spellList[PET_IMP].push_back(std::pair(38, 11766));
            spellList[PET_IMP].push_back(std::pair(50, 11767));
            spellList[PET_IMP].push_back(std::pair(62, 27268));
            spellList[PET_IMP].push_back(std::pair(74, 47982));

            // Fire Shield
            spellList[PET_IMP].push_back(std::pair(14, 2947));
            spellList[PET_IMP].push_back(std::pair(24, 8316));
            spellList[PET_IMP].push_back(std::pair(34, 8317));
            spellList[PET_IMP].push_back(std::pair(44, 11770));
            spellList[PET_IMP].push_back(std::pair(54, 11771));
            spellList[PET_IMP].push_back(std::pair(64, 27269));
            spellList[PET_IMP].push_back(std::pair(76, 47983));

            // Firebolt
            spellList[PET_IMP].push_back(std::pair(1, 3110));
            spellList[PET_IMP].push_back(std::pair(8, 7799));
            spellList[PET_IMP].push_back(std::pair(18, 7800));
            spellList[PET_IMP].push_back(std::pair(28, 7801));
            spellList[PET_IMP].push_back(std::pair(38, 7802));
            spellList[PET_IMP].push_back(std::pair(48, 11762));
            spellList[PET_IMP].push_back(std::pair(58, 11763));
            spellList[PET_IMP].push_back(std::pair(68, 27267));
            spellList[PET_IMP].push_back(std::pair(78, 47964));

            // Phase Shift
            spellList[PET_IMP].push_back(std::pair(12, 4511));
        }

        // Felhunter spells
        {
            // Devour Magic
            spellList[PET_FELHUNTER].push_back(std::pair(30, 19505));
            spellList[PET_FELHUNTER].push_back(std::pair(38, 19731));
            spellList[PET_FELHUNTER].push_back(std::pair(46, 19734));
            spellList[PET_FELHUNTER].push_back(std::pair(54, 19736));
            spellList[PET_FELHUNTER].push_back(std::pair(62, 27276));
            spellList[PET_FELHUNTER].push_back(std::pair(70, 27277));
            spellList[PET_FELHUNTER].push_back(std::pair(77, 48011));

            // Paranoia
            spellList[PET_FELHUNTER].push_back(std::pair(42, 19480));

            // Spell Lock
            spellList[PET_FELHUNTER].push_back(std::pair(36, 19244));
            spellList[PET_FELHUNTER].push_back(std::pair(52, 19647));

            // Tainted Blood
            spellList[PET_FELHUNTER].push_back(std::pair(32, 19478));
            spellList[PET_FELHUNTER].push_back(std::pair(40, 19655));
            spellList[PET_FELHUNTER].push_back(std::pair(48, 19656));
            spellList[PET_FELHUNTER].push_back(std::pair(56, 19660));
            spellList[PET_FELHUNTER].push_back(std::pair(64, 27280));
        }

        // Voidwalker spells
        {
            // Consume Shadows
            spellList[PET_VOIDWALKER].push_back(std::pair(18, 17767));
            spellList[PET_VOIDWALKER].push_back(std::pair(26, 17850));
            spellList[PET_VOIDWALKER].push_back(std::pair(34, 17851));
            spellList[PET_VOIDWALKER].push_back(std::pair(42, 17852));
            spellList[PET_VOIDWALKER].push_back(std::pair(50, 17853));
            spellList[PET_VOIDWALKER].push_back(std::pair(58, 17854));
            spellList[PET_VOIDWALKER].push_back(std::pair(66, 27272));
            spellList[PET_VOIDWALKER].push_back(std::pair(73, 47987));
            spellList[PET_VOIDWALKER].push_back(std::pair(78, 47988));

            // Sacrifice
            spellList[PET_VOIDWALKER].push_back(std::pair(16, 7812));
            spellList[PET_VOIDWALKER].push_back(std::pair(24, 19438));
            spellList[PET_VOIDWALKER].push_back(std::pair(32, 19440));
            spellList[PET_VOIDWALKER].push_back(std::pair(40, 19441));
            spellList[PET_VOIDWALKER].push_back(std::pair(48, 19442));
            spellList[PET_VOIDWALKER].push_back(std::pair(56, 19443));
            spellList[PET_VOIDWALKER].push_back(std::pair(64, 27273));
            spellList[PET_VOIDWALKER].push_back(std::pair(72, 47985));
            spellList[PET_VOIDWALKER].push_back(std::pair(79, 47986));

            // Suffering
            spellList[PET_VOIDWALKER].push_back(std::pair(24, 17735));
            spellList[PET_VOIDWALKER].push_back(std::pair(36, 17750));
            spellList[PET_VOIDWALKER].push_back(std::pair(48, 17751));
            spellList[PET_VOIDWALKER].push_back(std::pair(60, 17752));
            spellList[PET_VOIDWALKER].push_back(std::pair(63, 27271));
            spellList[PET_VOIDWALKER].push_back(std::pair(69, 33701));
            spellList[PET_VOIDWALKER].push_back(std::pair(75, 47989));
            spellList[PET_VOIDWALKER].push_back(std::pair(80, 47990));

            // Torment
            spellList[PET_VOIDWALKER].push_back(std::pair(10, 3716));
            spellList[PET_VOIDWALKER].push_back(std::pair(20, 7809));
            spellList[PET_VOIDWALKER].push_back(std::pair(30, 7810));
            spellList[PET_VOIDWALKER].push_back(std::pair(40, 7811));
            spellList[PET_VOIDWALKER].push_back(std::pair(50, 11774));
            spellList[PET_VOIDWALKER].push_back(std::pair(60, 11775));
            spellList[PET_VOIDWALKER].push_back(std::pair(70, 27270));
            spellList[PET_VOIDWALKER].push_back(std::pair(80, 47984));
        }

        // Succubus spells
        {
            // Lash of Pain
            spellList[PET_SUCCUBUS].push_back(std::pair(20, 7814));
            spellList[PET_SUCCUBUS].push_back(std::pair(28, 7815));
            spellList[PET_SUCCUBUS].push_back(std::pair(36, 7816));
            spellList[PET_SUCCUBUS].push_back(std::pair(44, 11778));
            spellList[PET_SUCCUBUS].push_back(std::pair(52, 11779));
            spellList[PET_SUCCUBUS].push_back(std::pair(60, 11780));
            spellList[PET_SUCCUBUS].push_back(std::pair(68, 27274));
            spellList[PET_SUCCUBUS].push_back(std::pair(74, 47991));
            spellList[PET_SUCCUBUS].push_back(std::pair(80, 47992));

            // Lesser Invisibility
            spellList[PET_SUCCUBUS].push_back(std::pair(32, 7870));

            // Seduction
            spellList[PET_SUCCUBUS].push_back(std::pair(26, 6358));

            // Soothing Kiss
            spellList[PET_SUCCUBUS].push_back(std::pair(22, 6360));
            spellList[PET_SUCCUBUS].push_back(std::pair(34, 7813));
            spellList[PET_SUCCUBUS].push_back(std::pair(46, 11784));
            spellList[PET_SUCCUBUS].push_back(std::pair(58, 11785));
            spellList[PET_SUCCUBUS].push_back(std::pair(70, 27275));
        }

        // Felguard spells
        {
            // Anguish
            spellList[PET_FELGUARD].push_back(std::pair(50, 33698));
            spellList[PET_FELGUARD].push_back(std::pair(60, 33699));
            spellList[PET_FELGUARD].push_back(std::pair(69, 33700));
            spellList[PET_FELGUARD].push_back(std::pair(78, 47993));

            // Avoidance
            spellList[PET_FELGUARD].push_back(std::pair(60, 32233));

            // Cleave
            spellList[PET_FELGUARD].push_back(std::pair(50, 30213));
            spellList[PET_FELGUARD].push_back(std::pair(60, 30219));
            spellList[PET_FELGUARD].push_back(std::pair(68, 30223));
            spellList[PET_FELGUARD].push_back(std::pair(76, 47994));

            // Demonic Frenzy
            spellList[PET_FELGUARD].push_back(std::pair(56, 32850));

            // Intercept
            spellList[PET_FELGUARD].push_back(std::pair(52, 30151));
            spellList[PET_FELGUARD].push_back(std::pair(61, 30194));
            spellList[PET_FELGUARD].push_back(std::pair(69, 30198));
            spellList[PET_FELGUARD].push_back(std::pair(79, 47996));
        }

        // Learn the appropriate spells by level and type
        const auto& petSpellListItr = spellList.find(pet->GetEntry());
        if (petSpellListItr != spellList.end())
        {
            std::map<uint32, uint32> expectedSpells;
            for (const auto& pair : petSpellListItr->second)
            {
                const uint32& levelRequired = pair.first;
                const uint32& spellID = pair.second;

                if (pet->GetLevel() >= levelRequired)
                {
                    uint32 firstSpell = sSpellMgr.GetFirstSpellInChain(spellID);
                    std::map<uint32, uint32>::iterator expectedSpell = expectedSpells.find(firstSpell);
                    if (expectedSpell == expectedSpells.end() || sSpellMgr.IsHighRankOfSpell(spellID, expectedSpell->second))
                        expectedSpells[firstSpell] = spellID;
                }
            }

            bool spellsAdded = false;
            for (const auto& expectedSpell : expectedSpells)
            {
                if (pet->AddSpell(expectedSpell.second))
                    spellsAdded = true;
            }

            bool autocastChanged = false;
            for (PetSpellMap::const_iterator itr = pet->m_petSpells.begin(); itr != pet->m_petSpells.end(); ++itr)
            {
                if (itr->second.state == PETSPELL_REMOVED || Spells::IsPassiveSpell(itr->first))
                    continue;

                const bool wasEnabled = itr->second.active == ACT_ENABLED;
                const uint8 autoSpellCount = pet->GetPetAutoSpellSize();
                pet->ToggleAutocast(itr->first, true);
                if (!wasEnabled || pet->GetPetAutoSpellSize() != autoSpellCount)
                    autocastChanged = true;
            }

            if (spellsAdded || autocastChanged)
            {
                pet->CleanupActionBar();
                bot->PetSpellInitialize();
            }
        }
    }
}
