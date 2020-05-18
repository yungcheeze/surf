#!/usr/bin/env bash
xid="$1"
tac ~/.surf/history | dmenu -l 10 -b -w "$xid" -i | cut -d ' ' -f 3
