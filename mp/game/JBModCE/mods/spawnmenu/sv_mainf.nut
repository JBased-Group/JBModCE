function OnClientExecCmd(client,cmd)
{
	if(startswith(cmd,"SpawnProp "))
	{
		local prop = CreateEntity("prop_physics_override");
		local pos = EntityGetPosition(client);
		EntitySetPosition(prop,pos[0],pos[1],pos[2]);
		SetEntityKeyvalue(prop,"model",cmd.slice(10));
		SpawnEntity(prop);
	}
}
/*
function OnTakeDamage(entindex, damage, attackerEntIndex, weaponEntIndex, damageType)
{
	PrintToServer(weaponEntIndex.tostring()+"\n");
    if (weaponEntIndex != -1) // Check if a weapon was used
    {
        return 1000.0; // Deal 1000 dmg
    }
    
    return damage; // Return OG dmg
}
*/
/* tests

function OnClientExecCmd(client,cmd)
{
	PrintToServer(client.tostring()+": "+cmd+"\n");
	if(startswith(cmd,"DisplayHint "))
	{
		SendKeyHintToClient(client,cmd.slice(12));
	}
	else if(startswith(cmd,"GetPosition"))
	{
		local pos = EntityGetPosition(client);
		PrintToServer(pos[0].tostring()+" "+pos[1].tostring()+" "+pos[2].tostring()+"\n");
	}
}
*/
function OnClientSpawned(client)
{
	PrintToServer(client.tostring()+"\n")
	SendKeyHintToClient(client,"BRUBURBURU");
	GiveNamedItem(client,"weapon_ar2",0);
	local ent = FindEntityByName("SEObspwn1",-1,-1);
	if(ent == null)
	{
		PrintToServer("Couldnt find ent\n");
		return;
	}
	PrintToServer("theent: "+ent.tostring()+"\n");
	//EntityFireInputBool(ent,"ForceSpawn",-1,-1,true);
}
/*
function OnEntityKilled(ent, damage, attacker, weapon, type)
{
	if(EntityGetClassname(ent) == "npc_citizen" && attacker != -1)
	{
		local atkcls = EntityGetClassname(attacker);
		local zombie = 0;
		if(atkcls == "npc_zombie" || atkcls == "npc_zombie_torso" || atkcls == "npc_headcrab")
		{
			zombie = CreateEntity("npc_zombie")
		}
		else if(atkcls == "npc_poisonzombie" || atkcls == "npc_headcrab_poison" || atkcls == "npc_headcrab_black")
		{
			zombie = CreateEntity("npc_poisonzombie")
		}
		else if(atkcls == "npc_headcrab_fast" || atkcls == "npc_fastzombie")
		{
			zombie = CreateEntity("npc_fastzombie")
		}
		if(zombie == 0)
		{
			return;
		}
		local pos = EntityGetPosition(ent);
		EntitySetPosition(zombie,pos[0],pos[1],pos[2]);
		SpawnEntity(zombie);

	}
}

*/