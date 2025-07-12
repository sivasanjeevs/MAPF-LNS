# Real-Time MAPF for Warehouse Robots

This extension implements **real-time Multi-Agent Path Finding (MAPF)** for warehouse robots where goals can be assigned dynamically during execution.

## 🎯 Key Features

### **Real-Time Goal Assignment**
- **Dynamic Goals**: Assign new goals to robots anytime during execution
- **Goal Queuing**: Handle multiple goal assignments per robot
- **Priority-Based**: Support task priorities for warehouse operations
- **Conflict-Free**: Maintain collision-free paths during replanning

### **Agent States**
- **IDLE**: Robot waiting for new task assignment
- **MOVING**: Robot executing current path to goal
- **ARRIVED**: Robot reached goal, ready for next assignment
- **REASSIGNING**: Robot being assigned new goal while moving

### **Replanning Algorithms**
- **PP (Prioritized Planning)**: Fast, suboptimal replanning
- **CBS**: Optimal conflict-based replanning
- **EECBS**: Enhanced CBS for better performance

## 🏗️ Architecture

### **Core Components**

**`RealTimeAgent`**
- Manages individual robot state and path
- Handles goal assignments and path replanning
- Tracks position and movement over time

**`RealTimeMAPF`**
- Coordinates all robots in real-time
- Manages path table and conflict detection
- Provides API for goal assignment and status queries

**`WarehouseSimulator`**
- Simulates realistic warehouse scenarios
- Generates random tasks (pickup, dropoff, recharge)
- Demonstrates real-time operation

## 🚀 Usage

### **Basic Real-Time Operation**

```cpp
#include "RealTimeMAPF.h"

// Create instance and real-time MAPF system
Instance instance("warehouse.map", "agents.scen", 10);
RealTimeMAPF rt_mapf(instance, 300.0, "PP", true);

// Main real-time loop
double current_time = 0.0;
while (current_time < simulation_duration) {
    // Update system
    rt_mapf.update(current_time);
    
    // Assign goals dynamically
    rt_mapf.assignGoal(0, new_goal_location);
    
    // Query status
    vector<RealTimeAgent*> idle_agents = rt_mapf.getIdleAgents();
    int conflicts = rt_mapf.getNumConflicts();
    
    current_time += time_step;
}
```

### **Warehouse Task Assignment**

```cpp
// Assign multiple goals with priorities
vector<pair<int, int>> tasks = {
    {0, 100},  // Agent 0 -> Location 100
    {1, 200},  // Agent 1 -> Location 200
    {2, 300}   // Agent 2 -> Location 300
};
rt_mapf.assignGoals(tasks);
```

### **Command Line Usage**

```bash
# Run warehouse simulation
./realtime_warehouse -m warehouse.map -a agents.scen -k 20 -t 600 -r PP

# Run simple example
./realtime_example
```

## 📊 Real-Time vs Offline MAPF

| Feature | Offline MAPF | Real-Time MAPF |
|---------|-------------|----------------|
| **Goal Assignment** | All goals known upfront | Dynamic assignment anytime |
| **Path Planning** | One-time computation | Continuous replanning |
| **Conflict Resolution** | Pre-computed | Real-time detection |
| **Scalability** | Limited by computation time | Limited by replanning speed |
| **Robustness** | Fragile to changes | Adapts to new goals |

## 🔧 Implementation Details

### **Goal Assignment Strategies**

1. **Immediate Assignment**: For idle robots
   ```cpp
   if (agent->status == IDLE) {
       agent->current_goal = new_goal;
       agent->status = REASSIGNING;
       agent->needs_replanning = true;
   }
   ```

2. **Queued Assignment**: For moving robots
   ```cpp
   if (agent->status == MOVING) {
       agent->next_goal = new_goal; // Store for later
   }
   ```

### **Path Replanning**

1. **Single Agent**: Use existing path planner
2. **Multiple Agents**: Use specified algorithm (PP/CBS/EECBS)
3. **Conflict Resolution**: Update path table and detect conflicts

### **Time Management**

- **Discrete Time**: System updates at regular intervals
- **Position Tracking**: Agents move along paths based on time
- **Goal Detection**: Check if agent reached current goal

## 🏭 Warehouse Integration

### **Task Types**
- **PICKUP**: Robot picks up item from location
- **DROPOFF**: Robot delivers item to location  
- **RECHARGE**: Robot goes to charging station
- **IDLE**: No task assigned

### **Priority System**
- High priority tasks assigned first
- Emergency tasks can interrupt current goals
- Battery level affects task assignment

### **Real-World Considerations**
- **Communication Delays**: Handle network latency
- **Robot Failures**: Remove failed robots from system
- **Dynamic Obstacles**: Update map in real-time
- **Battery Management**: Assign recharge tasks

## 📈 Performance Metrics

### **Real-Time Metrics**
- **Response Time**: Time to assign new goal
- **Replanning Time**: Time to compute new path
- **Conflict Rate**: Number of conflicts per time unit
- **Task Completion Rate**: Tasks completed per time unit

### **Quality Metrics**
- **Path Length**: Total distance traveled
- **Makespan**: Time to complete all tasks
- **Flowtime**: Average time per task
- **Throughput**: Tasks completed per time unit

## 🔄 Comparison with Existing Systems

### **Advantages of Real-Time MAPF**
1. **Flexibility**: Handle dynamic warehouse operations
2. **Responsiveness**: Quick adaptation to new tasks
3. **Scalability**: Add/remove robots dynamically
4. **Robustness**: Handle failures and changes

### **Challenges**
1. **Computation Overhead**: Continuous replanning
2. **Path Quality**: Suboptimal paths due to time constraints
3. **Conflict Management**: Real-time conflict resolution
4. **Predictability**: Harder to predict system behavior

## 🛠️ Building and Running

### **Build Instructions**
```bash
# Build real-time components
cmake -DCMAKE_BUILD_TYPE=RELEASE -f CMakeLists_realtime.txt .
make

# Build with existing MAPF library
cmake -DCMAKE_BUILD_TYPE=RELEASE .
make realtime_warehouse realtime_example
```

### **Example Commands**
```bash
# Run warehouse simulation
./realtime_warehouse -m random-32-32-20.map -a random-32-32-20-random-1.scen -k 10 -t 300 -r PP

# Run simple example
./realtime_example

# Run with different algorithms
./realtime_warehouse -m warehouse.map -a agents.scen -k 20 -t 600 -r CBS
```

## 🔮 Future Enhancements

### **Planned Features**
1. **Predictive Replanning**: Anticipate future conflicts
2. **Machine Learning**: Learn from past replanning decisions
3. **Distributed Planning**: Multi-robot coordination
4. **Human-in-the-Loop**: Interactive goal assignment

### **Research Directions**
1. **Online Learning**: Adapt replanning strategies
2. **Multi-Objective**: Balance multiple objectives
3. **Uncertainty**: Handle uncertain environments
4. **Scalability**: Support hundreds of robots

## 📚 References

- **Real-Time MAPF**: [Paper on real-time multi-agent path finding]
- **Warehouse Robotics**: [Survey of warehouse automation]
- **Dynamic Replanning**: [Methods for online path planning]

## 🤝 Contributing

To contribute to the real-time MAPF system:

1. **Fork** the repository
2. **Create** a feature branch
3. **Implement** your changes
4. **Test** with warehouse scenarios
5. **Submit** a pull request

## 📄 License

This real-time MAPF extension is released under the same license as the original MAPF-LNS project. 