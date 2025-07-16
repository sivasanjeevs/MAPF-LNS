#pragma once
#include "common.h"

#define NO_AGENT -1

class PathTable
{
public:
    int makespan = 0;
    // Now stores [location][orientation][timestep] = agent_id
    vector< vector< vector<int> > > table; // [location][orientation][timestep]
    vector<int> goals; // this stores the goal locatons of the paths: key is the location, while value is the timestep when the agent reaches the goal
    void reset() { auto map_size = table.size(); table.clear(); table.resize(map_size); goals.assign(map_size, MAX_COST); makespan = 0; }
    void insertPath(int agent_id, const Path& path);
    void deletePath(int agent_id, const Path& path);
    // Now checks for conflicts at (location, orientation, timestep)
    bool constrained(int from, int from_ori, int to, int to_ori, int to_time) const;
    void get_agents(set<int>& conflicting_agents, int loc, int ori) const;
    void getConflictingAgents(int agent_id, set<int>& conflicting_agents, int from, int from_ori, int to, int to_ori, int to_time) const;
    int getHoldingTime(int location, int orientation, int earliest_timestep) const;
    explicit PathTable(int map_size = 0) : table(map_size, vector<vector<int>>(4)), goals(map_size, MAX_COST) {}
};

class PathTableWC // with collisions
{
public:
    int makespan = 0;
    // Now stores [location][orientation][timestep] = list of agent_ids
    vector< vector< vector< list<int> > > > table; // [location][orientation][timestep]
    vector<int> goals; // this stores the goal locatons of the paths: key is the location, while value is the timestep when the agent reaches the goal
    void reset() { auto map_size = table.size(); table.clear(); table.resize(map_size, vector<vector<list<int>>>(4)); goals.assign(map_size, MAX_COST); makespan = 0; }
    void insertPath(int agent_id, const Path& path);
    void insertPath(int agent_id);
    void deletePath(int agent_id);
    const Path* getPath(int agent_id) const {return paths[agent_id]; }
    int getFutureNumOfCollisions(int loc, int ori, int time) const;
    int getNumOfCollisions(int from, int from_ori, int to, int to_ori, int to_time) const;
    bool hasCollisions(int from, int from_ori, int to, int to_ori, int to_time) const;
    bool hasEdgeCollisions(int from, int from_ori, int to, int to_ori, int to_time) const;
    int getLastCollisionTimestep(int location, int orientation) const;
    int getAgentWithTarget(int target_location, int target_orientation, int latest_timestep) const;
    void clear();
    explicit PathTableWC(int map_size = 0, int num_of_agents = 0) : table(map_size, vector<vector<list<int>>>(4)), goals(map_size, MAX_COST),
        paths(num_of_agents, nullptr) {}
private:
    vector<const Path*> paths;
};