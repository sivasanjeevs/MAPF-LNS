# MAPF-LNS2 (LNS(PP;PP) Python Implementation)

This is a full-featured Python implementation of the LNS(PP;PP) algorithm for Multi-Agent Path Finding (MAPF).

## Features
- Loads .map and .scen files (MovingAI format)
- Prioritized Planning (PP) for initial and repair steps
- Large Neighborhood Search (LNS) metaheuristic
- Outputs solution cost, runtime, stats, and paths
- Uses only the Python standard library

## Usage
```sh
python -m mapf_lns_py --map <mapfile> --agents <scenfile> --agentNum <num> --neighborSize 8 --maxIterations 10 --cutoffTime 60 --outputPaths paths.txt --stats stats.csv --screen 1
```

- `--map`: Path to the .map file
- `--agents`: Path to the .scen file
- `--agentNum`: Number of agents (0 = all in .scen file)
- `--neighborSize`: Number of agents to replan in each LNS iteration
- `--maxIterations`: Maximum number of LNS iterations
- `--cutoffTime`: Time limit in seconds
- `--outputPaths`: Output file for agent paths
- `--stats`: Output file for iteration stats
- `--screen`: Verbosity (0 = none, 1 = summary)

## Example
```sh
python -m mapf_lns_py --map ../random-32-32-20.map --agents ../random-32-32-20-random-1.scen --agentNum 10 --outputPaths paths.txt --stats stats.csv
```

## Requirements
- Python 3.6+
- No external dependencies (standard library only) 