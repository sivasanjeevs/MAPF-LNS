#pragma once
#include "common.h"
#include "Instance.h"
#include "SingleAgentSolver.h"
#include "PathTable.h"
#include "LNS.h"
#include "CBS/CBS.h"
#include "PIBT/pibt.h"

// Real-time agent status
enum AgentStatus {
    IDLE,           // Agent is idle, waiting for new goal
    MOVING,         // Agent is moving to current goal
    ARRIVED,        // Agent has reached goal, waiting for next assignment
    REASSIGNING     // Agent is being reassigned to new goal
};

// Real-time agent with dynamic goals
struct RealTimeAgent {
    int id;
    AgentStatus status;
    int current_location;
    int current_goal;
    int next_goal;  // New goal assigned while moving
    Path current_path;
    int path_index;  // Current position in path
    double last_update_time;
    
    // For path replanning
    SingleAgentSolver* path_planner;
    bool needs_replanning;
    
    RealTimeAgent(const Instance& instance, int agent_id, bool use_sipp);
    ~RealTimeAgent();
    
    void updatePosition(double current_time);
    bool hasReachedGoal() const;
    void assignNewGoal(int new_goal);
    void replanPath(const PathTable& path_table);
    bool isIdle() const { return status == IDLE; }
    bool isMoving() const { return status == MOVING; }
};

class RealTimeMAPF {
public:
    RealTimeMAPF(const Instance& instance, double time_limit, 
                 const string& replan_algo = "PP", bool use_sipp = true);
    ~RealTimeMAPF();
    
    // Main real-time functions
    void update(double current_time);
    bool assignGoal(int agent_id, int new_goal);
    bool assignGoals(const vector<pair<int, int>>& agent_goal_pairs);
    void removeAgent(int agent_id);
    void addAgent(int agent_id, int start_location);
    
    // Status queries
    vector<RealTimeAgent*> getIdleAgents() const;
    vector<RealTimeAgent*> getMovingAgents() const;
    int getAgentLocation(int agent_id) const;
    AgentStatus getAgentStatus(int agent_id) const;
    
    // Statistics
    double getTotalCost() const;
    int getNumConflicts() const;
    void writeStatsToFile(const string& filename) const;
    
private:
    const Instance& instance;
    double time_limit;
    string replan_algo;
    bool use_sipp;
    
    vector<RealTimeAgent*> agents;
    PathTable path_table;
    
    // Statistics
    double total_cost;
    int total_conflicts;
    double start_time;
    
    // Replanning methods
    bool replanAgentPath(RealTimeAgent* agent);
    bool replanMultipleAgents(const vector<int>& agent_ids);
    void updatePathTable();
    
    // Conflict detection
    void detectConflicts();
    bool hasConflict(const RealTimeAgent* agent1, const RealTimeAgent* agent2) const;
    
    // Helper functions
    RealTimeAgent* getAgent(int agent_id);
    void updateAgentStatus(RealTimeAgent* agent, double current_time);
}; 