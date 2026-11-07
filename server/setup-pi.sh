#!/bin/bash
# setup-pi.sh <hostname> [port]: runs the SW Chosen relay on a Raspberry Pi (or any Debian box) that
# already has Docker, Caddy (importing /etc/caddy/sites-enabled/*) and a cloudflared tunnel
# (/etc/cloudflared/config.yml). Run it as the normal user; it asks for sudo where needed.
#
#  - ~/sw-relay: a sparse clone of this repo's server/ folder, server.env with a new GAME_KEY
#  - the sw-relay container on 127.0.0.1:<port> (default 8102)
#  - /etc/caddy/sites-enabled/sw.caddy: http://<hostname> -> the container
#  - a tunnel ingress rule and DNS route for <hostname> (backup of config.yml first)
#  - sw-relay-update.timer: every 5 minutes pulls main and rebuilds when server/ changed
set -euo pipefail
HOST=${1:?usage: setup-pi.sh <hostname> [port]}
PORT=${2:-8102}
DIR=$HOME/sw-relay
REPO=https://github.com/AdrianPol-projects/ShadowWarrior-Chosen.git

if [ ! -d "$DIR/src/.git" ]; then
    mkdir -p "$DIR"
    git clone -q --depth 1 --filter=blob:none --sparse "$REPO" "$DIR/src"
    git -C "$DIR/src" sparse-checkout set server
fi
cd "$DIR"
if [ ! -f server.env ]; then
    printf 'GAME_KEY=%s\nTRUST_PROXY=1\n' "$(openssl rand -hex 4 | sed 's/\(....\)\(....\)/\1-\2/')" > server.env
    chmod 600 server.env
fi
sed "s|127.0.0.1:8102:8080|127.0.0.1:$PORT:8080|; s|env_file: server.env|env_file: $DIR/server.env|; s|build: .|build: $DIR/src/server|" \
    src/server/docker-compose.yml > docker-compose.yml
docker compose -p sw-relay up -d --build

cat > update.sh << EOF
#!/bin/bash
# pulls ShadowWarrior-Chosen main and rebuilds the relay when server/ changed
exec 9> "$DIR/.update.lock"
flock -n 9 || exit 0
cd "$DIR/src" || exit 1
old=\$(git rev-parse HEAD:server 2>/dev/null)
git fetch -q --depth 1 origin main && git reset -q --hard origin/main
new=\$(git rev-parse HEAD:server)
if [ "\$old" != "\$new" ] || [ "\${1:-}" = --force ]; then
    cd "$DIR" && docker compose -p sw-relay up -d --build
fi
EOF
chmod +x update.sh

sudo tee /etc/systemd/system/sw-relay-update.service > /dev/null << EOF
[Unit]
Description=Update the SW Chosen relay
[Service]
Type=oneshot
User=$USER
ExecStart=$DIR/update.sh
EOF
sudo tee /etc/systemd/system/sw-relay-update.timer > /dev/null << EOF
[Unit]
Description=Update the SW Chosen relay every 5 minutes
[Timer]
OnBootSec=2min
OnUnitActiveSec=5min
[Install]
WantedBy=timers.target
EOF
sudo systemctl daemon-reload
sudo systemctl enable --now sw-relay-update.timer

# Caddy
echo "http://$HOST {
    reverse_proxy 127.0.0.1:$PORT
}" | sudo tee /etc/caddy/sites-enabled/sw.caddy > /dev/null
sudo systemctl reload caddy

# Cloudflare tunnel: an ingress rule like the existing ones, before the catch-all
CFG=/etc/cloudflared/config.yml
if ! sudo grep -q "hostname: $HOST\$" "$CFG"; then
    sudo cp "$CFG" "$CFG.bak-before-sw"
    SERVICE=$(sudo grep -A1 -- '- hostname:' "$CFG" | grep -m1 'service:' | sed 's/.*service: *//')
    sudo python3 - "$CFG" "$HOST" "$SERVICE" << 'EOF'
import sys
path, host, service = sys.argv[1:]
lines = open(path).read().split('\n')
i = next(n for n, l in enumerate(lines) if l.strip().startswith('- service:'))  # the catch-all rule
indent = lines[i][:len(lines[i]) - len(lines[i].lstrip())]
lines[i:i] = [indent + '- hostname: ' + host, indent + '  service: ' + service]
open(path, 'w').write('\n'.join(lines))
EOF
    TUNNEL=$(sudo sed -n 's/^tunnel: *//p' "$CFG")
    sudo cloudflared tunnel route dns "$TUNNEL" "$HOST" || true
    sudo systemctl restart cloudflared
fi

sleep 3
curl -s "http://127.0.0.1:$PORT/health"; echo
echo "Done. Dashboard: https://$HOST/  game server address: $HOST  key: $(sed -n 's/^GAME_KEY=//p' server.env)"
