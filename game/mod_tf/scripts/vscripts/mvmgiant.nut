


::VMVM_BecomeGiant <- function()
{
	local ply = self
	local playerclass = ply.GetPlayerClass()
	local sound = null
	local giantmdl = null
	switch(playerclass) {
			case 1:
				sound = "MVM.GiantScoutLoop"
				giantmdl = "models/bots/scout_boss/bot_scout_boss.mdl"
				ply.SetHealth(1600)
				ply.AddCustomAttribute("max health additive bonus", 1475, -1)
				ply.AddCustomAttribute("damage force reduction", 0.7, -1)
				ply.AddCustomAttribute("airblast vulnerability multiplier", 0.7, -1)
				break
			case 3:
				sound = "MVM.GiantSoldierLoop"
				giantmdl = "models/bots/soldier_boss/bot_soldier_boss.mdl"
				ply.SetHealth(3800)
				ply.AddCustomAttribute("max health additive bonus", 3600, -1)
				ply.AddCustomAttribute("damage force reduction", 0.4, -1)
				ply.AddCustomAttribute("airblast vulnerability multiplier", 0.4, -1)			
				break
			case 7:
				sound = "MVM.GiantPyroLoop"
				giantmdl = "models/bots/pyro_boss/bot_pyro_boss.mdl"
				ply.SetHealth(3000)
				ply.AddCustomAttribute("max health additive bonus", 2825, -1)
				ply.AddCustomAttribute("damage force reduction", 0.6, -1)
				ply.AddCustomAttribute("airblast vulnerability multiplier", 0.6, -1)			
				break
			case 4:
				sound = "MVM.GiantDemomanLoop"
				giantmdl = "models/bots/demo_boss/bot_demo_boss.mdl"
				ply.SetHealth(3000)
				ply.AddCustomAttribute("max health additive bonus", 2825, -1)
				ply.AddCustomAttribute("damage force reduction", 0.5, -1)
				ply.AddCustomAttribute("airblast vulnerability multiplier", 0.5, -1)
				break
			case 6:
				sound = "MVM.GiantHeavyLoop"
				giantmdl = "models/bots/heavy_boss/bot_heavy_boss.mdl"
				ply.SetHealth(5000)
				ply.AddCustomAttribute("max health additive bonus", 4700, -1)
				ply.AddCustomAttribute("damage force reduction", 0.3, -1)
				ply.AddCustomAttribute("airblast vulnerability multiplier", 0.3, -1)
				break
		}
	PrecacheScriptSound(sound)
	PrecacheModel(giantmdl)
	ply.SetIsMiniBoss(true)
	ply.SetCustomModelWithClassAnimations(giantmdl)
	ply.SetModelScale(1.9,0)
	ply.AddCustomAttribute("override footstep sound set", 2, -1)
	if (playerclass != 1)
		ply.AddCustomAttribute("move speed bonus", 0.5, -1)
	EmitSoundEx({ sound_name = sound, volume = 1, sound_level = 80, entity = ply, filter_type = 5 })
	ply.ValidateScriptScope()
	ply.GetScriptScope().IsMVMGiant <- true
	ply.GetScriptScope().MVM_Loopsnd <- sound
}
VMVM_BecomeGiant()
	
if (!("MVMGiantVscript" in getroottable()))
	::MVMGiantVscript <- {};
::MVMGiantVscript.clear();
::MVMGiantVscript =
{
	OnGameEvent_player_death = function(params)
	{
		local player = GetPlayerFromUserID(params.userid);
		if (!player)
			return;
		if (params.death_flags & 32) // dead ringer
			return;
		if (player.GetScriptScope().IsMVMGiant)
			player.StopSound(player.GetScriptScope().MVM_Loopsnd)
	}

	OnGameEvent_scorestats_accumulated_update = function(params)
	{
		for (local player; player = Entities.FindByClassname(player, "player");)
		{
			if (player.GetScriptScope().IsMVMGiant)
				player.StopSound(player.GetScriptScope().MVM_Loopsnd)
		}
	}
}
__CollectGameEventCallbacks(MVMGiantVscript)