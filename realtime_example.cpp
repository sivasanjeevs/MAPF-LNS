#include "RealTimeMAPF.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

int main() {
    cout << "=== Real-Time Warehouse MAPF Example ===" << endl;
    
    // Create a simple instance (you can replace with your own map/agents)
    Instance instance("random-32-32-20.map", "random-32-32-20-random-1.scen", 5);
    
    // Create real-time MAPF system
    RealTimeMAPF rt_mapf(instance, 300.0, "PP", true); // 5 minutes, PP algorithm, SIPP
    
    cout << "Initialized with " << instance.getDefaultNumberOfAgents() << " agents" << endl;
    
    // Simulate real-time operation
    double current_time = 0.0;
    double time_step = 1.0; // 1 second per update
    
    while (current_time < 60.0) { // Run for 60 seconds
        cout << "\n=== Time: " << (int)current_time << "s ===" << endl;
        
        // Update the system
        rt_mapf.update(current_time);
        
        // Assign some goals to agents
        if (current_time == 10.0) {
            cout << "Assigning goals to agents..." << endl;
            rt_mapf.assignGoal(0, 100); // Agent 0 -> location 100
            rt_mapf.assignGoal(1, 200); // Agent 1 -> location 200
            rt_mapf.assignGoal(2, 300); // Agent 2 -> location 300
        }
        
        if (current_time == 20.0) {
            cout << "Assigning new goals while agents are moving..." << endl;
            rt_mapf.assignGoal(0, 150); // Change agent 0's goal
            rt_mapf.assignGoal(3, 250); // Assign goal to agent 3
        }
        
        if (current_time == 30.0) {
            cout << "Assigning goals to idle agents..." << endl;
            rt_mapf.assignGoal(4, 350); // Agent 4 -> location 350
        }
        
        // Print status
        cout << "Idle agents: " << rt_mapf.getIdleAgents().size() << endl;
        cout << "Moving agents: " << rt_mapf.getMovingAgents().size() << endl;
        cout << "Total conflicts: " << rt_mapf.getNumConflicts() << endl;
        
        // Print agent locations
        cout << "Agent locations: ";
        for (int i = 0; i < 5; i++) {
            int loc = rt_mapf.getAgentLocation(i);
            AgentStatus status = rt_mapf.getAgentStatus(i);
            cout << "A" << i << "(" << loc << ") ";
        }
        cout << endl;
        
        // Simulate time passing
        current_time += time_step;
        this_thread::sleep_for(chrono::milliseconds(500)); // 0.5 second delay
    }
    
    cout << "\n=== Simulation Complete ===" << endl;
    cout << "Final statistics:" << endl;
    cout << "Total cost: " << rt_mapf.getTotalCost() << endl;
    cout << "Total conflicts: " << rt_mapf.getNumConflicts() << endl;
    
    return 0;
} 