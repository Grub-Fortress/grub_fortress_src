# Wanna help? join us on [Discord](https://discord.gg/NM82mujJsf).
> [!IMPORTANT]
> ### Collection of changes that could end up in LIVE-TF2, the goal is to try making as much stuff as possible less hardcoded, to give mappers and vscripters more customization like, Custom map titles, killicons, and so on.
> ## This is also meant to be as a "Playground" build, demonstating how TF2 could function with community fixes and such, this is just for fun feel free to fork, or use

> [!WARNING]
> - Features some community fixes which i DO NOT CLAIM as mine, but credited (if we forgot please reach us on Discord).
> - Feel free to use my code for your own mods! i just wanted to improve TF2 a little, and add custom stuff.
> - More coders may be needed (i am not a full expert coder, but i try my best).

![new_tf2_logo](https://github.com/user-attachments/assets/7e53da2e-abd5-4d16-bbb3-7e37997372a5)
# FEATURES:
- External scenes loader  (Mapbase | .VCD, Faceposer stuff)
- Pyrovision Support keyvalue/netprop for tf_gamerules 
- ```nav_generate``` now reloads navigation instead of the map 
- ```sv_infinite_ammo```
## Player
- GiveItem (I/O and Command)
- Penetration kill sound toggle cvar | ```tf_snd_penetrationkill```
- Voice spam and delay restore | ```tf_voicespam | (1 normal, 2 no text)``` ```tf_max_voice_speak_delay 1.5```
- Custom Responses | Sniper!
## NPC's
### Horseless Headless Horsemann
- Now features an hammer keyvalue (from halloween 2014) | ```mallet```
## Items
- "Addon" items_game loader (BetaM)
- Reserve Shooter 🔧 | Engineer and Heavy can use it
- The Winger, Prettyboy Pocket Pistol | Engineer can use it
### Attributes | 🔧 Edited original one
- [330]🔧| Added Robot Footsteps | 10
- [4000] | override voice sound set | 0 off, 1 robot, 2 giant robot
- [4001] | sniperrifle has laser pointer | MVM bot lasers as an attribute
- [4002] | drops reviver machine | Player drops the reviver from MVM
- [4003] | medigun heals buildings | Restored leaked MVM attribute
### Custom Items - W weapon , C cosmetic
- [W] 32000 The Holy Marlin (Altranade)
- [W] 32001 The Underpressurer
- [W] 31999 Super Maul (TEST ITEM)
# Gamemode Changes/Improvements
- Flags can now have custom Pickup,Defend,Captured Text in the killfeed | ```text_captured, text_pickup, text_defend```
## Robot Destruction
- Edit Blu/Red score via Input
- Removed the 0 cap limit on flags to allow scoring negative points
- Implemented **RUDE** Cvar to silence the robots from the Leak
### The Robots
- Customizable sounds (Death,Hurt,Collide,Idle)
- Spawn with Dispenser Y/N
- OnPanicStart, OnPanicEnd Outputs

## License
The SDK is licensed to users on a non-commercial basis under the [SOURCE 1 SDK LICENSE](LICENSE), which is contained in the [LICENSE](LICENSE) file in the root of the repository.
For more information, see [Distributing your Mod](#markdown-header-distributing-your-mod).
