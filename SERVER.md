# The SW Chosen server

SW Chosen plays over the internet **without port forwarding**: games connect out to a small server,
the **relay**, which forwards their network traffic, shows the running games live on a **dashboard**
and serves **the game for web browsers** at `/play/`.

**Public server: sw.pi-mole.com.** The game uses it when SERVER is left empty in MULTIPLAYER > ONLINE.
Anybody can host and join there (lobbies can have a password); open https://sw.pi-mole.com to watch
the games or play in the browser.

## Your own server

The relay is shared by NotBlood Chosen, Duke3D Chosen and SW Chosen: it is the `server/` folder of
[NotBlood Chosen](https://github.com/AdrianPol-projects/NotBlood-Chosen/tree/main/server), set up
as its [SERVER.md](https://github.com/AdrianPol-projects/NotBlood-Chosen/blob/main/SERVER.md)
explains, with `SITE=sw` in `server.env`:

```sh
git clone --depth 1 https://github.com/AdrianPol-projects/NotBlood-Chosen.git
cd NotBlood-Chosen/server
printf 'BLOOD_KEY=%s\nSITE=sw\n' "$(openssl rand -hex 6)" > server.env
docker compose up -d --build
```

`SITE=sw` gives the dashboard SW Chosen's name and weapons and makes `/play/` serve SW Chosen's
web version (players bring their own `SW.GRP`). `PUBLIC=1` makes it a public server. Players
then enter your server's address and key in MULTIPLAYER > ONLINE.
