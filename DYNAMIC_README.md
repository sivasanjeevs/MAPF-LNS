# Dynamic Multi-Agent Pathfinding (MAPF) System

This project has been extended to support dynamic agent addition in real-time. You can now add new agents while existing agents are moving, and the system will automatically replan paths to avoid collisions.

## Features

- **Real-time Agent Addition**: Click on the grid to add new agents while others are moving
- **Automatic Collision Avoidance**: The system replans all paths when new agents are added
- **Interactive Visualization**: Use mouse and keyboard to control the simulation
- **Dynamic Pathfinding**: Uses LNS (Large Neighborhood Search) algorithm for efficient replanning

## How to Use

### 1. Build the System

```bash
./run_dynamic.sh
```

This will:
- Build the C++ pathfinding engine
- Start the dynamic visualization

### 2. Alternative: Start with Initial Agents

```bash
./run_dynamic.sh --with-initial
```

This starts with 10 pre-defined agents and allows you to add more.

### 3. Manual Build (if needed)

```bash
mkdir -p build
cd build
cmake ..
make -j4
cd ..
cp build/dynamic_lns ./dynamic_lns
python3 dynamic_visualizer.py random-32-32-20.map
```

## Controls

### Keyboard Controls
- **SPACE**: Pause/Resume simulation
- **LEFT/RIGHT ARROWS**: Step through timesteps manually
- **A**: Start adding a new agent (then click start and goal positions)
- **R**: Force replan all paths
- **ESC**: Quit

### Mouse Controls
- **Left Click**: Select positions when adding agents
- **A + Click**: Add new agent (first click = start, second click = goal)

## How It Works

1. **Initial Setup**: The system loads the map and optionally some initial agents
2. **Real-time Simulation**: Agents move along their planned paths
3. **Dynamic Addition**: When you press 'A' and select start/goal positions:
   - The system adds the new agent
   - Calls the C++ LNS solver to replan all paths
   - Ensures collision avoidance for all agents
4. **Continuous Operation**: The simulation continues with the new paths

## Technical Details

### Architecture
- **Python Frontend**: Handles visualization and user interaction
- **C++ Backend**: Performs the actual pathfinding using LNS algorithm
- **Inter-process Communication**: Python calls C++ executable for pathfinding

### Algorithms Used
- **LNS (Large Neighborhood Search)**: Main pathfinding algorithm
- **Space-Time A***: Single-agent pathfinding component
- **SIPP**: Safe Interval Path Planning for efficiency

### Files
- `dynamic_visualizer.py`: Main visualization and interaction system
- `src/dynamic_driver.cpp`: C++ driver for dynamic pathfinding
- `run_dynamic.sh`: Build and run script

## Troubleshooting

### Build Issues
- Ensure you have CMake, Boost, and Eigen3 installed
- On Ubuntu: `sudo apt-get install cmake libboost-all-dev libeigen3-dev`

### Runtime Issues
- Make sure the `dynamic_lns` executable is in the current directory
- Check that map files exist and are readable
- Ensure Python has pygame installed: `pip install pygame`

### Performance
- Pathfinding time increases with the number of agents
- The system uses a 30-second timeout for replanning
- For large numbers of agents (>20), consider starting with fewer agents

## Example Usage

1. Start the system: `./run_dynamic.sh`
2. Watch the initial agents move (if any)
3. Press 'A' to add a new agent
4. Click on a free cell for the start position (green highlight)
5. Click on another free cell for the goal position (red highlight)
6. The system will automatically replan and the new agent will start moving
7. Repeat to add more agents as needed

## Customization

You can modify the system by:
- Changing the map file in the script
- Adjusting the pathfinding timeout in `dynamic_visualizer.py`
- Modifying the LNS parameters in `dynamic_driver.cpp`
- Adding new visualization features in `dynamic_visualizer.py` 