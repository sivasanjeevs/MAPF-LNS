# MAPF-LNS2 (LNS(PP;PP) Only)

This project is now restricted to only support the LNS(PP;PP) case for Multi-Agent Path Finding (MAPF).

## Usage

### Running the Solver

To find paths for agents, run:

```
./lns --map random-32-32-20.map --agents random-32-32-20-random-1.scen --agentNum 10 --outputPaths paths.txt
```

This command will:
- Use the random-32-32-20.map file as the map
- Use random-32-32-20-random-1.scen as the scenario file
- Solve for 10 agents
- Output the paths to paths.txt

### Visualizing Paths

To visualize the computed paths using the UI, run:

```
python3 visualize_paths_pygame.py paths.txt
```

This will open a pygame-based visualization showing the agent paths.

## Maintainers
This version is a minimal, single-case fork of the original MAPF-LNS2 project.
