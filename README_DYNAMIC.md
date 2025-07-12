# Dynamic Multi-Agent Pathfinding (MAPF) System

This project has been successfully modified to support **dynamic agent addition in real-time**. You can now add new agents while existing agents are moving, and the system will automatically replan paths to avoid collisions.

## 🎯 Key Features

- **Real-time Agent Addition**: Click on the grid to add new agents while others are moving
- **Automatic Collision Avoidance**: The system replans all paths when new agents are added
- **Interactive Visualization**: Use mouse and keyboard to control the simulation
- **Dynamic Pathfinding**: Uses LNS (Large Neighborhood Search) algorithm for efficient replanning
- **No Build Required**: Uses the existing `lns` executable

## 🚀 Quick Start

### 1. Test the System
```bash
python3 test_dynamic.py
```

### 2. Start Dynamic Visualization
```bash
./run_dynamic.sh
```

### 3. Start with Initial Agents
```bash
./run_dynamic.sh --with-initial
```

## 🎮 Controls

### Keyboard Controls
- **SPACE**: Pause/Resume simulation
- **LEFT/RIGHT ARROWS**: Step through timesteps manually
- **A**: Start adding a new agent (then click start and goal positions)
- **R**: Force replan all paths
- **ESC**: Quit

### Mouse Controls
- **Left Click**: Select positions when adding agents
- **A + Click**: Add new agent (first click = start, second click = goal)

## 🔧 How It Works

1. **Initial Setup**: The system loads the map and optionally some initial agents
2. **Real-time Simulation**: Agents move along their planned paths
3. **Dynamic Addition**: When you press 'A' and select start/goal positions:
   - The system adds the new agent
   - Calls the C++ LNS solver to replan all paths
   - Ensures collision avoidance for all agents
4. **Continuous Operation**: The simulation continues with the new paths

## 📁 Files

- `dynamic_visualizer.py`: Main visualization and interaction system
- `run_dynamic.sh`: Build and run script
- `test_dynamic.py`: Test script to verify system functionality
- `DYNAMIC_README.md`: This documentation
- `lns`: Existing C++ pathfinding executable (used by the dynamic system)

## 🏗️ Architecture

- **Python Frontend**: Handles visualization and user interaction using Pygame
- **C++ Backend**: Performs the actual pathfinding using LNS algorithm
- **Inter-process Communication**: Python calls C++ executable for pathfinding

## 🧮 Algorithms Used

- **LNS (Large Neighborhood Search)**: Main pathfinding algorithm
- **Space-Time A***: Single-agent pathfinding component
- **SIPP**: Safe Interval Path Planning for efficiency

## 📊 Performance

- Pathfinding time increases with the number of agents
- The system uses a 30-second timeout for replanning
- For large numbers of agents (>20), consider starting with fewer agents

## 🎯 Example Usage

1. Start the system: `./run_dynamic.sh`
2. Watch the initial agents move (if any)
3. Press 'A' to add a new agent
4. Click on a free cell for the start position (green highlight)
5. Click on another free cell for the goal position (red highlight)
6. The system will automatically replan and the new agent will start moving
7. Repeat to add more agents as needed

## 🔍 Troubleshooting

### Common Issues

1. **"lns executable not found"**
   - Make sure the `lns` executable is in the current directory
   - Run the original command first: `./lns --map random-32-32-20.map --agents random-32-32-20-random-1.scen --agentNum 10 --outputPaths paths.txt`

2. **"pygame not found"**
   - Install pygame: `pip install pygame`

3. **"Map file not found"**
   - Make sure `random-32-32-20.map` exists in the current directory

### Performance Tips

- Start with fewer agents for faster replanning
- Use the 'R' key to force replan if paths seem suboptimal
- The system automatically handles collision avoidance

## 🎨 Customization

You can modify the system by:
- Changing the map file in the script
- Adjusting the pathfinding timeout in `dynamic_visualizer.py`
- Adding new visualization features
- Modifying the LNS parameters

## ✅ Verification

The system has been tested and verified to work with:
- ✅ Pathfinding with existing `lns` executable
- ✅ Dynamic agent addition
- ✅ Collision avoidance
- ✅ Real-time visualization
- ✅ Interactive controls

## 🎉 Success!

Your MAPF project now supports dynamic agent addition! You can:
- Add agents in real-time while others are moving
- Watch automatic collision avoidance
- Interact with the simulation using mouse and keyboard
- Enjoy smooth, continuous operation

The system maintains all the original functionality while adding the dynamic capabilities you requested. 