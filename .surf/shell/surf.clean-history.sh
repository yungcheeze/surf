#!/usr/bin/env bash
cat ~/.surf/history > ~/.surf/history.$$
cat ~/.surf/history.$$ | sort | uniq >~/.surf/history
rm -f ~/.surf/history.$$
