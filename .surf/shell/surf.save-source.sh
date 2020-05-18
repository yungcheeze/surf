#!/usr/bin/env bash
tmpfile=$(mktemp --dry-run surf.download.XXXXXX)
cat > "$HOME/Downloads/$tmpfile"
