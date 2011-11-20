#!/usr/bin/env sh
./playgame.py --scenario --player_seed 42 --food=none --end_wait=0.25 --verbose --log_dir game_logs --turns 500 --map_file maps/example/tactical.map "$@"
