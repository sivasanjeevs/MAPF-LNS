# MAPF-LNS2 (LNS(PP;PP))

This project is now restricted to only support the LNS(PP;PP) case for Multi-Agent Path Finding (MAPF).

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

---

# Collision Detection and Timing Features

The dynamic MAPF system now includes comprehensive collision detection to ensure path safety and provide real-time feedback.

## 🚨 Collision Detection System

### **Types of Collisions Detected:**

1. **Vertex Collisions**: Two agents occupy the same position at the same time
2. **Edge Collisions**: Two agents swap positions (cross paths) at the same time

### **When Collisions Are Checked:**

1. **After Replanning**: Every time paths are replanned (when adding new agents)
2. **During Simulation**: Every 10 timesteps during the simulation
3. **Manual Check**: Press 'C' key for immediate collision check

### **Collision Logging:**

When collisions are detected, you'll see console output like:
```
🚨 COLLISION DETECTED! Found 2 collision(s):
   Vertex collision: Agents (0, 1) at position (10, 15) at timestep 25
   Edge collision: Agents (2, 3) swapping positions ((5, 5), (6, 6)) at timestep 30
```

Or during simulation:
```
🚨 COLLISION AT TIMESTEP 45! Found 1 collision(s):
   Vertex collision: Agents (1, 4) at position (12, 18)
```

### **No Collision Message:**
```
✅ No collisions detected - all paths are collision-free!
```

## ⏰ New Agent Timing Fix

### **Problem Solved:**
Previously, when you added new agents, they would start moving from the current timestep, which looked unnatural.

### **Solution:**
New agents now start from **timestep 0** and are properly synchronized with existing agents.

### **How It Works:**
1. When you add a new agent, the system calculates the current timestep
2. The new agent's path is padded with its start position for all previous timesteps
3. The agent appears to have been "waiting" at its start position until now
4. This creates a natural-looking entry into the simulation

### **Example:**
- Current timestep: 25
- You add a new agent at position (5, 5)
- The agent's path becomes: [(5,5), (5,5), ..., (5,5), (6,5), (7,5), ...]
- The agent appears to have been waiting at (5,5) for 25 timesteps, then starts moving

## 🎮 Enhanced Controls

### **New Keyboard Controls:**
- **C**: Manual collision check (prints current collision status to console)
- **R**: Replan all paths (also triggers collision check)
- **A**: Add new agent (with proper timing)
- **SPACE**: Pause/Resume simulation
- **LEFT/RIGHT ARROWS**: Step through timesteps
- **ESC**: Quit

### **Updated Instructions:**
The on-screen instructions now show: `SPACE: Pause/Play   ←/→: Step   ESC: Quit   A: Add Agent   R: Replan   C: Check Collisions`

## 🔍 How to Use Collision Detection

### **1. Automatic Detection:**
- Collisions are automatically checked after every replanning
- During simulation, collisions are checked every 10 timesteps
- Watch the console for collision messages

### **2. Manual Detection:**
- Press 'C' at any time to check for collisions
- Useful when you suspect there might be an issue
- Provides immediate feedback

### **3. Monitoring During Agent Addition:**
- When you add new agents, collision detection runs automatically
- You'll see either "✅ No collisions detected" or detailed collision information
- This ensures your new agents don't create conflicts

## 🎯 Benefits

### **Safety:**
- Real-time collision monitoring ensures path safety
- Immediate feedback if the pathfinding algorithm fails
- Helps identify potential issues in the MAPF solver

### **Debugging:**
- Detailed collision information helps understand what went wrong
- Timestep-specific collision detection helps pinpoint when issues occur
- Manual collision checking allows for on-demand verification

### **Natural Timing:**
- New agents enter the simulation naturally
- No more jarring "teleportation" of new agents
- Maintains visual consistency of the simulation

## 🚀 Example Usage

1. **Start the system:**
   ```bash
   ./run_dynamic.sh --with-initial
   ```

2. **Watch for collision messages** in the console as agents move

3. **Add a new agent** and observe:
   - Collision check runs automatically
   - New agent starts from timestep 0
   - Console shows collision status

4. **Press 'C'** to manually check for collisions at any time

5. **Monitor the simulation** for any collision warnings

## 🔧 Technical Details

### **Collision Detection Algorithm:**
- Compares all agent pairs at each timestep
- Checks both vertex and edge collisions
- Efficient implementation to avoid performance impact

### **Timing Adjustment:**
- Automatically detects when new agents are added
- Calculates proper padding for new agent paths
- Maintains synchronization with existing agents

### **Performance:**
- Collision checking every 10 timesteps to balance safety and performance
- Manual collision checks available on demand
- Efficient algorithms to handle large numbers of agents

## 🎉 Result

Your dynamic MAPF system now provides:
- ✅ **Real-time collision monitoring**
- ✅ **Natural agent timing**
- ✅ **Comprehensive safety checks**
- ✅ **Detailed debugging information**
- ✅ **Manual verification tools**

The system is now both safer and more user-friendly! 🛡️✨
