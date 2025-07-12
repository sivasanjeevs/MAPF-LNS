#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <chrono>
#include <thread>
#include <random>
#include "RealTimeMAPF.h"

using namespace std;

// Warehouse task types
enum TaskType {
    PICKUP,     // Pick up item from location
    DROPOFF,    // Drop off item at location
    RECHARGE,   // Go to charging station
    IDLE        // No task assigned
};

// Warehouse task structure
struct WarehouseTask {
    int agent_id;
    TaskType type;
    int location;
    double priority;
    double assigned_time;
    
    WarehouseTask(int agent, TaskType t, int loc, double prio = 1.0) 
        : agent_id(agent), type(t), location(loc), priority(prio), assigned_time(0) {}
};

class WarehouseSimulator {
private:
    RealTimeMAPF* rt_mapf;
    const Instance& instance;
    vector<WarehouseTask> pending_tasks;
    vector<WarehouseTask> completed_tasks;
    double current_time;
    double simulation_duration;
    int max_agents;
    
    // Random generators for realistic warehouse scenarios
    random_device rd;
    mt19937 gen;
    uniform_int_distribution<int> location_dist;
    uniform_real_distribution<double> priority_dist;
    
public:
    WarehouseSimulator(const Instance& instance, double duration, int num_agents, 
                      const string& replan_algo = "PP")
        : instance(instance), current_time(0), simulation_duration(duration), max_agents(num_agents),
          gen(rd()), location_dist(0, instance.map_size - 1), priority_dist(0.1, 1.0)
    {
        rt_mapf = new RealTimeMAPF(instance, duration, replan_algo, true);
    }
    
    ~WarehouseSimulator() {
        delete rt_mapf;
    }
    
    // Main simulation loop
    void run() {
        cout << "Starting warehouse simulation..." << endl;
        cout << "Simulation duration: " << simulation_duration << " seconds" << endl;
        cout << "Number of agents: " << max_agents << endl;
        
        auto start_time = chrono::high_resolution_clock::now();
        
        while (current_time < simulation_duration) {
            // Update real-time MAPF system
            rt_mapf->update(current_time);
            
            // Generate new tasks randomly
            generateRandomTasks();
            
            // Assign tasks to idle agents
            assignTasksToIdleAgents();
            
            // Print status every 10 seconds
            if ((int)current_time % 10 == 0) {
                printStatus();
            }
            
            // Simulate time passing
            current_time += 1.0; // 1 second per iteration
            
            // Small delay to simulate real-time operation
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        
        auto end_time = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
        
        cout << "\nSimulation completed!" << endl;
        cout << "Total runtime: " << duration.count() << " ms" << endl;
        cout << "Total tasks completed: " << completed_tasks.size() << endl;
        cout << "Total conflicts: " << rt_mapf->getNumConflicts() << endl;
        
        // Save statistics
        rt_mapf->writeStatsToFile("warehouse_stats.txt");
    }
    
    // Generate random warehouse tasks
    void generateRandomTasks() {
        // Generate new tasks with some probability
        if (gen() % 100 < 20) { // 20% chance per second
            int agent_id = gen() % max_agents;
            TaskType task_type = (TaskType)(gen() % 3); // PICKUP, DROPOFF, RECHARGE
            int location = location_dist(gen);
            double priority = priority_dist(gen);
            
            // Make sure location is not an obstacle
            while (instance.isObstacle(location)) {
                location = location_dist(gen);
            }
            
            WarehouseTask task(agent_id, task_type, location, priority);
            task.assigned_time = current_time;
            pending_tasks.push_back(task);
            
            cout << "Generated task: Agent " << agent_id << " -> " 
                 << getTaskTypeString(task_type) << " at location " << location << endl;
        }
    }
    
    // Assign tasks to idle agents
    void assignTasksToIdleAgents() {
        vector<RealTimeAgent*> idle_agents = rt_mapf->getIdleAgents();
        
        // Sort pending tasks by priority (highest first)
        sort(pending_tasks.begin(), pending_tasks.end(), 
             [](const WarehouseTask& a, const WarehouseTask& b) {
                 return a.priority > b.priority;
             });
        
        // Assign tasks to idle agents
        for (auto task : pending_tasks) {
            if (task.agent_id < idle_agents.size() && idle_agents[task.agent_id]->isIdle()) {
                // Assign goal to agent
                if (rt_mapf->assignGoal(task.agent_id, task.location)) {
                    cout << "Assigned task: Agent " << task.agent_id << " -> " 
                         << getTaskTypeString(task.type) << " at location " << task.location << endl;
                    
                    // Move task to completed (will be marked as completed when agent reaches goal)
                    completed_tasks.push_back(task);
                }
            }
        }
        
        // Remove assigned tasks from pending
        pending_tasks.clear();
    }
    
    // Print current simulation status
    void printStatus() {
        cout << "\n=== Time: " << (int)current_time << "s ===" << endl;
        cout << "Idle agents: " << rt_mapf->getIdleAgents().size() << endl;
        cout << "Moving agents: " << rt_mapf->getMovingAgents().size() << endl;
        cout << "Pending tasks: " << pending_tasks.size() << endl;
        cout << "Completed tasks: " << completed_tasks.size() << endl;
        cout << "Total conflicts: " << rt_mapf->getNumConflicts() << endl;
        
        // Print agent locations
        cout << "Agent locations: ";
        for (int i = 0; i < max_agents; i++) {
            int loc = rt_mapf->getAgentLocation(i);
            AgentStatus status = rt_mapf->getAgentStatus(i);
            cout << "A" << i << "(" << loc << "," << getStatusString(status) << ") ";
        }
        cout << endl;
    }
    
    // Helper functions
    string getTaskTypeString(TaskType type) {
        switch (type) {
            case PICKUP: return "PICKUP";
            case DROPOFF: return "DROPOFF";
            case RECHARGE: return "RECHARGE";
            case IDLE: return "IDLE";
            default: return "UNKNOWN";
        }
    }
    
    string getStatusString(AgentStatus status) {
        switch (status) {
            case IDLE: return "IDLE";
            case MOVING: return "MOVING";
            case ARRIVED: return "ARRIVED";
            case REASSIGNING: return "REASSIGNING";
            default: return "UNKNOWN";
        }
    }
};

int main(int argc, char** argv) {
    namespace po = boost::program_options;
    
    // Declare the supported options
    po::options_description desc("Real-time Warehouse MAPF Simulator");
    desc.add_options()
        ("help", "produce help message")
        ("map,m", po::value<string>()->required(), "input file for map")
        ("agents,a", po::value<string>()->required(), "input file for agents")
        ("agentNum,k", po::value<int>()->default_value(10), "number of agents")
        ("duration,t", po::value<double>()->default_value(300), "simulation duration (seconds)")
        ("replanAlgo,r", po::value<string>()->default_value("PP"), 
         "replanning algorithm (PP, CBS, EECBS)")
        ("output,o", po::value<string>()->default_value("warehouse_output"), "output file name")
    ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    
    if (vm.count("help")) {
        cout << desc << endl;
        return 1;
    }
    
    po::notify(vm);
    
    try {
        // Create instance
        Instance instance(vm["map"].as<string>(), vm["agents"].as<string>(),
                        vm["agentNum"].as<int>());
        
        // Create and run warehouse simulator
        WarehouseSimulator simulator(instance, 
                                   vm["duration"].as<double>(),
                                   vm["agentNum"].as<int>(),
                                   vm["replanAlgo"].as<string>());
        
        simulator.run();
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
} 