struct DropItem
{
	char szName[ITEM_NAME_MAX_LEN + 1];
	DWORD count;
};

ACMD(do_drop)
{
	if (!ch || !ch->GetDesc()) { return; }
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: drop_simul <mob vnum> <count>");
		return;
	}

	DWORD vnum = 0;
	str_to_number(vnum, arg1);
	DWORD loopCount = 0;
	str_to_number(loopCount, arg2);

	if (vnum == 0 || loopCount == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: drop_simul <mob vnum> <count>");
		return;
	}

	auto* mob = CMobManager::instance().Get(vnum);
	if (!mob)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Mob vnum: %u not found", vnum);
		return;
	}


	std::vector<LPITEM> drop_vec;
	std::unordered_map<DWORD/*vnum*/, DropItem> dropMap;

	if (ITEM_MANAGER::instance().CreateMobDropVec(vnum, drop_vec))
	{
		for (const auto& item : drop_vec)
		{
			if (!item) { continue; }
			
			const auto itemVnum = item->GetVnum();
			const auto itemCount = item->GetCount();
			
			if (itemVnum == 0 || itemCount == 0) { continue; }
		
			const auto& it = dropMap.find(itemVnum);
			if (it != dropMap.end())
			{
				it->second.count += itemCount;
			}
			else
			{
				DropItem dropItem{};
				dropItem.count = itemCount;
				strlcpy(dropItem.szName, item->GetName(), sizeof(dropItem.szName));
				dropMap[itemVnum] = dropItem;
			}

			M2_DESTROY_ITEM(item);
		}

		ch->ChatPacket(CHAT_TYPE_PARTY, "############### |cFF6d8cf2|H|h(%d)x %s|h|r DROP SIMULATION ###############", loopCount, mob->m_table.szLocaleName);

		const auto& func = [&](auto& pair)
		{
			auto& [vnum, dropItem] = pair;
			dropItem.count *= loopCount;
			ch->ChatPacket(CHAT_TYPE_PARTY, "Item: %s - Count: %u", dropItem.szName, dropItem.count);
		};

		std::for_each(dropMap.begin(), dropMap.end(), func);
	}
}