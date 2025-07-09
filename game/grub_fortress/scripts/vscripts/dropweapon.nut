::SpawnDroppedWeapon <- function(index, modelname, origin, attributes = {})
{
	local weapon = Entities.CreateByClassname("tf_dropped_weapon");
	NetProps.SetPropInt(weapon, "m_Item.m_iItemDefinitionIndex", index);
	NetProps.SetPropInt(weapon, "m_Item.m_iEntityLevel", 5);
	NetProps.SetPropInt(weapon, "m_Item.m_iEntityQuality", 6);
	NetProps.SetPropInt(weapon, "m_Item.m_bInitialized", 1);
	weapon.SetModelSimple(modelname);
	weapon.SetOrigin(origin);

	weapon.DispatchSpawn();
	foreach(name, value in attributes)
	{
		weapon.AddAttribute(name, value, -1)
	}
	return weapon;
}

/*
local attribs = 
{
	"fire rate bonus" : 0.5
	"damage bonus" : 1
}
SpawnDroppedWeapon(14, "models/weapons/c_models/c_sniperrifle/c_sniperrifle.mdl", GetListenServerHost().GetOrigin(), attribs)
*/