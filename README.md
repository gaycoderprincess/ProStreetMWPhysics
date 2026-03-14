# ProStreet Most Wanted Physics

Experiment for Need for Speed: ProStreet that converts the physics and handling model from Most Wanted and Carbon, including rechargeable NOS and the ability to use speedbreaker

Massive thanks to everyone involved with [dbalatoni13's MW reverse engineering project](https://github.com/dbalatoni13/nfsmw), this mod was heavily based off of it.

## Installation

- Make sure you have v1.1 of the game, as this is the only version this plugin is compatible with. (exe size of 3765248 or 28739656 bytes)
- Plop the files into your game folder.
- Enjoy, nya~ :3

## Known issues

- Drift and drag events revert to the vanilla game's handling, as burnouts, wheelies and drift scoring currently don't work with MW physics
- Skidmarks and tire smoke aren't 100% accurate

## Building

Building is done on an Arch Linux system with CLion and vcpkg being used for the build process. 

Before you begin, clone [nya-common](https://github.com/gaycoderprincess/nya-common), [nya-common-nfsps](https://github.com/gaycoderprincess/nya-common-nfsps) and [CwoeeMenuLib](https://github.com/gaycoderprincess/CwoeeMenuLib) to folders next to this one, so they can be found.

Required packages: `mingw-w64-gcc`

You should be able to build the project now in CLion.