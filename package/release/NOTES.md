# SW Chosen 1.0

**SW Chosen** is Shadow Warrior (the VoidSW port) with online lobbies without port forwarding, a LAN game finder, a live game dashboard and touch controls. It runs on **Windows, Linux, macOS, Android, iPad/iPhone and in web browsers**, and all of them play together.

You need your own `SW.GRP`: Shadow Warrior Classic Complete or Classic Redux (GOG, Steam), the CD, or the free shareware version. The game files are not included.

## Downloads

| File | For |
| --- | --- |
| `ShadowWarrior-Chosen-1.0-win64.zip` | Windows 10/11 (64-bit) |
| `ShadowWarrior-Chosen-1.0-linux-x64.zip` | Linux on PCs (x86_64), any distribution from about 2022 on, nothing to install |
| `ShadowWarrior-Chosen-1.0-linux-arm64.zip` | Linux on ARM (Raspberry Pi 4/5 with a 64-bit system, ARM laptops and handhelds) |
| `ShadowWarrior-Chosen-1.0-macos.zip` | macOS 11 or newer, Apple Silicon and Intel |
| `ShadowWarrior-Chosen-1.0-android.apk` | Android 5 or newer (phones, tablets, Chromebooks) |
| `ShadowWarrior-Chosen-1.0-ios.ipa` | iPad/iPhone with iOS 15 or newer (unsigned: sideload it with Signulous, Sideloadly or AltStore) |
| `ShadowWarrior-Chosen-1.0-web.zip` | the web version; every SW Chosen server installs it by itself and serves it at `/play/` |

Everybody in a network game needs **1.0**.

## Setup

**Windows / Linux:** unzip into a folder of its own, copy `SW.GRP` (and `SW.RTS` if you have it) next to the game, start `swchosen.exe` (Windows) or `./swchosen` (Linux).

**macOS:** unzip, copy `SW.GRP` into the `ShadowWarrior` folder next to `ShadowWarrior.app`, double-click the app (the first time: right-click > Open).

**Android:** install the APK, start it once and allow it "access to all files". Copy `SW.GRP` into the `ShadowWarrior` folder of the main storage (next to `Download`) and start the game again.

**iPad / iPhone:** sign and install the IPA (Signulous, Sideloadly or AltStore), start it once, copy `SW.GRP` to Files > On My iPad > SW Chosen.

**Web:** open your server's address followed by `/play/` and pick your Shadow Warrior files once. Add it to the home screen to use it like an app, offline too.

## Multiplayer

**MULTIPLAYER** in the main menu:
- **PLAYERS**: how many play (2-8). The game starts when all of them are there.
- **HOST LAN GAME**: waits for the players; then pick game type, level and limits in NETWORK GAME and START GAME.
- **FIND LAN GAMES**: games waiting on your network; pick one to join, then wait for the host to start.
- **ONLINE**: SERVER, KEY and LOBBY NAME (select one and type, Return keeps it), then **HOST A LOBBY** or **FIND LOBBIES**. Running your own server: [SERVER.md](https://github.com/AdrianPol-projects/ShadowWarrior-Chosen/blob/main/SERVER.md). There is no default server.
- **LEAVE NETWORK GAME** goes back to playing alone.

## Touch controls

On Android, iPad and in the browser on phones and tablets: a stick on the left, look with your finger on the right, FIRE, ALT, JUMP, DUCK, USE, a weapon picker (WPNS), inventory and the map. In menus, TOUCH sets the sensitivity, size and see-through of the buttons and gyro aiming, and has a layout editor.

## Known issues
- Shadow Warrior's network game has a fixed number of players: nobody can join once it runs.
- Tested on Linux and in browsers (LAN and online games, touch controls). The Windows, macOS, Android and iPad builds come from the same code but weren't played on those devices before this release; please report what you find.
