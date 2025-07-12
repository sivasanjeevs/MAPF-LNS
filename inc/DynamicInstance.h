#pragma once
#include "Instance.h"
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>

// Dynamic task structure for dynamic goal assignment
struct DynamicTask {
    int agent_id;
    int goal_location;
    int priority;  // Higher number = higher priority
    double timestamp;  // When the task was assigned
    bool completed;
    
    DynamicTask(int agent, int goal, int prio = 1) : 
        agent_id(agent), goal_location(goal), priority(prio), 
        timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0),
        completed(false) {}
};

// Dynamic agent status for real-time tracking
struct DynamicAgentStatus {
    int current_location;
    int current_goal;
    bool has_task;
    double last_update_time;
    std::vector<int> path_to_goal;
    
    DynamicAgentStatus() : current_location(-1), current_goal(-1), has_task(false), last_update_time(0.0) {}
};

class DynamicInstance : public Instance {
private:
    std::queue<DynamicTask> task_queue;
    std::vector<DynamicAgentStatus> agent_statuses;
    mutable std::mutex task_mutex;
    mutable std::mutex status_mutex;
    bool simulation_running;
    std::thread simulation_thread;
    
    // Real-time simulation parameters
    double simulation_speed;  // Time multiplier (1.0 = real-time)
    double last_simulation_time;
    
public:
    DynamicInstance(const string& map_fname, const string& agent_fname, 
                   int num_of_agents = 0, int num_of_rows = 0, int num_of_cols = 0, 
                   int num_of_obstacles = 0, int warehouse_width = 0);
    ~DynamicInstance();
    
    // Dynamic goal assignment methods
    void assignGoal(int agent_id, int goal_location, int priority = 1);
    void assignRandomGoal(int agent_id, int priority = 1);
    void assignWarehouseTask(int agent_id, int priority = 1);
    
    // Real-time simulation control
    void startSimulation(double speed = 1.0);
    void stopSimulation();
    void updateSimulation(double current_time);
    
    // Status queries
    int getAgentLocation(int agent_id) const;
    int getAgentGoal(int agent_id) const;
    bool isAgentAtGoal(int agent_id) const;
    bool hasAgentTask(int agent_id) const;
    std::vector<int> getAgentPath(int agent_id) const;
    
    // Task management
    bool hasPendingTasks() const;
    DynamicTask getNextTask();
    void completeTask(int agent_id);
    
    // Warehouse-specific methods
    void generateWarehouseTasks(int num_tasks);
    std::pair<int, int> getWarehousePickupLocation();
    std::pair<int, int> getWarehouseDropoffLocation();
    
private:
    void simulationLoop();
    void updateAgentPositions(double current_time);
    bool isValidWarehouseLocation(int row, int col) const;
    int findNearestFreeLocation(int target_row, int target_col) const;
}; 