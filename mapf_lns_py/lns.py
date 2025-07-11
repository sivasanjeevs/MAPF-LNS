import time
import random
from mapf_lns_py.agent import Agent
from mapf_lns_py.pathfinding import prioritized_planning

class LNS:
    def __init__(self, instance, time_limit=60, neighbor_size=8, max_iterations=10, screen=1):
        self.instance = instance
        self.time_limit = time_limit
        self.neighbor_size = neighbor_size
        self.max_iterations = max_iterations
        self.screen = screen
        self.agents = [Agent(i, a['start'], a['goal']) for i, a in enumerate(instance.agents)]
        self.paths = []
        self.stats = []
        self.initial_solution_cost = None
        self.initial_solution_runtime = None
        self.failed_iterations = 0

    def get_solution_cost(self):
        return sum(len(agent.path)-1 for agent in self.agents)

    def get_initial_solution(self):
        start = time.time()
        result = prioritized_planning(self.instance, self.agents)
        self.initial_solution_runtime = time.time() - start
        if result:
            self.initial_solution_cost = self.get_solution_cost()
        return result

    def destroy(self):
        # Randomly select a neighborhood of agents
        return random.sample(self.agents, min(self.neighbor_size, len(self.agents)))

    def repair(self, neighborhood):
        # Remove paths for neighborhood agents
        for agent in neighborhood:
            agent.path = []
        # Replan for neighborhood in order
        occupied = dict()
        for agent in self.agents:
            if agent not in neighborhood:
                for t, pos in enumerate(agent.path):
                    occupied[(pos[0], pos[1], t)] = True
                for t in range(len(agent.path), len(agent.path)+20):
                    occupied[(agent.path[-1][0], agent.path[-1][1], t)] = True
        for agent in neighborhood:
            from mapf_lns_py.pathfinding import space_time_astar
            path = space_time_astar(self.instance.grid, agent.start, agent.goal, occupied)
            if not path:
                return False
            for t, pos in enumerate(path):
                occupied[(pos[0], pos[1], t)] = True
            for t in range(len(path), len(path)+20):
                occupied[(path[-1][0], path[-1][1], t)] = True
            agent.path = path
        return True

    def run(self):
        start_time = time.time()
        if not self.get_initial_solution():
            print("Failed to find initial solution")
            return False
        if self.screen:
            print(f"Initial solution cost = {self.initial_solution_cost}, runtime = {self.initial_solution_runtime:.3f}s")
        for it in range(self.max_iterations):
            if time.time() - start_time > self.time_limit:
                break
            neighborhood = self.destroy()
            old_paths = {agent.id: list(agent.path) for agent in neighborhood}
            old_cost = self.get_solution_cost()
            success = self.repair(neighborhood)
            new_cost = self.get_solution_cost() if success else old_cost
            if not success or new_cost >= old_cost:
                # Restore old paths
                for agent in neighborhood:
                    agent.path = old_paths[agent.id]
                self.failed_iterations += 1
            if self.screen:
                print(f"Iteration {it+1}: cost = {self.get_solution_cost()}, failed = {self.failed_iterations}")
            self.stats.append({
                'iteration': it+1,
                'solution_cost': self.get_solution_cost(),
                'failed_iterations': self.failed_iterations,
                'runtime': time.time() - start_time
            })
        return True

    def write_paths(self, filename):
        with open(filename, 'w') as f:
            for agent in self.agents:
                f.write(f"{agent.id}: {agent.path}\n")

    def write_stats(self, filename):
        import csv
        with open(filename, 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=['iteration','solution_cost','failed_iterations','runtime'])
            writer.writeheader()
            for row in self.stats:
                writer.writerow(row) 