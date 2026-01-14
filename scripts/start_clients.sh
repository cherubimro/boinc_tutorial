#!/bin/bash
# Start 3 BOINC clients for testing

set -e

BOINC_CLIENT="$HOME/boinc_source/client/boinc_client"
CLIENTS_DIR="$HOME/boinc_clients"
PROJECT_URL="http://127.0.0.1/pi_test/"

mkdir -p "$CLIENTS_DIR"/{client_a,client_b,client_c}

# Start clients
for client in a b c; do
    cd "$CLIENTS_DIR/client_$client"
    port=$((31420 + $(echo $client | tr 'abc' '123')))
    
    echo "Starting client_$client on port $port..."
    nohup $BOINC_CLIENT --dir . --allow_remote_gui_rpc --gui_rpc_port $port > client_$client.log 2>&1 &
    echo "  PID: $!"
    sleep 2
    
    # Attach to project
    password=$(cat gui_rpc_auth.cfg)
    ~/boinc_source/client/boinccmd --host 127.0.0.1:$port --passwd $password \
        --project_attach $PROJECT_URL $(grep authenticator ~/boinc_test/projects/html/user/index.html | head -1 || echo "")
done

echo ""
echo "All clients started. Check status with: ps aux | grep boinc_client"
