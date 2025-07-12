#include "DynamicInstance.h"
#include <algorithm>
#include <random>
#include <iostream>

DynamicInstance::DynamicInstance(const string& map_fname, const string& agent_fname, 
                               int num_of_agents, int num_of_rows, int num_of_cols, 
                               int num_of_obstacles, int warehouse_width) :
    Instance(map_fname, agent_fname, num_of_agents, num_of_rows, num_of_cols, num_of_obstacles, warehouse_width),
    simulation_running(false),
    simulation_speed(1.0),
    last_simulation_time(0.0)
{
    // Initialize agent statuses
    agent_statuses.resize(num_of_agents);
    for (int i = 0; i < num_of_agents; i++) {
        agent_statuses[i].current_location = start_locations[i];
        agent_statuses[i].current_goal = goal_locations[i];
        agent_statuses[i].has_task = true;
        agent_statuses[i].last_update_time = 0.0;
    }
}

DynamicInstance::~DynamicInstance() {
    stopSimulation();
}

void DynamicInstance::assignGoal(int agent_id, int goal_location, int priority) {
    if (agent_id < 0 || agent_id >= num_of_agents) {
        std::cerr << "Invalid agent ID: " << agent_id << std::endl;
        return;
    }
    
    if (goal_location < 0 || goal_location >= map_size || isObstacle(goal_location)) {
        std::cerr << "Invalid goal location: " << goal_location << std::endl;
        return;
    }
    
    std::lock_guard<std::mutex> lock(task_mutex);
    task_queue.push(Task(agent_id, goal_location, priority));
    
    std::lock_guard<std::mutex> status_lock(status_mutex);
    agent_statuses[agent_id].has_task = true;
    agent_statuses[agent_id].current_goal = goal_location;
}

void DynamicInstance::assignRandomGoal(int agent_id, int priority) {
    if (agent_id < 0 || agent_id >= num_of_agents) {
        std::cerr << "Invalid agent ID: " << agent_id << std::endl;
        return;
    }
    
    // Find a random free location
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, map_size - 1);
    
    int goal_location;
    do {
        goal_location = dis(gen);
    } while (isObstacle(goal_location));
    
    assignGoal(agent_id, goal_location, priority);
}

void DynamicInstance::assignWarehouseTask(int agent_id, int priority) {
    if (agent_id < 0 || agent_id >= num_of_agents) {
        std::cerr << "Invalid agent ID: " << agent_id << std::endl;
        return;
    }
    
    // For warehouse scenario, alternate between pickup and dropoff areas
    auto [pickup_row, pickup_col] = getWarehousePickupLocation();
    auto [dropoff_row, dropoff_col] = getWarehouseDropoffLocation();
    
    int goal_location;
    if (agent_id % 2 == 0) {
        // Even agents go to pickup area
        goal_location = linearizeCoordinate(pickup_row, pickup_col);
    } else {
        // Odd agents go to dropoff area
        goal_location = linearizeCoordinate(dropoff_row, dropoff_col);
    }
    
    assignGoal(agent_id, goal_location, priority);
}

void DynamicInstance::startSimulation(double speed) {
    if (simulation_running) {
        std::cerr << "Simulation already running!" << std::endl;
        return;
    }
    
    simulation_speed = speed;
    simulation_running = true;
    last_simulation_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
    
    simulation_thread = std::thread(&DynamicInstance::simulationLoop, this);
    std::cout << "Dynamic simulation started with speed: " << speed << "x" << std::endl;
}

void DynamicInstance::stopSimulation() {
    if (!simulation_running) return;
    
    simulation_running = false;
    if (simulation_thread.joinable()) {
        simulation_thread.join();
    }
    std::cout << "Dynamic simulation stopped" << std::endl;
}

void DynamicInstance::updateSimulation(double current_time) {
    updateAgentPositions(current_time);
    
    // Check if agents reached their goals
    std::lock_guard<std::mutex> status_lock(status_mutex);
    for (int i = 0; i < num_of_agents; i++) {
        if (agent_statuses[i].has_task && 
            agent_statuses[i].current_location == agent_statuses[i].current_goal) {
            completeTask(i);
        }
    }
}

void DynamicInstance::simulationLoop() {
    while (simulation_running) {
        auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
        
        updateSimulation(current_time);
        
        // Sleep for a short interval
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void DynamicInstance::updateAgentPositions(double current_time) {
    std::lock_guard<std::mutex> lock(status_mutex);
    
    for (int i = 0; i < num_of_agents; i++) {
        if (!agent_statuses[i].has_task) continue;
        
        // Simple movement: move towards goal one step at a time
        if (agent_statuses[i].current_location != agent_statuses[i].current_goal) {
            // Get neighbors of current location
            auto neighbors = getNeighbors(agent_statuses[i].current_location);
            
            // Find the neighbor closest to the goal
            int best_neighbor = agent_statuses[i].current_location;
            int min_distance = getManhattanDistance(agent_statuses[i].current_location, agent_statuses[i].current_goal);
            
            for (int neighbor : neighbors) {
                int distance = getManhattanDistance(neighbor, agent_statuses[i].current_goal);
                if (distance < min_distance) {
                    min_distance = distance;
                    best_neighbor = neighbor;
                }
            }
            
            // Move to the best neighbor
            if (best_neighbor != agent_statuses[i].current_location) {
                agent_statuses[i].current_location = best_neighbor;
                agent_statuses[i].last_update_time = current_time;
            }
        }
    }
}

int DynamicInstance::getAgentLocation(int agent_id) const {
    if (agent_id < 0 || agent_id >= num_of_agents) return -1;
    
    std::lock_guard<std::mutex> lock(status_mutex);
    return agent_statuses[agent_id].current_location;
}

int DynamicInstance::getAgentGoal(int agent_id) const {
    if (agent_id < 0 || agent_id >= num_of_agents) return -1;
    
    std::lock_guard<std::mutex> lock(status_mutex);
    return agent_statuses[agent_id].current_goal;
}

bool DynamicInstance::isAgentAtGoal(int agent_id) const {
    if (agent_id < 0 || agent_id >= num_of_agents) return false;
    
    std::lock_guard<std::mutex> lock(status_mutex);
    return agent_statuses[agent_id].current_location == agent_statuses[agent_id].current_goal;
}

bool DynamicInstance::hasAgentTask(int agent_id) const {
    if (agent_id < 0 || agent_id >= num_of_agents) return false;
    
    std::lock_guard<std::mutex> lock(status_mutex);
    return agent_statuses[agent_id].has_task;
}

std::vector<int> DynamicInstance::getAgentPath(int agent_id) const {
    if (agent_id < 0 || agent_id >= num_of_agents) return {};
    
    std::lock_guard<std::mutex> lock(status_mutex);
    return agent_statuses[agent_id].path_to_goal;
}

bool DynamicInstance::hasPendingTasks() const {
    std::lock_guard<std::mutex> lock(task_mutex);
    return !task_queue.empty();
}

Task DynamicInstance::getNextTask() {
    std::lock_guard<std::mutex> lock(task_mutex);
    if (task_queue.empty()) {
        return Task(-1, -1); // Invalid task
    }
    
    Task task = task_queue.front();
    task_queue.pop();
    return task;
}

void DynamicInstance::completeTask(int agent_id) {
    if (agent_id < 0 || agent_id >= num_of_agents) return;
    
    std::lock_guard<std::mutex> lock(status_mutex);
    agent_statuses[agent_id].has_task = false;
    std::cout << "Agent " << agent_id << " completed task at location " 
              << agent_statuses[agent_id].current_location << std::endl;
}

void DynamicInstance::generateWarehouseTasks(int num_tasks) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> agent_dis(0, num_of_agents - 1);
    std::uniform_int_distribution<> priority_dis(1, 5);
    
    for (int i = 0; i < num_tasks; i++) {
        int agent_id = agent_dis(gen);
        int priority = priority_dis(gen);
        assignWarehouseTask(agent_id, priority);
    }
}

std::pair<int, int> DynamicInstance::getWarehousePickupLocation() {
    // Pickup area: left side of warehouse
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> row_dis(0, num_of_rows - 1);
    std::uniform_int_distribution<> col_dis(0, num_of_cols / 4); // Left quarter
    
    int row, col;
    do {
        row = row_dis(gen);
        col = col_dis(gen);
    } while (isObstacle(linearizeCoordinate(row, col)));
    
    return {row, col};
}

std::pair<int, int> DynamicInstance::getWarehouseDropoffLocation() {
    // Dropoff area: right side of warehouse
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> row_dis(0, num_of_rows - 1);
    std::uniform_int_distribution<> col_dis(3 * num_of_cols / 4, num_of_cols - 1); // Right quarter
    
    int row, col;
    do {
        row = row_dis(gen);
        col = col_dis(gen);
    } while (isObstacle(linearizeCoordinate(row, col)));
    
    return {row, col};
}

bool DynamicInstance::isValidWarehouseLocation(int row, int col) const {
    if (row < 0 || row >= num_of_rows || col < 0 || col >= num_of_cols) return false;
    return !isObstacle(linearizeCoordinate(row, col));
}

int DynamicInstance::findNearestFreeLocation(int target_row, int target_col) const {
    // Simple BFS to find nearest free location
    std::queue<std::pair<int, int>> q;
    std::vector<std::vector<bool>> visited(num_of_rows, std::vector<bool>(num_of_cols, false));
    
    q.push({target_row, target_col});
    visited[target_row][target_col] = true;
    
    while (!q.empty()) {
        auto [row, col] = q.front();
        q.pop();
        
        int location = linearizeCoordinate(row, col);
        if (!isObstacle(location)) {
            return location;
        }
        
        // Check neighbors
        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        for (int i = 0; i < 4; i++) {
            int new_row = row + directions[i][0];
            int new_col = col + directions[i][1];
            
            if (new_row >= 0 && new_row < num_of_rows && 
                new_col >= 0 && new_col < num_of_cols && 
                !visited[new_row][new_col]) {
                visited[new_row][new_col] = true;
                q.push({new_row, new_col});
            }
        }
    }
    
    return -1; // No free location found
} 