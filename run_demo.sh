#!/usr/bin/env bash
# Simple demo script: build and run publisher and subscriber in separate terminals (tmux)
set -euo pipefail

mkdir -p build
cd build
cmake ..
make -j

# Try to use tmux if available
if command -v tmux >/dev/null 2>&1; then
  tmux new-session -d -s fedpub \
    "./subscriber sub1 news" ";" \
    "sleep 1; ./publisher pub1 news"
  echo "Started demo in tmux session 'fedpub'. Attach with: tmux attach -t fedpub"
else
  echo "tmux not found. You can run in two terminals:"
  echo "  ./subscriber sub1 news"
  echo "  ./publisher pub1 news"
fi
