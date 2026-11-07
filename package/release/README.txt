SW CHOSEN
=========

Version 1.0. Shadow Warrior (the VoidSW port) with online lobbies without port forwarding,
FIND LAN GAMES, a live dashboard for your own server and touch controls. Everybody in a network
game needs the same version - PC, Mac, iPad, Android and browser versions play together.

https://github.com/AdrianPol-projects/ShadowWarrior-Chosen


GAME FILES

  Copy SW.GRP (and SW.RTS if you have it) into this folder: Shadow Warrior Classic Complete or
  Classic Redux (GOG, Steam), the CD, or the free shareware version. The game files are not
  included. Add-ons: WT.GRP (Wanton Destruction), TD.GRP (Twin Dragon), user maps (.map).


STARTING

  Windows: swchosen.exe       Linux: ./swchosen (nothing else to install)
  macOS: ShadowWarrior.app (the first time: right-click > Open)


MULTIPLAYER (main menu)

  PLAYERS           how many play (2-8); the game starts when all of them are there.
  HOST LAN GAME     waits for the players, then NETWORK GAME: game type, level, limits,
                    START GAME.
  FIND LAN GAMES    the games waiting on your network; pick one to join.
  ONLINE            SERVER, KEY and LOBBY NAME of your relay server (select, type, Return),
                    then HOST A LOBBY or FIND LOBBIES. No port forwarding needed.
                    Running a server: SERVER.md on GitHub.
  LEAVE NETWORK GAME  back to playing alone.

  On a LAN the host's computer needs UDP port 23513 open (Windows asks the first time) and
  UDP 23517 for FIND LAN GAMES. The settings are kept in swchosen.cfg ([Comm Setup]).
