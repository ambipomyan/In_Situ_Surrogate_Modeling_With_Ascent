# build Ascent
- Following the instruction in this page: https://ascent.readthedocs.io/en/latest/BuildingAscent.html to build ascent
- Dependences are seen in `/source`

# build examples
## /src/examples/proxies/lulesh2.0.3
- Modify `CMakeLists.txt` include dependences
- Insert mesh definitions, `lulesh-init.cc`, line 196 - 265
- Write `ascent_actions.yaml`
- Insert Ascent integration, `lulesh.cc`, line 2857 - 2930
- Implement python scripts, `/build/auto_io.py`, `/build/auto_mem.py`
- Following instructions in `README` file to build lulesh2.0.3

## /src/examples/proxies/laghos
- Modify `CMakeLists.txt` include dependences
- Insert mesh definitions, `laghos.cpp`, line 535 - 579
- Write `ascent_actions.yaml`
- Insert Ascent integration, `laghos.cpp`, line 581 - 591
- Implement python scripts, `/build/auto_io.py`, `/build/auto_mem.py`
- Following instructions in `README` file to build laghos, with dependences, including
```
mfem
hypre
GKlib
METIS
```

## /src/examples/proxies/kripke
- Modify `CMakeLists.txt` include dependences
- Insert mesh definitions, `/Kripke/Sweep_Solver.cpp`, line 42 - 83
- Write `ascent_actions.yaml`
- Insert Ascent integration, /Kripke/Sweep_Solver.cpp, line 104 - 117
- Implement python scripts, `/build/auto_io.py`, `/build/auto_mem.py`
- Following instructions in `README` file to build Kripke
