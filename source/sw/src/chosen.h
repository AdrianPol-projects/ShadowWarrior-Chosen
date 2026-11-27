//-------------------------------------------------------------------------
/*
This file is part of SW Chosen (VoidSW with the NotBlood Chosen features).

SW Chosen is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.
*/
//-------------------------------------------------------------------------

// SW Chosen: online lobbies through a relay server, a live dashboard, FIND LAN GAMES and a
// MULTIPLAYER menu (chosen.cpp). Everybody in a network game needs the same version.

#pragma once

#include "compat.h"

#define CHOSEN_NAME "SW Chosen"
#define CHOSEN_VERSION "1.1"

// the public server ONLINE uses when SERVER is left empty (its page: live games and the web version)
#ifndef CHOSEN_PUBLIC_SERVER
#define CHOSEN_PUBLIC_SERVER "sw.pi-mole.com"
#endif

enum
{
    kOnlineNone = 0,
    kOnlineHost,    // hosting a lobby: remote players come in through the relay
    kOnlineClient,  // joined a lobby
    kOnlineReport,  // LAN game whose stats are shown on the server's dashboard
};

extern char gOnlineServer[64];
extern char gOnlineKey[48];
extern char gOnlineLobby[25];
extern char gOnlineLobbyPassword[25];  // a private lobby: players need the same password
extern int32_t gOnlineReportLan;  // show hosted LAN games on the dashboard
extern int gOnlineMode;
// the server to use: SERVER from the menu, else the public one
const char *onlineServerAddress(void);
extern int32_t gChosenPlayers;    // how many players a hosted game waits for (2-8)

const char *onlineVersion(void);
const char *onlineStatusText(void);
void onlineSetStatus(const char *pzText);

// ---- the MULTIPLAYER menu ----
// Shadow Warrior's network game has a fixed number of players: the host waits until all of
// them are there (gChosenPlayers), then picks the level in NETWORK GAME and starts it.
bool chosenHostGame(bool bOnline);
bool chosenJoinAddress(const char *pzAddress);
bool chosenJoinLobby(const char *pzLobby);
// a network game is being left: closes the connections, back to single player
void chosenLeaveGame(void);
bool chosenInNetGame(void);

enum { kChosenListMax = 8 };
struct CHOSENLISTENTRY
{
    char szName[25];
    char szInfo[64];
    char szAddress[48];
    int nPort;
};
int chosenListLobbies(CHOSENLISTENTRY *pList, int nMax);
void chosenLanScanStart(void);
int chosenLanScanPoll(CHOSENLISTENTRY *pList, int nMax);
void chosenLanScanStop(void);

// every frame: relay messages, LAN answers, stats for the dashboard
void chosenFrame(void);
