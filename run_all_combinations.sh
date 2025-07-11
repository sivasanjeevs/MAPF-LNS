#!/bin/bash

MAP="random-32-32-20.map"
AGENTS="random-32-32-20-random-1.scen"
AGENTNUM=1000

./lns --map $MAP --agents $AGENTS --agentNum $AGENTNUM \
  --solver LNS \
  --initAlgo PP \
  --replanAlgo PP \
  --destoryStrategy Adaptive \
  --initDestoryStrategy Adaptive \
  --output output-LNS-PP-PP-Adaptive-Adaptive.csv \
  --stats stats-LNS-PP-PP-Adaptive-Adaptive.csv

mv random-32-32-20-random-1.scen random-32-32-20-random-1.scen.bak
./lns -m random-32-32-20.map -a random-32-32-20-random-1.scen -o test -k 500 -t 300 --outputPaths=paths.txt 