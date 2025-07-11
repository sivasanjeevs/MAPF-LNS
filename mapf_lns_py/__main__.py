import argparse
from mapf_lns_py.instance import Instance
from mapf_lns_py.lns import LNS

def main():
    parser = argparse.ArgumentParser(description='MAPF-LNS2 (LNS(PP;PP) Python)')
    parser.add_argument('--map', required=True, help='Input map file')
    parser.add_argument('--agents', required=True, help='Input scenario file')
    parser.add_argument('--agentNum', type=int, default=0, help='Number of agents (0=all)')
    parser.add_argument('--neighborSize', type=int, default=8, help='Neighborhood size')
    parser.add_argument('--maxIterations', type=int, default=10, help='Maximum LNS iterations')
    parser.add_argument('--cutoffTime', type=float, default=60, help='Time limit (seconds)')
    parser.add_argument('--outputPaths', help='Output file for paths')
    parser.add_argument('--stats', help='Output file for stats')
    parser.add_argument('--screen', type=int, default=1, help='Screen verbosity (0=none, 1=summary)')
    args = parser.parse_args()

    instance = Instance(args.map, args.agents, args.agentNum if args.agentNum > 0 else None)
    lns = LNS(instance, time_limit=args.cutoffTime, neighbor_size=args.neighborSize, max_iterations=args.maxIterations, screen=args.screen)
    lns.run()
    if args.outputPaths:
        lns.write_paths(args.outputPaths)
    if args.stats:
        lns.write_stats(args.stats)

if __name__ == '__main__':
    main() 