#include "InvManager.hpp"

#include <Features/FeatureManager.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <Features/Events/PacketOutEvent.hpp>
#include <Features/Events/PingUpdateEvent.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/Inventory/PlayerInventory.hpp>
#include <SDK/Minecraft/Network/LoopbackPacketSender.hpp>
#include <SDK/Minecraft/Network/MinecraftPackets.hpp>
#include <SDK/Minecraft/Network/Packets/InteractPacket.hpp>
#include <SDK/Minecraft/Network/Packets/ContainerClosePacket.hpp>
#include <SDK/Minecraft/World/BlockLegacy.hpp>

void InvManager::onEnable()
{
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &InvManager::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->listen<PacketInEvent, &InvManager::onPacketInEvent>(this);
    gFeatureManager->mDispatcher->listen<PacketOutEvent, &InvManager::onPacketOutEvent>(this);
    gFeatureManager->mDispatcher->listen<PingUpdateEvent, &InvManager::onPingUpdateEvent>(this);
}

void InvManager::onDisable()
{
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &InvManager::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketInEvent, &InvManager::onPacketInEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketOutEvent, &InvManager::onPacketOutEvent>(this);
    gFeatureManager->mDispatcher->deafen<PingUpdateEvent, &InvManager::onPingUpdateEvent>(this);
}

void InvManager::onBaseTickEvent(BaseTickEvent& event)
{
    auto player = ClientInstance::get()->getLocalPlayer();
    auto armorContainer = player->getArmorContainer();
    auto supplies = player->getSupplies();
    auto container = supplies->getContainer();

    if (mManagementMode.mValue != ManagementMode::Always && !mHasOpenContainer)
    {
        return;
    }

    int freeSlots = 0;
    for (int i = 0; i < 36; i++)
    {
        if (!container->getItem(i)->mItem) freeSlots++;
    }

    if (ClientInstance::get()->getMouseGrabbed() && player && freeSlots > 0 && mManagementMode.mValue == ManagementMode::Always)
    {
        return;
    }

    std::vector<int> itemsToEquip;
    bool isInstant = mMode.mValue == Mode::Instant;
    if (mLastAction + static_cast<uint64_t>(mDelay.mValue) > NOW)
    {
        return;
    }

    int bestHelmetSlot = -1;
    int bestChestplateSlot = -1;
    int bestLeggingsSlot = -1;
    int bestBootsSlot = -1;
    int bestSwordSlot = -1;
    int bestPickaxeSlot = -1;
    int bestAxeSlot = -1;
    int bestShovelSlot = -1;

    int bestHelmetValue = 0;
    int bestChestplateValue = 0;
    int bestLeggingsValue = 0;
    int bestBootsValue = 0;
    int bestSwordValue = 0;
    int bestPickaxeValue = 0;
    int bestAxeValue = 0;
    int bestShovelValue = 0;

    int equippedHelmetValue = ItemUtils::getItemValue(armorContainer->getItem(0));
    int equippedChestplateValue = ItemUtils::getItemValue(armorContainer->getItem(1));
    int equippedLeggingsValue = ItemUtils::getItemValue(armorContainer->getItem(2));
    int equippedBootsValue = ItemUtils::getItemValue(armorContainer->getItem(3));

    int firstBowSlot = -1;
    int fireSwordSlot = ItemUtils::getFireSword(false);

    for (int i = 0; i < 36; i++)
    {
        auto item = container->getItem(i);
        if (!item->mItem) continue;
        auto itemType = item->getItem()->getItemType();

        if (item->getItem()->mName.contains("bow") && firstBowSlot == -1 && mDropExtraBows.mValue)
        {
            firstBowSlot = i;
        }
        else if (firstBowSlot != -1 && mDropExtraBows.mValue && item->getItem()->mName.contains("bow"))
        {
            supplies->getContainer()->dropSlot(i);
            if (mSwing.mValue) player->swing();

            mLastAction = NOW;
            if (!isInstant)
            {
                return;
            }
        }

        if (mIgnoreFireSword.mValue && fireSwordSlot != -1 && fireSwordSlot == i) continue;

        auto itemValue = ItemUtils::getItemValue(item);
        if (itemType == SItemType::Helmet && itemValue > bestHelmetValue)
        {
            if (equippedHelmetValue >= itemValue)
            {
                bestHelmetSlot = -1;
                continue;
            }

            bestHelmetSlot = i;
            bestHelmetValue = itemValue;
        }
        else if (itemType == SItemType::Chestplate && itemValue > bestChestplateValue)
        {
            if (equippedChestplateValue >= itemValue)
            {
                bestChestplateSlot = -1;
                continue;
            }

            bestChestplateSlot = i;
            bestChestplateValue = itemValue;
        }
        else if (itemType == SItemType::Leggings && itemValue > bestLeggingsValue)
        {
            if (equippedLeggingsValue >= itemValue)
            {
                bestLeggingsSlot = -1;
                continue;
            }

            bestLeggingsSlot = i;
            bestLeggingsValue = itemValue;
        }
        else if (itemType == SItemType::Boots && itemValue > bestBootsValue)
        {
            if (equippedBootsValue >= itemValue)
            {
                bestBootsSlot = -1;
                continue;
            }

            bestBootsSlot = i;
            bestBootsValue = itemValue;
        }
        else if (itemType == SItemType::Sword && itemValue > bestSwordValue)
        {
            bestSwordSlot = i;
            bestSwordValue = itemValue;
        }
        else if (itemType == SItemType::Pickaxe && itemValue > bestPickaxeValue)
        {
            bestPickaxeSlot = i;
            bestPickaxeValue = itemValue;
        }
        else if (itemType == SItemType::Axe && itemValue > bestAxeValue)
        {
            bestAxeSlot = i;
            bestAxeValue = itemValue;
        }
        else if (itemType == SItemType::Shovel && itemValue > bestShovelValue)
        {
            bestShovelSlot = i;
            bestShovelValue = itemValue;
        }
    }

    std::vector<int> itemsToDrop;
    for (int i = 0; i < 36; i++)
    {
        auto item = container->getItem(i);
        if (!item->mItem) continue;
        if (mIgnoreFireSword.mValue && fireSwordSlot != -1 && fireSwordSlot == i) continue;
        auto itemType = item->getItem()->getItemType();
        auto itemValue = ItemUtils::getItemValue(item);
        bool hasFireProtection = item->getEnchantValue(Enchant::FIRE_PROTECTION) > 0;

        if (mStealFireProtection.mValue && hasFireProtection) {
            continue;
        }

        if (itemType == SItemType::Sword && i != bestSwordSlot)
        {
            itemsToDrop.push_back(i);
        }
        else if (itemType == SItemType::Pickaxe && i != bestPickaxeSlot)
        {
            itemsToDrop.push_back(i);
        }
        else if (itemType == SItemType::Axe && i != bestAxeSlot)
        {
            itemsToDrop.push_back(i);
        }
        else if (itemType == SItemType::Shovel && i != bestShovelSlot)
        {
            itemsToDrop.push_back(i);
        }
        else if (itemType == SItemType::Helmet && i != bestHelmetSlot)
        {
            if (!(mStealFireProtection.mValue && hasFireProtection)) {
                itemsToDrop.push_back(i);
            }
        }
        else if (itemType == SItemType::Chestplate && i != bestChestplateSlot)
        {
            if (!(mStealFireProtection.mValue && hasFireProtection)) {
                itemsToDrop.push_back(i);
            }
        }
        else if (itemType == SItemType::Leggings && i != bestLeggingsSlot)
        {
            if (!(mStealFireProtection.mValue && hasFireProtection)) {
                itemsToDrop.push_back(i);
            }
        }
        else if (itemType == SItemType::Boots && i != bestBootsSlot)
        {
            if (!(mStealFireProtection.mValue && hasFireProtection)) {
                itemsToDrop.push_back(i);
            }
        }
    }

    for (auto& item : itemsToDrop)
    {
        supplies->getContainer()->dropSlot(item);
        if (mSwing.mValue) player->swing();

        mLastAction = NOW;
        if (!isInstant)
        {
            return;
        }
    }

    if (mPreferredSlots.mValue)
    {
        auto handlePreferredSlot = [&](int itemSlot, EnumSettingT<int>& preferredSlotSetting, std::function<bool(ItemStack*)> isTargetItem) {
            int pref = preferredSlotSetting.mValue;
            if (pref != 0) {
                int targetSlot = -1;
                bool inRegion = false;
                int regionStart = -1, regionEnd = -1;
                if (pref >= 1 && pref <= 9) targetSlot = pref - 1;
                else if (pref == 10) {
                    regionStart = 9; regionEnd = 17;
                }
                else if (pref == 11) {
                    regionStart = 17; regionEnd = 9;
                }
                if (regionStart != -1 && regionEnd != -1) {

                    if (pref == 11) {
                        for (int i = regionStart; i >= regionEnd; --i) {
                            ItemStack* slotItem = container->getItem(i);
                            if (slotItem && slotItem->mItem && isTargetItem(slotItem)) {
                                if (itemSlot == i) return false;
                            }
                        }

                        for (int i = regionStart; i >= regionEnd; --i) {
                            ItemStack* slotItem = container->getItem(i);
                            if (!slotItem->mItem || !isTargetItem(slotItem)) {
                                targetSlot = i;
                                break;
                            }
                        }
                        if (itemSlot >= regionEnd && itemSlot <= regionStart) inRegion = true;
                    }
                    else {
                        for (int i = regionStart; i <= regionEnd; ++i) {
                            ItemStack* slotItem = container->getItem(i);
                            if (slotItem && slotItem->mItem && isTargetItem(slotItem)) {
                                if (itemSlot == i) return false;
                            }
                        }

                        for (int i = regionStart; i <= regionEnd; ++i) {
                            ItemStack* slotItem = container->getItem(i);
                            if (!slotItem->mItem || !isTargetItem(slotItem)) {
                                targetSlot = i;
                                break;
                            }
                        }
                        if (itemSlot >= regionStart && itemSlot <= regionEnd) inRegion = true;
                    }
                }
                if (itemSlot != -1) {
                    if (pref == 12) {
                        supplies->getContainer()->dropSlot(itemSlot);
                        if (mSwing.mValue) player->swing();
                        mLastAction = NOW;
                        if (!isInstant) return true;
                    }
                    else if (targetSlot != -1 && itemSlot != targetSlot) {

                        if (!inRegion || (inRegion && itemSlot != targetSlot)) {
                            supplies->getContainer()->swapSlots(itemSlot, targetSlot);
                            if (mSwing.mValue) player->swing();
                            mLastAction = NOW;
                            if (!isInstant) return true;
                        }
                    }
                }
            }
            return false;
            };

        if (handlePreferredSlot(bestSwordSlot, mPreferredSwordSlot, [](ItemStack*) {return false;})) return;
        if (handlePreferredSlot(bestPickaxeSlot, mPreferredPickaxeSlot, [](ItemStack*) {return false;})) return;
        if (handlePreferredSlot(bestAxeSlot, mPreferredAxeSlot, [](ItemStack*) {return false;})) return;
        if (handlePreferredSlot(bestShovelSlot, mPreferredShovelSlot, [](ItemStack*) {return false;})) return;
        if (handlePreferredSlot(fireSwordSlot, mPreferredFireSwordSlot, [](ItemStack*) {return false;})) return;
        if (mPreferredBlocksSlot.mValue != 0) {
            int pref = mPreferredBlocksSlot.mValue;
            int targetSlot = -1;
            if (pref >= 1 && pref <= 9) targetSlot = pref - 1;
            else if (pref == 10) targetSlot = 9;
            else if (pref == 11) targetSlot = 17;
            if (pref == 12) {
                int firstPlaceable = ItemUtils::getFirstPlaceable(false);
                if (firstPlaceable != -1) {
                    supplies->getContainer()->dropSlot(firstPlaceable);
                    if (mSwing.mValue) player->swing();
                    mLastAction = NOW;
                    if (!isInstant) return;
                }
            }
                            else {
                    ItemStack* item = container->getItem(targetSlot);
                    if (!ItemUtils::isUsableBlock(item)) {
                        int firstPlaceable = ItemUtils::getFirstPlaceable(false);
                        if (firstPlaceable != -1) {
                            supplies->getContainer()->swapSlots(firstPlaceable, targetSlot);
                            if (mSwing.mValue) player->swing();
                            mLastAction = NOW;
                            if (!isInstant) return;
                        }
                    }
                }
        }

        auto findItemSlot = [&](std::function<bool(ItemStack*)> isTargetItem) -> int {
            for (int i = 0; i < 36; ++i) {
                ItemStack* item = container->getItem(i);
                if (item && item->mItem && isTargetItem(item)) return i;
            }
            return -1;
            };

        if (handlePreferredSlot(findItemSlot(ItemUtils::isArrow), mPreferredArrowSlot, ItemUtils::isArrow)) return;

        if (handlePreferredSlot(findItemSlot(ItemUtils::isBow), mPreferredBowSlot, ItemUtils::isBow)) return;

        if (handlePreferredSlot(findItemSlot(ItemUtils::isCrumblingCobblestone), mPreferredCrumbleStoneSlot, ItemUtils::isCrumblingCobblestone)) return;

        if (handlePreferredSlot(findItemSlot(ItemUtils::isEnderPearl), mPreferredPearlSlot, ItemUtils::isEnderPearl)) return;

        if (handlePreferredSlot(findItemSlot(ItemUtils::isGoldenApple), mPreferredGappleSlot, ItemUtils::isGoldenApple)) return;

        if (handlePreferredSlot(findItemSlot(ItemUtils::isSnowball), mPreferredSnowballSlot, ItemUtils::isSnowball)) return;
    }

    if (bestHelmetSlot != -1) itemsToEquip.push_back(bestHelmetSlot);
    if (bestChestplateSlot != -1) itemsToEquip.push_back(bestChestplateSlot);
    if (bestLeggingsSlot != -1) itemsToEquip.push_back(bestLeggingsSlot);
    if (bestBootsSlot != -1) itemsToEquip.push_back(bestBootsSlot);

    for (auto& item : itemsToEquip)
    {
        supplies->getContainer()->equipArmor(item);
        if (mSwing.mValue) player->swing();
        mLastAction = NOW;
        if (!isInstant)
        {
            break;
        }
    }
}

void InvManager::onPacketInEvent(PacketInEvent& event)
{
    if (event.mPacket->getId() == PacketID::ContainerOpen)
    {
        auto packet = event.getPacket<ContainerOpenPacket>();
        if (mManagementMode.mValue == ManagementMode::ContainerOnly || mManagementMode.mValue == ManagementMode::InvOnly && packet->mType == ContainerType::Inventory)
        {
            mHasOpenContainer = true;
        }
    }
    if (event.mPacket->getId() == PacketID::ContainerClose)
    {
        mHasOpenContainer = false;
    }
}

void InvManager::onPacketOutEvent(PacketOutEvent& event)
{
    if (event.mPacket->getId() == PacketID::ContainerClose)
    {
        mHasOpenContainer = false;
    }
    else if (event.mPacket->getId() == PacketID::ContainerOpen)
    {
        auto packet = event.getPacket<ContainerOpenPacket>();
        if (mManagementMode.mValue == ManagementMode::ContainerOnly || mManagementMode.mValue == ManagementMode::InvOnly && packet->mType == ContainerType::Inventory)
        {
            mHasOpenContainer = true;
        }
    }
}

void InvManager::onPingUpdateEvent(PingUpdateEvent& event)
{
    mLastPing = event.mPing;
}

bool InvManager::isItemUseless(ItemStack* item, int slot)
{
    if (!item->mItem) return true;
    auto player = ClientInstance::get()->getLocalPlayer();
    SItemType itemType = item->getItem()->getItemType();
    auto itemValue = ItemUtils::getItemValue(item);
    auto Inv_Manager = gFeatureManager->mModuleManager->getModule<InvManager>();

    if (itemType == SItemType::Helmet || itemType == SItemType::Chestplate || itemType == SItemType::Leggings || itemType == SItemType::Boots)
    {
        int equippedItemValue = ItemUtils::getItemValue(player->getArmorContainer()->getItem(static_cast<int>(itemType)));
        bool hasFireProtection = item->getEnchantValue(Enchant::FIRE_PROTECTION) > 0;

        if (Inv_Manager->mStealFireProtection.mValue && hasFireProtection) {
            return false;
        }

        return equippedItemValue >= itemValue;
    }

    if (itemType == SItemType::Sword || itemType == SItemType::Pickaxe || itemType == SItemType::Axe || itemType == SItemType::Shovel)
    {
        int bestSlot = ItemUtils::getBestItem(itemType);
        int bestValue = ItemUtils::getItemValue(player->getSupplies()->getContainer()->getItem(bestSlot));

        return bestValue >= itemValue && bestSlot != slot;
    }

    return false;
}