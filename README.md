# TF:Grub Source Code
Keep in mind, most of it is pretty shitty

## Info

[Discord](https://grub-fortress.github.io/discord)

[Website](https://grub-fortress.github.io)

## Credits

- **Weapon Models**
  - **The Under Pressure – Haau, Dim**  
    https://gamebanana.com/mods/529270

  - **The Perforator – Haau, kibbleknight**  
    https://gamebanana.com/mods/516732

   - **The Big Owen – Haau, kibbleknight**  
    https://gamebanana.com/mods/603188

- **Community Help/Contributions**
  - **BetaM** – Custom Items, Credits menu, misc. fixes  
    https://www.youtube.com/BetaM
  - **Better Fortress 2** – The mod Grub Fortress is a fork of  
    https://github.com/ALIEN31ITA/Better-Fortress-2
  - **Team Fortress 2 Classic** – Rocket Gravity Attribute  
    https://www.tf2classic.com
  - **Team Fortress 2 Goldrush** – Removed MYM Hud, And V_Model Support  
    https://www.tf2goldrush.com/
  - **Toru the Red Fox** – Old TF2 Main menu  
    https://github.com/TorutheRedFox/source-sdk-2013
  - **Hactica** – Ultimate Weapon Animation Fixes  
    https://gamebanana.com/wips/86367

### Windows

Requirements:
 - Source SDK 2013 Multiplayer installed via Steam
 - Visual Studio 2022 with the following workload and components:
   - Desktop development with C++:
     - MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
     - Windows 11 SDK (10.0.22621.0) or Windows 10 SDK (10.0.19041.1)
 - Python 3.13 or later

Inside the cloned directory, navigate to `src`, run:
```bat
creategameprojects.bat
```
This will generate the Visual Studio project `grub_fortress.sln` which will be used to build TF2 Mirrored.

Then, on the menu bar, go to `Build > Build Solution`, and wait for everything to build.

You can then select the `Client (Grub Fortress)` project you wish to run, right click and select `Set as Startup Project` and hit the big green `> Local Windows Debugger` button on the tool bar in order to launch your mod.

The default launch options should be already filled in for the `Release` configuration.

### Linux (Not Tested For TF:Grub, pretty sure you will have to modify something to make it work)

Requirements:
 - Source SDK 2013 Multiplayer installed via Steam
 - podman

Inside the cloned directory, navigate to `src`, run:
```bash
./creategameprojects
```

This will build all the projects related to the SDK and your mods automatically against the Steam Runtime.

You can then, in the root of the cloned directory, you can navigate to `game` and run your mod by launching the build launcher for your mod project, eg:
```bash
./mod_tf
```
