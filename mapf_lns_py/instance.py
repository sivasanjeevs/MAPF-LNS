import os

class Instance:
    def __init__(self, map_file, scen_file, agent_num=None):
        self.grid = self._load_map(map_file)
        self.agents = self._load_scen(scen_file, agent_num)
        self.height = len(self.grid)
        self.width = len(self.grid[0]) if self.grid else 0

    def _load_map(self, map_file):
        grid = []
        with open(map_file, 'r') as f:
            lines = f.readlines()
            for line in lines:
                line = line.strip()
                if line and not line.startswith('type') and not line.startswith('height') and not line.startswith('width') and not line.startswith('map'):
                    grid.append([c != '@' and c != 'T' for c in line])
        return grid

    def _load_scen(self, scen_file, agent_num):
        agents = []
        with open(scen_file, 'r') as f:
            lines = f.readlines()
            for line in lines:
                if line.startswith('version'):
                    continue
                parts = line.strip().split()
                if len(parts) < 8:
                    continue
                start = (int(parts[4]), int(parts[5]))
                goal = (int(parts[6]), int(parts[7]))
                agents.append({'start': start, 'goal': goal})
                if agent_num and len(agents) >= agent_num:
                    break
        return agents 