# SW Chosen for iPad / iPhone

The iOS/iPadOS build of SW Chosen: touch controls, Magic Keyboard trackpad and mouse, controllers, and LAN and online multiplayer that plays together with the Windows, Linux, macOS, Android and web versions of the same version.

## Getting the IPA

The releases of [ShadowWarrior-Chosen](https://github.com/AdrianPol-projects/ShadowWarrior-Chosen/releases) have `ShadowWarrior-Chosen-<version>-ios.ipa`. It is unsigned: sign and install it with Signulous, Sideloadly or AltStore.

## Adding the game data

1. Launch the app once. It creates its folder and asks for the game files.
2. Open **Files → On My iPad → SW Chosen** and copy `SW.GRP` into it (Atomic Edition, 20th Anniversary World Tour, Megaton Edition, the CD or the free shareware version; `SW.RTS`, user maps and add-ons too if you like).
3. Go back to the game and tap **Check again**.

Settings, saves and `touchcontrols.cfg` live in the same folder.

## Playing

* **Touch:** stick on the left, look by dragging on the right, FIRE, ALT, JUMP, DUCK, USE, a weapon picker, inventory and the map. In menus, **TOUCH** opens the touch settings (sensitivity, size, gyro aiming, layout editor).
* **Keyboard, trackpad, mouse and controllers** work as on a computer; the touch buttons hide while a controller is connected.
* **Multiplayer:** MULTIPLAYER in the main menu (host a LAN game, find LAN games, online lobbies through your own server). iOS asks for **Local Network** permission the first time: allow it.

## Building yourself

```sh
cmake -S platform/iOS -B build-ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 \
      -DSDL2_SOURCE_DIR=/path/to/SDL2 -DGL4ES_SOURCE_DIR=/path/to/gl4es
cmake --build build-ios --config Release --target SWChosen -- -sdk iphoneos CODE_SIGNING_ALLOWED=NO
```

gl4es needs the patches in `platform/iOS/gl4es/`; `release.yml` shows the exact steps.

* `source/build/src/iosbits.mm`: folders, game data check, OpenGL fallback
* `source/sw/src/touchcontrols.cpp`: touch overlay, weapon picker, settings screen and layout editor
* `source/build/include/chosengame.h`: the names and folder of this game
