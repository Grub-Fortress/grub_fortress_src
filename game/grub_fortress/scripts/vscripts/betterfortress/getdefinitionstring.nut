// You can get any key from items_game.txt thats on the root of the item
local weapon = GetListenServerHost().GetActiveWeapon()

function printdef(name)
{
	local value = weapon.GetDefinitionString(name)
	value = value ? value : "NULL"
	printf("%s: %s\n", name, value)
}

printdef("name")
printdef("first_sale_date")
printdef("item_class")
printdef("craft_class")
printdef("craft_material_type")
printdef("item_name")
printdef("item_type_name")
printdef("item_slot")
printdef("anim_slot")
printdef("item_quality")
printdef("a_key_that_does_not_exist")