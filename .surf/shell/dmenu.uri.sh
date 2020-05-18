#!/usr/bin/env bash
tac ~/.surf/history | dmenu -l 10 -b -i | cut -d ' ' -f 3
