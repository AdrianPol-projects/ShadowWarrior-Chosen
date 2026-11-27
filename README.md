# SW Chosen

Shadow Warrior with **online lobbies without port forwarding**, a **LAN game finder**, a **live game dashboard** and **touch controls**, for **Windows, Linux (PC and ARM), macOS, Android, iPad/iPhone and web browsers**. It is VoidSW (the Shadow Warrior port in [NBlood](https://github.com/nukeykt/NBlood)) with the multiplayer and platform work from [NotBlood Chosen](https://github.com/AdrianPol-projects/NotBlood-Chosen). Every player in a network game needs the same version; all platforms of one version play together.

**Downloads:** [releases](https://github.com/AdrianPol-projects/ShadowWarrior-Chosen/releases). Web browsers (no install): every SW Chosen server serves the game at `/play/` ([how it works](platform/Web/README.md)).

Unzip, put your `SW.GRP` next to the game and start it. Shadow Warrior Classic Complete or Classic Redux (GOG, Steam), the CD, or the free shareware version all work. The game files are not included.

**What SW Chosen adds to VoidSW**
* **MULTIPLAYER** in the main menu (until now network games needed command line options):
  * **PLAY WITH BOTS**: WangBang against up to 7 computer players (4 difficulties), offline.
  * **Bots in network games**: the host's *Other Options* have *Bots* and *Bot Skill*; the host plays the bots, so LAN games and online lobbies (public or with the server key) can mix people and bots. Everybody sees them as players, the lobby list counts them and the website's live view shows them. Command line: `-hostbotsN`, `-botskillN` (0-3).
  * **PLAYERS**: how many play (2-8). Shadow Warrior's network game waits until all of them are there.
  * **HOST LAN GAME**: waits for the players, then the NETWORK GAME screen: game type (WangBang, co-op, ...), level, limits, START GAME.
  * **FIND LAN GAMES**: the games waiting on your network; pick one to join.
  * **ONLINE**: your server's address and key, then **HOST A LOBBY** or **FIND LOBBIES**. No port forwarding: the game traffic goes through a relay server: the public one, sw.pi-mole.com (leave SERVER empty), or your own ([SERVER.md](SERVER.md)). Lobbies can have a password.
  * **LEAVE NETWORK GAME**.
* **Live dashboard** in the browser: lobbies, players' health, armor, weapons, keys, a live level map and an event list.
* **Touch devices** (Android, iPad, phones and tablets in the browser): on-screen stick, finger look, fire/jump/duck/use buttons, a weapon picker, gyro aiming, a layout editor; the buttons hide while a controller is connected.
* **Web version**: the whole game in Safari, Chrome, Edge or Firefox, installable as an app that works offline; your files stay in the browser.
* **macOS** app for Apple Silicon and Intel, **Android** keeps everything in the `ShadowWarrior` folder of the main storage, **iPad** in Files > On My iPad > SW Chosen.
* Engine fixes for network games set up after the start (a second network setup reused the old socket and a stale "ready" flag).

Building: `.github/workflows/release.yml` (all platforms), `platform/Android`, `platform/iOS`, `platform/Web`; on Linux `make sw`. The relay server is NotBlood Chosen's `server/` with `SITE=sw` ([SERVER.md](SERVER.md)); the public one is sw.pi-mole.com.

## Acknowledgments

VoidSW by the NBlood contributors (based on JFShadowWarrior by Jonathon Fowler and the EDuke32 engine work), NBlood by nukeykt and contributors, NotBlood by clipmove, EDuke32 by the EDuke32 team, the Build engine by Ken Silverman, Shadow Warrior by 3D Realms. See [AUTHORS.md](AUTHORS.md). Shadow Warrior is a trademark of its owners; this project contains no game data.

GPL 2 (game code) and the Build license (engine), see `package/common`.
