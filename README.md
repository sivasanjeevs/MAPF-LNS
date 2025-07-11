# MAPF-LNS2 (LNS(PP;PP) Only)

**Note:** A full-featured Python version is being created in the `mapf_lns_py/` directory. See that folder for the new implementation.

This project is now restricted to only support the LNS(PP;PP) case for Multi-Agent Path Finding (MAPF).

## Usage

Run the solver with:

```
./lns --map <mapfile> --agents <scenfile> --agentNum <num> --solver LNS --initAlgo PP --replanAlgo PP --destoryStrategy Adaptive --initDestoryStrategy Adaptive
```

All other algorithms and combinations have been removed from this codebase.

## Maintainers
This version is a minimal, single-case fork of the original MAPF-LNS2 project.
