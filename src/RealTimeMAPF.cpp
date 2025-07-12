#include "RealTimeMAPF.h"
#include "SIPP.h"
#include "SpaceTimeAStar.h"
#include "CBS/ECBS.h"
#include <algorithm>
#include <chrono>

// RealTimeAgent implementation
RealTimeAgent::RealTimeAgent(const Instance& instance, int agent_id, bool use_sipp) 
    : id(agent_id), status(IDLE), current_location(instance.start_locations[agent_id]),
      current_goal(-1), next_goal(-1), path_index(0), last_update_time(0), needs_replanning(false)
{
    if (use_sipp)
        path_planner = new SIPP(instance, agent_id);
    else
        path_planner = new SpaceTimeAStar(instance, agent_id);
}

RealTimeAgent::~RealTimeAgent() {
    delete path_planner;
}

void RealTimeAgent::updatePosition(double current_time) {
    if (status != MOVING || current_path.empty())
        return;
    
    // Update position based on current time
    int time_step = (int)(current_time - last_update_time);
    if (time_step > 0) {
        path_index = min(path_index + time_step, (int)current_path.size() - 1);
        current_location = current_path[path_index].location;
        last_update_time = current_time;
        
        // Check if reached goal
        if (hasReachedGoal()) {
            status = ARRIVED;
            if (next_goal != -1) {
                // New goal was assigned while moving
                current_goal = next_goal;
                next_goal = -1;
                status = REASSIGNING;
                needs_replanning = true;
            } else {
                status = IDLE;
            }
        }
    }
}

bool RealTimeAgent::hasReachedGoal() const {
    return path_index >= (int)current_path.size() - 1 && 
           current_location == current_goal;
}

void RealTimeAgent::assignNewGoal(int new_goal) {
    if (status == IDLE) {
        current_goal = new_goal;
        status = REASSIGNING;
        needs_replanning = true;
    } else if (status == MOVING) {
        // Store new goal for when current goal is reached
        next_goal = new_goal;
    } else if (status == ARRIVED) {
        current_goal = new_goal;
        status = REASSIGNING;
        needs_replanning = true;
    }
}

void RealTimeAgent::replanPath(const PathTable& path_table) {
    if (!needs_replanning || current_goal == -1)
        return;
    
    // Create constraint table from current path table
    ConstraintTable constraint_table(instance.num_of_cols, instance.map_size, nullptr, &path_table);
    
    // Find new path
    Path new_path = path_planner->findPath(constraint_table);
    
    if (!new_path.empty()) {
        current_path = new_path;
        path_index = 0;
        current_location = current_path[0].location;
        status = MOVING;
        needs_replanning = false;
    } else {
        // Replanning failed, keep current path
        status = IDLE;
    }
}

// RealTimeMAPF implementation
RealTimeMAPF::RealTimeMAPF(const Instance& instance, double time_limit, 
                           const string& replan_algo, bool use_sipp)
    : instance(instance), time_limit(time_limit), replan_algo(replan_algo), 
      use_sipp(use_sipp), total_cost(0), total_conflicts(0)
{
    start_time = chrono::duration_cast<chrono::milliseconds>(
        chrono::high_resolution_clock::now().time_since_epoch()).count() / 1000.0;
    
    // Initialize agents
    int num_agents = instance.getDefaultNumberOfAgents();
    agents.resize(num_agents);
    for (int i = 0; i < num_agents; i++) {
        agents[i] = new RealTimeAgent(instance, i, use_sipp);
    }
}

RealTimeMAPF::~RealTimeMAPF() {
    for (auto agent : agents) {
        delete agent;
    }
}

void RealTimeMAPF::update(double current_time) {
    // Update all agent positions
    for (auto agent : agents) {
        agent->updatePosition(current_time);
    }
    
    // Handle agents that need replanning
    vector<int> agents_to_replan;
    for (auto agent : agents) {
        if (agent->needs_replanning) {
            agents_to_replan.push_back(agent->id);
        }
    }
    
    // Replan paths for agents that need it
    if (!agents_to_replan.empty()) {
        replanMultipleAgents(agents_to_replan);
    }
    
    // Update path table
    updatePathTable();
    
    // Detect and handle conflicts
    detectConflicts();
}

bool RealTimeMAPF::assignGoal(int agent_id, int new_goal) {
    RealTimeAgent* agent = getAgent(agent_id);
    if (!agent) return false;
    
    agent->assignNewGoal(new_goal);
    return true;
}

bool RealTimeMAPF::assignGoals(const vector<pair<int, int>>& agent_goal_pairs) {
    bool success = true;
    for (const auto& pair : agent_goal_pairs) {
        if (!assignGoal(pair.first, pair.second)) {
            success = false;
        }
    }
    return success;
}

void RealTimeMAPF::removeAgent(int agent_id) {
    RealTimeAgent* agent = getAgent(agent_id);
    if (agent) {
        agent->status = IDLE;
        agent->current_goal = -1;
        agent->next_goal = -1;
        agent->current_path.clear();
        agent->path_index = 0;
    }
}

void RealTimeMAPF::addAgent(int agent_id, int start_location) {
    if (agent_id >= 0 && agent_id < (int)agents.size()) {
        agents[agent_id]->current_location = start_location;
        agents[agent_id]->status = IDLE;
        agents[agent_id]->current_goal = -1;
        agents[agent_id]->next_goal = -1;
        agents[agent_id]->current_path.clear();
        agents[agent_id]->path_index = 0;
    }
}

vector<RealTimeAgent*> RealTimeMAPF::getIdleAgents() const {
    vector<RealTimeAgent*> idle_agents;
    for (auto agent : agents) {
        if (agent->isIdle()) {
            idle_agents.push_back(agent);
        }
    }
    return idle_agents;
}

vector<RealTimeAgent*> RealTimeMAPF::getMovingAgents() const {
    vector<RealTimeAgent*> moving_agents;
    for (auto agent : agents) {
        if (agent->isMoving()) {
            moving_agents.push_back(agent);
        }
    }
    return moving_agents;
}

int RealTimeMAPF::getAgentLocation(int agent_id) const {
    RealTimeAgent* agent = getAgent(agent_id);
    return agent ? agent->current_location : -1;
}

AgentStatus RealTimeMAPF::getAgentStatus(int agent_id) const {
    RealTimeAgent* agent = getAgent(agent_id);
    return agent ? agent->status : IDLE;
}

double RealTimeMAPF::getTotalCost() const {
    return total_cost;
}

int RealTimeMAPF::getNumConflicts() const {
    return total_conflicts;
}

bool RealTimeMAPF::replanAgentPath(RealTimeAgent* agent) {
    if (!agent || !agent->needs_replanning)
        return false;
    
    agent->replanPath(path_table);
    return !agent->needs_replanning;
}

bool RealTimeMAPF::replanMultipleAgents(const vector<int>& agent_ids) {
    if (agent_ids.empty()) return true;
    
    // For single agent, use simple replanning
    if (agent_ids.size() == 1) {
        RealTimeAgent* agent = getAgent(agent_ids[0]);
        return replanAgentPath(agent);
    }
    
    // For multiple agents, use the specified replanning algorithm
    if (replan_algo == "PP") {
        // Use Prioritized Planning
        vector<SingleAgentSolver*> search_engines;
        vector<Path> paths;
        
        for (int agent_id : agent_ids) {
            RealTimeAgent* agent = getAgent(agent_id);
            if (agent && agent->needs_replanning) {
                search_engines.push_back(agent->path_planner);
                paths.push_back(Path());
            }
        }
        
        // Plan paths in priority order
        for (size_t i = 0; i < search_engines.size(); i++) {
            ConstraintTable constraint_table(instance.num_of_cols, instance.map_size, nullptr, &path_table);
            paths[i] = search_engines[i]->findPath(constraint_table);
            
            if (!paths[i].empty()) {
                // Update agent path
                RealTimeAgent* agent = getAgent(agent_ids[i]);
                agent->current_path = paths[i];
                agent->path_index = 0;
                agent->current_location = paths[i][0].location;
                agent->status = MOVING;
                agent->needs_replanning = false;
                
                // Update path table
                path_table.insertPath(agent->id, paths[i]);
            }
        }
        
        return true;
    }
    else if (replan_algo == "CBS" || replan_algo == "EECBS") {
        // Use CBS/EECBS for optimal replanning
        vector<SingleAgentSolver*> search_engines;
        vector<Path> initial_paths;
        
        for (int agent_id : agent_ids) {
            RealTimeAgent* agent = getAgent(agent_id);
            if (agent && agent->needs_replanning) {
                search_engines.push_back(agent->path_planner);
                initial_paths.push_back(agent->current_path);
            }
        }
        
        // Use CBS for replanning
        CBS cbs(search_engines, 0, &path_table);
        bool success = cbs.solve(time_limit);
        
        if (success) {
            // Update agent paths
            for (size_t i = 0; i < agent_ids.size(); i++) {
                RealTimeAgent* agent = getAgent(agent_ids[i]);
                if (agent && i < cbs.paths.size()) {
                    agent->current_path = *cbs.paths[i];
                    agent->path_index = 0;
                    agent->current_location = agent->current_path[0].location;
                    agent->status = MOVING;
                    agent->needs_replanning = false;
                }
            }
        }
        
        return success;
    }
    
    return false;
}

void RealTimeMAPF::updatePathTable() {
    path_table.reset();
    for (auto agent : agents) {
        if (!agent->current_path.empty()) {
            path_table.insertPath(agent->id, agent->current_path);
        }
    }
}

void RealTimeMAPF::detectConflicts() {
    total_conflicts = 0;
    for (size_t i = 0; i < agents.size(); i++) {
        for (size_t j = i + 1; j < agents.size(); j++) {
            if (hasConflict(agents[i], agents[j])) {
                total_conflicts++;
            }
        }
    }
}

bool RealTimeMAPF::hasConflict(const RealTimeAgent* agent1, const RealTimeAgent* agent2) const {
    if (!agent1 || !agent2 || agent1->current_path.empty() || agent2->current_path.empty())
        return false;
    
    // Check for vertex conflicts
    for (size_t t = 0; t < min(agent1->current_path.size(), agent2->current_path.size()); t++) {
        if (agent1->current_path[t].location == agent2->current_path[t].location) {
            return true;
        }
    }
    
    // Check for edge conflicts
    for (size_t t = 0; t < min(agent1->current_path.size() - 1, agent2->current_path.size() - 1); t++) {
        if (agent1->current_path[t].location == agent2->current_path[t + 1].location &&
            agent1->current_path[t + 1].location == agent2->current_path[t].location) {
            return true;
        }
    }
    
    return false;
}

RealTimeAgent* RealTimeMAPF::getAgent(int agent_id) {
    if (agent_id >= 0 && agent_id < (int)agents.size()) {
        return agents[agent_id];
    }
    return nullptr;
}

void RealTimeMAPF::writeStatsToFile(const string& filename) const {
    ofstream file(filename);
    if (file.is_open()) {
        file << "Total Cost: " << total_cost << endl;
        file << "Total Conflicts: " << total_conflicts << endl;
        file << "Number of Agents: " << agents.size() << endl;
        file << "Replanning Algorithm: " << replan_algo << endl;
        file.close();
    }
} 