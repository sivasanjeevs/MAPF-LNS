# MAPF-LNS2 (LNS(PP;PP))

This project is now restricted to only support the LNS(PP;PP) case for Multi-Agent Path Finding (MAPF).

## Usage

### Running the Solver

To find paths for agents, run:

for map1
```
./lns --map random-32-32-20.map --agents random-32-32-20-random-1.scen --agentNum 10 --outputPaths paths.txt
```
for map2 (large map)
```
./lns --map warehouse-20-40-10-2-2.map --agents instances/warehouse-20-40-10-2-2-10000agents-1.scen --agentNum 100 --outputPaths paths.txt
```


### Visualizing Paths

To visualize the computed paths using the UI, run:

for map1
```
python3 visualize_paths_pygame.py paths.txt
```
for map2 (large map)
```
python3 visualize_paths_pygame.py paths.txt warehouse-20-40-10-2-2.map
```

This will open a pygame-based visualization showing the agent paths.

## Maintainers
This version is a minimal, single-case fork of the original MAPF-LNS2 project.
