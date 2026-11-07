# SW Chosen online relay and live dashboard

Lets SW Chosen games play over the internet without port forwarding, shows the running games
(players, health, weapons, keys, a live level map) in a browser, and serves the web version of
the game at `/play/`. Setup: [../SERVER.md](../SERVER.md).

```sh
echo "GAME_KEY=$(openssl rand -hex 6)" > server.env
docker compose up -d --build     # listens on 127.0.0.1:8102
```

* Games connect to `ws://<server>/ws`; the dashboard is at `/`.
* `test_relay.py` checks the protocol against a running server, `simulate.py` sends a fake game so
  the dashboard has something to show.
* `setup-pi.sh <hostname>`: container, Caddy site, Cloudflare tunnel route and auto-update on a
  Raspberry Pi that already has Caddy and a cloudflared tunnel.
