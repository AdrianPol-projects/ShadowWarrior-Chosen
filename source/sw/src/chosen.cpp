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

// Online lobbies on a relay server (server/relay.py), FIND LAN GAMES and live stats for the
// server's dashboard. Shadow Warrior's network game is the engine's mmulti (master/slave over
// UDP, a fixed number of players); this sets it up from the menu instead of the command line,
// and online the UDP datagrams travel through the relay (netrelay.cpp).

#include <string>

#include "build.h"
#include "mmulti.h"
#include "timer.h"
#include "keyboard.h"

#include "names2.h"
#include "game.h"
#include "menus.h"
#include "network.h"
#include "chosen.h"
#include "netrelay.h"

char gOnlineServer[64];
char gOnlineKey[48];
char gOnlineLobby[25];
int32_t gOnlineReportLan;
int gOnlineMode;
int32_t gChosenPlayers = 2;

extern char CommPlayerName[];
extern uint8_t CommPlayers;
extern int16_t OrigCommPlayers;
extern SWBOOL CommEnabled;
extern SWBOOL NetBroadcastMode;
extern int MovesPerPacket;
extern short Level;
extern MenuItem main_i[];
extern MenuGroup networkgroup, episodegroup;
void MultiSharewareCheck(void);

#define kGamePort 23513  // mmulti's NETPORT

static char zStatus[128];
static char zLobbyName[25];
static bool bReportSent;
static uint32_t nNextStats;
static std::string mapSentFor;
static bool bAdvertising;
static bool bNetGame;  // set up from the menu (the command line way stays as it was)

const char *onlineVersion(void)
{
    return CHOSEN_NAME " " CHOSEN_VERSION;
}

const char *onlineStatusText(void)
{
    return zStatus;
}

void onlineSetStatus(const char *pzText)
{
    Bstrncpyz(zStatus, pzText, sizeof(zStatus));
    LOG_F(INFO, "Online: %s", pzText);
}

bool chosenInNetGame(void)
{
    return numplayers > 1 || bNetGame;
}

static std::string msgField(const char *pzMsg, int nField)
{
    std::string s(pzMsg);
    size_t a = 0;
    while (nField-- > 0)
    {
        a = s.find('\t', a);
        if (a == std::string::npos)
            return "";
        a++;
    }
    size_t b = s.find_first_of("\t\n", a);
    return s.substr(a, b == std::string::npos ? std::string::npos : b - a);
}

static std::string cleanField(const char *pzText)
{
    std::string s;
    for (const char *p = pzText; *p; p++)
        if ((unsigned char)*p >= 32 && *p != '\t')
            s += *p;
    return s;
}

static const char *playerName(void)
{
    return CommPlayerName[0] ? CommPlayerName : "Player";
}

static const char *modName(void)
{
    extern char UserMapName[80];
    return UserMapName;
}

static void drawWaitScreen(const char *pzTitle, const char *pzText)
{
    short w, h;
    videoClearScreen(0);
    DrawMenuLevelScreen();
    char title[64];
    Bsnprintf(title, sizeof(title), "^%s", pzTitle);  // '^': the big letters
    MNU_MeasureString(title, &w, &h);
    MNU_DrawString(TEXT_XCENTER(w), 60, title, 1, 16);
    MNU_MeasureString(pzText, &w, &h);
    MNU_DrawString(TEXT_XCENTER(w), 100, pzText, 1, 16);
    char const *pzCancel = "Escape: cancel";
    MNU_MeasureString(pzCancel, &w, &h);
    MNU_DrawString(TEXT_XCENTER(w), 170, pzCancel, 1, 16);
    videoNextPage();
}

// shows a waiting screen until bDone() is true; false on timeout, Escape or a dead connection
template<typename F> static bool onlineWait(const char *pzText, int nTimeoutMs, F bDone)
{
    uint32_t nStart = timerGetTicks();
    while (!bDone())
    {
        if (relayStatus() == kRelayClosed)
        {
            onlineSetStatus(relayError()[0] ? relayError() : "Connection closed");
            return false;
        }
        if (nTimeoutMs > 0 && timerGetTicks() - nStart > (uint32_t)nTimeoutMs)
        {
            onlineSetStatus("The server didn't answer in time");
            return false;
        }
        handleevents();
        if (quitevent || KB_KeyPressed(sc_Escape))
        {
            KB_ClearKeyDown(sc_Escape);
            onlineSetStatus("Cancelled");
            return false;
        }
        drawWaitScreen("Online", pzText);
    }
    return true;
}

static bool onlineConnect(void)
{
#ifndef NOTBLOOD_WEB // in a browser no address means the server the page came from
    if (!gOnlineServer[0])
    {
        onlineSetStatus("Enter the server address first");
        return false;
    }
#endif
    if (relayStatus() == kRelayOnline)
        return true;
    relayConnect(gOnlineServer, gOnlineKey, onlineVersion());
    char buffer[128];
    Bsnprintf(buffer, sizeof(buffer), "Connecting to %s", gOnlineServer[0] ? gOnlineServer : "the server");
    if (!onlineWait(buffer, 15000, []() { return relayStatus() == kRelayOnline; }))
    {
        relayClose();
        return false;
    }
    return true;
}

static bool onlineReply(const char *pzText, char *pReply, int nSize, const char *pzWant)
{
    return onlineWait(pzText, 10000, [&]() {
        while (relayReadText(pReply, nSize))
        {
            std::string cmd = msgField(pReply, 0);
            if (cmd == pzWant || cmd == "error")
                return true;
        }
        return false;
    });
}

static void lobbyNameFor(char *pzName, int nSize)
{
    std::string s;
    for (const char *p = gOnlineLobby[0] ? gOnlineLobby : playerName(); *p && s.size() < 24; p++)
        if (isalnum((unsigned char)*p) || strchr(" _.-", *p))
            s += *p;
    if (!gOnlineLobby[0] && s.size() < 18)
        s += " game";
    if (s.empty())
        s = "SW game";
    Bstrncpyz(pzName, s.c_str(), nSize);
}

static bool onlineHostLobby(int nPlayers)
{
    if (!onlineConnect())
        return false;
    lobbyNameFor(zLobbyName, sizeof(zLobbyName));
    std::string cmd = std::string("host\t") + zLobbyName + "\t" + std::to_string(nPlayers) + "\t" + cleanField(playerName()) +
        "\t" + cleanField(modName()) + "\t" + onlineVersion();
    relaySendText(cmd.c_str());
    char reply[512];
    if (!onlineReply("Creating the lobby", reply, sizeof(reply), "hosted"))
    {
        relayClose();
        return false;
    }
    if (msgField(reply, 0) == "error")
    {
        onlineSetStatus(msgField(reply, 1).c_str());
        relayClose();
        return false;
    }
    relaySetHostGamePort(kGamePort);
    gOnlineMode = kOnlineHost;
    char buffer[128];
    Bsnprintf(buffer, sizeof(buffer), "Lobby \"%s\" is open", zLobbyName);
    onlineSetStatus(buffer);
    return true;
}

int chosenListLobbies(CHOSENLISTENTRY *pList, int nMax)
{
    if (!onlineConnect())
        return -1;
    relaySendText("list");
    char reply[8192];
    if (!onlineReply("Looking for lobbies", reply, sizeof(reply), "lobbies"))
    {
        relayClose();
        return -1;
    }
    int nCount = 0;
    const char *p = strchr(reply, '\n');
    while (p && nCount < nMax)
    {
        p++;
        const char *pEnd = strchr(p, '\n');
        std::string line(p, pEnd ? pEnd - p : strlen(p));
        // name host players max state mod version
        std::string state = msgField(line.c_str(), 4);
        CHOSENLISTENTRY *pEntry = &pList[nCount];
        Bstrncpyz(pEntry->szName, msgField(line.c_str(), 0).c_str(), sizeof(pEntry->szName));
        Bsnprintf(pEntry->szInfo, sizeof(pEntry->szInfo), "%s  %s/%s%s", msgField(line.c_str(), 1).c_str(), msgField(line.c_str(), 2).c_str(),
            msgField(line.c_str(), 3).c_str(), state == "waiting" ? "" : "  PLAYING");
        pEntry->szAddress[0] = 0;
        pEntry->nPort = 0;
        nCount++;
        p = pEnd;
    }
    char buffer[64];
    if (nCount)
        Bsnprintf(buffer, sizeof(buffer), "%d lobb%s on the server", nCount, nCount == 1 ? "y" : "ies");
    else
        Bsnprintf(buffer, sizeof(buffer), "No lobbies right now - host one!");
    onlineSetStatus(buffer);
    relayClose();
    return nCount;
}

// ---- the network game ---------------------------------------------------------------------------

// what game.cpp does after the command line's network setup
static void netStarted(void)
{
    NetBroadcastMode = FALSE;  // master/slave
    initsynccrc();
    if (numplayers > 4 && MovesPerPacket == 1)
        MovesPerPacket = 2;
    MultiSharewareCheck();
    CommPlayers = numplayers;
    OrigCommPlayers = CommPlayers;
    CommEnabled = TRUE;
    gNet.MultiGameType = MULTI_GAME_COMMBAT;
    Connect();
    main_i[0].child = &networkgroup;
    bNetGame = true;
}

// argv for mmulti (initmultiplayersparms); waits until everybody is connected
static bool netSetup(int argc, const char **argv, const char *pzWaitTitle)
{
    uninitmultiplayers();  // the single player setup holds the game's port
    if (!initmultiplayersparms(argc, argv))
    {
        onlineSetStatus("Couldn't open the network port (another game running?)");
        return false;
    }
    char text[96];
    uint32_t nNextDraw = 0;
    while (initmultiplayerscycle())
    {
        handleevents();
        chosenFrame();
        if (quitevent || KB_KeyPressed(sc_Escape) || (gOnlineMode != kOnlineNone && relayStatus() == kRelayClosed))
        {
            KB_ClearKeyDown(sc_Escape);
            onlineSetStatus(relayStatus() == kRelayClosed && gOnlineMode != kOnlineNone ? "The connection to the server was lost" : "Cancelled");
            uninitmultiplayers();
            initmultiplayers(0, NULL, 0, 0, 0);
            return false;
        }
        if (timerGetTicks() >= nNextDraw)
        {
            nNextDraw = timerGetTicks() + 50;
            if (myconnectindex == connecthead)
                Bsnprintf(text, sizeof(text), "Starts when all %d players are here", gChosenPlayers);
            else
                Bsnprintf(text, sizeof(text), "%s", zStatus);
            drawWaitScreen(pzWaitTitle, text);
        }
    }
    netStarted();
    return true;
}

bool chosenHostGame(bool bOnline)
{
    if (chosenInNetGame())
        chosenLeaveGame();
    gChosenPlayers = clamp(gChosenPlayers, 2, MAX_SW_PLAYERS);
    if (bOnline && !onlineHostLobby(gChosenPlayers))
        return false;
    if (!bOnline)
    {
        char address[32] = "";
        netLocalAddress(address, sizeof(address));
        char buffer[96];
        Bsnprintf(buffer, sizeof(buffer), "Others join %s (FIND LAN GAMES)", address[0] ? address : "this computer");
        onlineSetStatus(buffer);
        if (gOnlineReportLan && gOnlineServer[0])
        {
            relayConnect(gOnlineServer, gOnlineKey, onlineVersion());
            gOnlineMode = kOnlineReport;
            bReportSent = false;
        }
    }
    lanAdvertiseStart();
    bAdvertising = true;

    char players[16];
    Bsnprintf(players, sizeof(players), "-n0:%d", gChosenPlayers);
    const char *argv[] = { players };
    if (!netSetup(1, argv, bOnline ? "Waiting in the lobby" : "Waiting for players"))
    {
        chosenLeaveGame();
        return false;
    }
    lanAdvertiseStop();
    bAdvertising = false;
    onlineSetStatus("Everybody is here: pick the level and START GAME");
    return true;
}

static bool joinAt(const char *pzAddress)
{
    // our own port: any free one (a host on this machine has the game's port)
    char port[16];
    Bsnprintf(port, sizeof(port), "-p%d", 23600 + (int)(timerGetTicks() % 300));
    char address[64];
    Bstrncpyz(address, pzAddress, sizeof(address));
    if (!strchr(address, ':'))
        Bsnprintf(address, sizeof(address), "%s:%d", pzAddress, kGamePort);
    const char *argv[] = { "-n0", address, port };
    char buffer[96];
    Bsnprintf(buffer, sizeof(buffer), "Waiting for the host at %s", pzAddress);
    onlineSetStatus(buffer);
    if (!netSetup(3, argv, "Joining"))
        return false;
    onlineSetStatus("Waiting for the host to start the game");
    return true;
}

bool chosenJoinAddress(const char *pzAddress)
{
    if (!pzAddress || !pzAddress[0])
    {
        onlineSetStatus("Enter the host's address first");
        return false;
    }
    if (chosenInNetGame())
        chosenLeaveGame();
    if (!joinAt(pzAddress))
    {
        chosenLeaveGame();
        return false;
    }
    return true;
}

bool chosenJoinLobby(const char *pzLobby)
{
    if (!pzLobby || !pzLobby[0])
    {
        onlineSetStatus("Pick a lobby first");
        return false;
    }
    if (chosenInNetGame())
        chosenLeaveGame();
    if (!onlineConnect())
        return false;
    std::string cmd = std::string("join\t") + cleanField(pzLobby) + "\t" + cleanField(playerName()) + "\t" +
        cleanField(modName()) + "\t" + onlineVersion();
    relaySendText(cmd.c_str());
    char reply[512];
    if (!onlineReply("Joining the lobby", reply, sizeof(reply), "joined"))
    {
        relayClose();
        return false;
    }
    if (msgField(reply, 0) == "error")
    {
        onlineSetStatus(msgField(reply, 1).c_str());
        relayClose();
        return false;
    }
    if (!onlineWait("Joining the lobby", 5000, []() { return relayClientPort() != 0; }))
    {
        relayClose();
        return false;
    }
    Bstrncpyz(zLobbyName, msgField(reply, 1).c_str(), sizeof(zLobbyName));
    gOnlineMode = kOnlineClient;
    char address[32];
    Bsnprintf(address, sizeof(address), "127.0.0.1:%d", relayClientPort());
    if (!joinAt(address))
    {
        chosenLeaveGame();
        return false;
    }
    char buffer[64];
    Bsnprintf(buffer, sizeof(buffer), "In lobby \"%s\": waiting for the host", zLobbyName);
    onlineSetStatus(buffer);
    return true;
}

void chosenLeaveGame(void)
{
    if (bNetGame || numplayers > 1)
    {
        uninitmultiplayers();
        initmultiplayers(0, NULL, 0, 0, 0);
        CommPlayers = 0;
        OrigCommPlayers = 0;
        CommEnabled = FALSE;
        main_i[0].child = &episodegroup;
        screenpeek = myconnectindex = 0;
    }
    bNetGame = false;
    if (bAdvertising)
        lanAdvertiseStop();
    bAdvertising = false;
    if (gOnlineMode != kOnlineNone)
        relayClose();
    gOnlineMode = kOnlineNone;
    bReportSent = false;
    mapSentFor.clear();
}

// ---- FIND LAN GAMES ------------------------------------------------------------------------------

void chosenLanScanStart(void)
{
    lanScanStart();
}

int chosenLanScanPoll(CHOSENLISTENTRY *pList, int nMax)
{
    LANGAME games[kChosenListMax];
    int nCount = lanScanPoll(games, min(nMax, (int)kChosenListMax));
    for (int i = 0; i < nCount; i++)
    {
        Bstrncpyz(pList[i].szName, games[i].szHost, sizeof(pList[i].szName));
        bool bSame = !Bstrcmp(games[i].szVersion, onlineVersion());
        Bsnprintf(pList[i].szInfo, sizeof(pList[i].szInfo), "%s  %d/%d%s%s", games[i].szAddress, games[i].nPlayers, games[i].nMaxPlayers,
            bSame ? "" : "  OTHER VERSION: ", bSame ? "" : games[i].szVersion);
        Bstrncpyz(pList[i].szAddress, games[i].szAddress, sizeof(pList[i].szAddress));
        pList[i].nPort = games[i].nPort;
    }
    return nCount;
}

void chosenLanScanStop(void)
{
    lanScanStop();
}

// ---- stats for the dashboard ----------------------------------------------------------------

static void jsonString(std::string &s, const char *pzText)
{
    s += '"';
    for (const unsigned char *p = (const unsigned char *)pzText; *p; p++)
    {
        if (*p == '"' || *p == '\\')
        {
            s += '\\';
            s += (char)*p;
        }
        else if (*p >= 32 && *p < 127)
            s += (char)*p;
    }
    s += '"';
}

static void jsonInt(std::string &s, const char *pzKey, int nValue)
{
    s += '"';
    s += pzKey;
    s += "\":";
    s += std::to_string(nValue);
    s += ',';
}

static void buildStats(std::string &s)
{
    static const char *pzWeapons[] = { "Fists", "Shurikens", "Riot Gun", "UZI", "Missile Launcher", "Grenade Launcher",
        "Sticky Bombs", "Rail Gun", "Guardian Head", "Ripper Heart" };

    s = "{\"v\":1,\"mode\":";
    jsonString(s, numplayers > 1 ? (gNet.MultiGameType == MULTI_GAME_COOPERATIVE ? "Co-op" : "WangBang") : "Single player");
    s += ",\"level\":";
    char level[96];
    Bsnprintf(level, sizeof(level), "%s", Level >= 0 && Level < MAX_LEVELS_REG + 2 && LevelInfo[Level].Description ? LevelInfo[Level].Description : "");
    jsonString(s, level);
    s += ",\"diff\":\"\",";
    jsonInt(s, "time", 0);
    s += "\"kills\":[0,0],\"secrets\":[0,0],\"players\":[";
    bool bFirst = true;
    for (int p = connecthead; p >= 0; p = connectpoint2[p])
    {
        PLAYERp pp = &Player[p];
        if (pp->PlayerSprite < 0 || !User[pp->PlayerSprite])
            continue;
        if (!bFirst)
            s += ',';
        bFirst = false;
        s += '{';
        jsonInt(s, "id", p);
        s += "\"name\":";
        jsonString(s, pp->PlayerName[0] ? pp->PlayerName : "Player");
        s += ',';
        int nHealth = User[pp->PlayerSprite]->Health;
        jsonInt(s, "alive", nHealth > 0);
        jsonInt(s, "health", max(nHealth, 0));
        s += "\"armor\":[" + std::to_string(pp->Armor) + ",0,0],";
        int nWeapon = clamp((int)pp->WpnNum, 0, (int)ARRAY_SIZE(pzWeapons) - 1);
        s += "\"weapon\":";
        jsonString(s, pzWeapons[nWeapon]);
        s += ',';
        jsonInt(s, "wid", nWeapon + 1);
        jsonInt(s, "ammo", nWeapon ? pp->WpnAmmo[nWeapon] : -1);
        s += "\"weapons\":[";
        for (int i = 0, n = 0; i < (int)ARRAY_SIZE(pzWeapons); i++)
            if (TEST(pp->WpnFlags, BIT(i)))
                s += (n++ ? "," : "") + std::to_string(i + 1);
        s += "],\"keys\":[";
        for (int i = 0, n = 0; i < 8; i++)
            if (pp->HasKey[i])
                s += (n++ ? "," : "") + std::to_string(i + 1);
        s += "],\"items\":[],\"powerups\":[],";
        int nDeaths = 0;
        for (int k = connecthead; k >= 0; k = connectpoint2[k])
            if (k < MAX_SW_PLAYERS_REG && p < MAX_SW_PLAYERS_REG)
                nDeaths += Player[k].KilledPlayer[p];
        jsonInt(s, "frags", pp->Kills);
        jsonInt(s, "deaths", nDeaths);
        jsonInt(s, "x", pp->posx);
        jsonInt(s, "y", pp->posy);
        jsonInt(s, "z", pp->posz);
        s += "\"ang\":" + std::to_string(fix16_to_int(pp->q16ang) & 2047) + "}";
    }
    s += "],\"enemies\":[";
    int nEnemies = 0;
    for (int nSprite = headspritestat[STAT_ENEMY]; nSprite >= 0 && nEnemies < 400; nSprite = nextspritestat[nSprite])
    {
        spritetype const *pSprite = &sprite[nSprite];
        if (!User[nSprite] || User[nSprite]->Health <= 0)
            continue;
        s += nEnemies++ ? ",[" : "[";
        s += std::to_string(pSprite->x) + "," + std::to_string(pSprite->y) + "," + std::to_string(pSprite->ang & 2047) + "]";
    }
    s += "]}";
}

static void buildMap(std::string &s)
{
    s = "[";
    bool bFirst = true;
    for (int nSector = 0; nSector < numsectors; nSector++)
    {
        for (int nWall = sector[nSector].wallptr, nEnd = nWall + sector[nSector].wallnum; nWall < nEnd; nWall++)
        {
            walltype const *pWall = &wall[nWall];
            int nNext = pWall->nextwall;
            int nType = 0;
            if (nNext >= 0)
            {
                if (nNext < nWall)
                    continue;
                int nNextSector = pWall->nextsector;
                if (nNextSector < 0 || (sector[nNextSector].floorz == sector[nSector].floorz && sector[nNextSector].ceilingz == sector[nSector].ceilingz))
                    continue;
                nType = 1;
            }
            walltype const *pWall2 = &wall[pWall->point2];
            if (!bFirst)
                s += ',';
            bFirst = false;
            s += std::to_string(pWall->x) + "," + std::to_string(pWall->y) + "," + std::to_string(pWall2->x) + "," + std::to_string(pWall2->y) + "," + std::to_string(nType);
        }
    }
    s += "]";
}

void chosenFrame(void)
{
    if (bAdvertising)
    {
        lanAdvertiseUpdate(playerName(), onlineVersion(), modName(), kGamePort, numplayers, gChosenPlayers);
        lanAdvertisePoll();
    }

    if (gOnlineMode == kOnlineNone)
        return;

    char msg[512];
    while (relayReadText(msg, sizeof(msg)))
    {
        std::string cmd = msgField(msg, 0);
        if (cmd == "peer_join")
        {
            char buffer[64];
            Bsnprintf(buffer, sizeof(buffer), "%s is joining", msgField(msg, 2).c_str());
            onlineSetStatus(buffer);
        }
        else if (cmd == "hosted" && gOnlineMode == kOnlineReport)
            bReportSent = true;
        else if (cmd == "error" && gOnlineMode == kOnlineReport)
        {
            onlineSetStatus(msgField(msg, 1).c_str());
            relayClose();
            gOnlineMode = kOnlineNone;
            return;
        }
    }

    if (relayStatus() != kRelayOnline)
        return;
    if (gOnlineMode == kOnlineReport && !bReportSent)
    {
        static uint32_t nLastTry;
        if (timerGetTicks() - nLastTry < 2000)
            return;
        nLastTry = timerGetTicks();
        lobbyNameFor(zLobbyName, sizeof(zLobbyName));
        std::string name = std::string(zLobbyName) + " - LAN";
        std::string cmd = "report\t" + name.substr(0, 24) + "\t" + std::to_string(gChosenPlayers) + "\t" + cleanField(playerName()) +
            "\t" + cleanField(modName()) + "\t" + onlineVersion();
        relaySendText(cmd.c_str());
        bReportSent = true;
        return;
    }
    extern SWBOOL InMenuLevel;
    if (gOnlineMode == kOnlineClient || !bNetGame || InMenuLevel || myconnectindex != connecthead || numsectors <= 0 || Player[myconnectindex].PlayerSprite < 0)
        return;  // the host's level runs: the lobby is playing, the dashboard gets the stats

    static bool bPlaying;
    if (!bPlaying)
    {
        relaySendText("state\tplaying");
        bPlaying = true;
    }

    char mapId[64];
    Bsnprintf(mapId, sizeof(mapId), "%d-%d-%d", Level, numsectors, numwalls);
    if (mapSentFor != mapId)
    {
        std::string walls;
        buildMap(walls);
        std::string cmd = std::string("map\t") + mapId + "\t" + walls;
        relaySendText(cmd.c_str());
        mapSentFor = mapId;
    }

    uint32_t nNow = timerGetTicks();
    if ((int32_t)(nNow - nNextStats) < 0)
        return;
    nNextStats = nNow + 200;
    std::string stats;
    buildStats(stats);
    stats.insert(0, "stats\t");
    relaySendText(stats.c_str());
}
