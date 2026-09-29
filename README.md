UniVault is a console-based record management and simulation system written in C++. The entire system is built on raw heap memory and pointer arithmetic with no STL containers, no classes, and no structs anywhere in the codebase.

The campus is modeled as a 3D grid allocated entirely on the heap. Every room is accessed through pointer arithmetic only. Grid dimensions are pulled from a config file on startup so nothing is hardcoded.

Records are stored in four parallel dynamic arrays for names, IDs, GPAs, and statuses. Every time a record is added or removed, the arrays grow or shrink manually through a copy-and-grow approach using new and delete. No realloc anywhere.

The simulation engine runs on each step and applies a random outcome to every record in the system. Events are tracked using bitwise flags. When a record graduates, its memory is freed, the arrays compact by shifting indexes, and the grid slot is cleared through a full pointer arithmetic scan.

The live dashboard clears and redraws on every operation showing actual heap memory addresses of room pointers alongside record data, GPA values, and statuses in a formatted console layout.

All state saves to disk and reloads automatically on the next startup. Names are tokenized for file-safe storage and converted back on load.

Technical boundaries enforced throughout: no STL, no classes, no structs, no global variables, no bracket indexing on the grid, no fixed-size name buffers, no realloc, and full heap cleanup before exit.

Stack: C++, manual heap memory, pointer arithmetic, file I/O, bitwise operations

To build and run:
g++ -o univault main.cpp
./univault

<!-- structure:start -->
## Project structure

**Stack:** C++

```
.gitignore
config.txt
database.txt
README.md
UniVault.cpp
```
<!-- structure:end -->

