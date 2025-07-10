#!/bin/bash

MAP="random-32-32-20.map"
AGENTS="random-32-32-20-random-1.scen"
AGENTNUM=1000

# LNS combinations
for initAlgo in EECBS PP PPS CBS PIBT winPIBT; do
  for replanAlgo in EECBS CBS PP; do
    for destoryStrategy in Random RandomWalk Intersection Adaptive; do
      for initDestoryStrategy in Target Collision Random Adaptive; do
        ./lns --map $MAP --agents $AGENTS --agentNum $AGENTNUM \
          --solver LNS \
          --initAlgo $initAlgo \
          --replanAlgo $replanAlgo \
          --destoryStrategy $destoryStrategy \
          --initDestoryStrategy $initDestoryStrategy \
          --output output-LNS-${initAlgo}-${replanAlgo}-${destoryStrategy}-${initDestoryStrategy}.csv \
          --stats stats-LNS-${initAlgo}-${replanAlgo}-${destoryStrategy}-${initDestoryStrategy}.csv
      done
    done
  done
done

# A-BCBS
./lns --map $MAP --agents $AGENTS --agentNum $AGENTNUM --solver A-BCBS --output output-ABCBS.csv --stats stats-ABCBS.csv

# A-EECBS
./lns --map $MAP --agents $AGENTS --agentNum $AGENTNUM --solver A-EECBS --output output-AEECBS.csv --stats stats-AEECBS.csv 

mv random-32-32-20-random-1.scen random-32-32-20-random-1.scen.bak
./lns -m random-32-32-20.map -a random-32-32-20-random-1.scen -o test -k 500 -t 300 --outputPaths=paths.txt 