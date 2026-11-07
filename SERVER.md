# Running your own SW Chosen server

SW Chosen plays over the internet **without port forwarding** on the players' side. For that you run a small server, the **relay**, on a machine that's always on: a Raspberry Pi, a home server, a NAS with Docker or a cheap VPS. The same server shows a **live dashboard** of the running games and serves **the game for web browsers** at `/play/`.

There is no public default server: players enter your server's address and key in MULTIPLAYER > ONLINE.

## How it works

```
 host's game  ──WebSocket──►  relay  ◄──WebSocket──  joining players' games
                               │
                               └──►  dashboard in a web browser (same key)
```

- The host's game opens a **lobby** on the relay; other games list the lobbies and join one. Every game only makes an outgoing connection.
- The games keep their normal network code (ENet over UDP, as on a LAN); every packet travels inside a WebSocket message. The relay only forwards them.
- Shadow Warrior's network game has a fixed number of players: the host picks it, the lobby waits until everybody is there, and it closes when the game starts (`DROP_IN=0`, the default).
- Everything needs the **server key**. Game and browser answer a random challenge with HMAC-SHA256(key, challenge); the key itself is never sent. 10 wrong keys in 10 minutes lock that address out for 10 minutes.
- The host sends small stats (players, health, armor, weapons, keys, positions, the level's walls); the dashboard shows lobbies, player cards, a live map, an event list and a history.

The relay is one Python file (`server/relay.py`, aiohttp) plus the dashboard in `server/static/`. A Raspberry Pi 3 or 4 is plenty.

## The web version at /play/

The relay downloads the `-web.zip` of the newest ShadowWarrior-Chosen release from GitHub when it starts, every hour and when someone opens the page (at most every 10 minutes), and serves it at `/play/`. Players pick their own `SW.GRP` once; it stays in their browser. Installing it as an app (iPad: Share > Add to Home Screen) needs https. Set `WEB_UPDATE_HOURS=0` to turn it off.

## Setup with Docker

```sh
git clone --depth 1 https://github.com/AdrianPol-projects/ShadowWarrior-Chosen.git
cd ShadowWarrior-Chosen/server
echo "GAME_KEY=$(openssl rand -hex 6)" > server.env
docker compose up -d --build
```

The container listens on `127.0.0.1:8102`. Put a reverse proxy in front of it (Caddy, nginx or a Cloudflare tunnel; WebSockets must pass through), or for a home network change the `ports:` line to `"8102:8080"` and use `<server address>:8102` as SERVER in the game.

- Games connect with plain `ws://<server>/ws`, so the game connection must be reachable over plain HTTP (a Cloudflare tunnel does that if "Always Use HTTPS" is off for that hostname).
- Behind Cloudflare add `TRUST_PROXY=1` to `server.env`, so the limits per address use the players' real addresses.
- The dashboard is at `http(s)://<server>/`, the web game at `/play/`.
- `docker logs sw-relay` shows lobbies, players, their ping and why they left.

**A Raspberry Pi that already runs a Caddy + Cloudflare tunnel setup** (for example a NotBlood Chosen relay): `server/setup-pi.sh <hostname>` sets up the container, a Caddy site, the tunnel route and an update timer that rebuilds the container when `server/` changes on GitHub. Read it before running it.

## Without Docker

Python 3.10 or newer: `pip install aiohttp`, then `GAME_KEY=yourkey PORT=8102 DATA_DIR=./data python3 relay.py`.

## Settings (server.env)

| Setting | Default | |
| --- | --- | --- |
| `GAME_KEY` | (none) | the server key; nobody can connect without one |
| `TRUST_PROXY` | 0 | 1 behind Cloudflare: use `CF-Connecting-IP` |
| `DROP_IN` | 0 | 1: players may join lobbies whose game is running |
| `WEB_REPO` | AdrianPol-projects/ShadowWarrior-Chosen | where the web version comes from |
| `WEB_UPDATE_HOURS` | 1 | 0: never download the web version |
