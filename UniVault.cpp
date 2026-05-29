// UniVault — Record Management and Simulation System Project 
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>

using namespace std;

// Function to get string's length
int getStrLen(const char* str) 
{
    int l = 0;
    while (str[l] != '\0')
    {
        l++;
    }
    return l;
}

// Manually copies content from a source string to a destination buffer
void strCopy(char* dest, const char* src) 
{
    int i = 0;
    while (src[i] != '\0') 
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0'; // Ensures the new string is properly terminated
}

// File-friendly conversion: Swaps spaces with underscores
// This prevents file reading issues where spaces are treated as delimiters
void nameToToken(char* dest, const char* src) 
{
    int len = getStrLen(src);
    for (int i = 0; i <= len; i++) 
    {
        if(src[i] == ' ')
        {
            dest[i] = '_';
        }
        else
        {
            dest[i] = src[i];
        }
    }
}

// Reverses the tokenization: Swaps underscores back to spaces for UI display
void tokenToName(char* dest, const char* src) 
{
    int len = getStrLen(src);
    for (int i = 0; i <= len; i++) 
    {
        if(src[i] == '_')
        {
            dest[i] = ' ';
        }
        else
        {
            dest[i] = src[i];
        }
    }
}

// Handles the setup of campus dimensions (Depts, Floors, Rooms)
// It tries to read from a file first; if missing, it prompts the user to define the campus size
void readConfig(int &numDepts, int &numFloors, int &numRooms) 
{
    ifstream fin("config.txt");
    if (fin.is_open()) 
    {
        fin >> numDepts >> numFloors >> numRooms;
        fin.close();
        cout << "Config loaded: " << numDepts << " dept(s), "<< numFloors << " floor(s), " << numRooms << " room(s).\n";
    } 
    else 
    {
        cout << "config.txt not found. Enter values manually:\n";
        cout << "Number of Departments  : "; 
        cin >> numDepts;
        cout << "Number of Floors  : "; 
        cin >> numFloors;
        cout << "Number of Rooms/Floor    : "; 
        cin >> numRooms;

        cin.ignore(1000, '\n'); // Clean up the input buffer

        ofstream fout("config.txt");
        fout << numDepts  << "\n"<< numFloors << "\n"<< numRooms  << "\n";
        fout.close();
        cout << "config.txt created.\n";
    }

    // Safety check: ensure the campus has at least 1 of everything
    if (numDepts  < 1) 
    {
        numDepts  = 1;
    }

    if (numFloors < 1) 
    {
        numFloors = 1;
    }
    
    if (numRooms  < 1) 
    {
        numRooms  = 1;
    }
}

// Dynamically allocates a 3D array (Grid) representing the physical campus
// Uses pointer-to-pointer-to-pointer logic to map [Dept][Floor][Room]
int*** initCampus(int numDepts, int numFloors, int numRooms) 
{
    int*** CampusGrid = new int**[numDepts];
    for (int i = 0; i < numDepts; i++) 
    {
        *(CampusGrid + i) = new int*[numFloors];
        for (int j = 0; j < numFloors; j++) 
        {
            *(*(CampusGrid + i) + j) = new int[numRooms];
            for (int k = 0; k < numRooms; k++) 
            {
                // Initialize all rooms as 0 (indicates vacant/empty)
                *(*(*(CampusGrid + i) + j) + k) = 0;
            }
        }
    }
    return CampusGrid;
}

// Carefully deallocates the 3D grid from the heap to prevent memory leaks
void freeCampus(int*** grid, int nd, int nf) 
{
    for (int d = 0; d < nd; d++) 
    {
        for (int f = 0; f < nf; f++)
        {
            delete[] *(*(grid + d) + f); // Delete rooms
        }
        delete[] *(grid + d);           // Delete floors
    }
    delete[] grid;                      // Delete departments
}

// Adds a new student to the system and assigns them a physical room
// Manages dynamic array resizing (growing arrays by 1 for each new student)
void enrollStudent(int*** grid, char** &names, int* &ids, float* &gpas, int* &statuses,int &numStudents, int &nextID, int nd, int nf, int nr)
{
    int cap = nd * nf * nr;
    if (numStudents >= cap) 
    {
        cout << "Campus is full! Cannot enrol student.\n";
        return;
    }

    char tempName[200];
    cout << "Enter student name: ";
    cin.getline(tempName, 200);

    if (getStrLen(tempName) == 0) 
    {
        cout << "Name cannot be empty. Enrolment cancelled.\n";
        return;
    }

    // Try to assign a random room first for variety
    int aD = rand() % nd;
    int aF = rand() % nf;
    int aR = rand() % nr;
    int sk = 0;

    // If the random room is taken, perform a linear search for the first available spot
    if (*(*(*(grid + aD) + aF) + aR) != 0) 
    {
        bool found = false;
        for (int d = 0; d < nd && !found; d++)
        {
            for (int f = 0; f < nf && !found; f++)
            {
                for (int r = 0; r < nr && !found; r++) 
                {
                    if (*(*(*(grid + d) + f) + r) == 0) 
                    {
                        aD = d; aF = f; aR = r;
                        found = true;
                    } 
                    else 
                    {
                        sk++;
                    }
                }
            }
        }

        if (!found) 
        {
            cout << "Campus is full! Cannot enrol student.\n";
            return;
        }
        cout << "Original room was occupied. Skipped " << sk << " slot(s).\n";
    }

    // GROWING THE ARRAYS: Create new temporary arrays with size + 1
    int ns = numStudents;
    char** n2 = new char*[ns + 1];
    int* i2 = new int  [ns + 1];
    float* g2 = new float[ns + 1];
    int* s2 = new int  [ns + 1];

    // Transfer existing student data to the new larger arrays
    for (int k = 0; k < ns; k++) 
    {
        n2[k] = names[k];
        i2[k] = ids[k];
        g2[k] = gpas[k];
        s2[k] = statuses[k];
    }

    // Initialize the new student's data
    n2[ns] = new char[getStrLen(tempName) + 1];
    strCopy(n2[ns], tempName);
    i2[ns] = nextID++;
    g2[ns] = 0.0f;
    s2[ns] = 0; // Status: 0 = STUDYING

    // Clean up old small arrays and repoint to new large ones
    delete[] names;
    delete[] ids;
    delete[] gpas;
    delete[] statuses;

    names    = n2;
    ids      = i2;
    gpas     = g2;
    statuses = s2;
    numStudents = ns + 1;

    // Link the Student ID to the 3D Campus Grid
    *(*(*(grid + aD) + aF) + aR) = ids[ns];

    cout << "Enrolled!  ID=" << ids[ns]<< "  Room -> Dept " << aD + 1<< "  Floor " << aF + 1<< "  Room "  << aR + 1 << "\n";
}

// Simulates time passing. Students' GPAs change based on random probability.
// Uses Bitwise operations to track events (Progress, Warning, Graduation)
void runSimulation(int*** grid, char** &names, int* &ids, float* &gpas, int* &statuses,int &numStudents, int nd, int nf, int nr, int &step) 
{
    step++;
    for (int i = 0; i < numStudents; i++) 
    {

        int val = (rand() % 10) + 1;
        unsigned char fl = 0; // bitmask: bit0=progress, bit1=warning, bit2=graduated

        if (val <= 5) 
        {
            gpas[i] += 0.1f;
            fl = fl | 1; // Setting bit 0
        }
        else if (val >= 6 && val <= 8) 
        {
            // No change (Steady state)
        }
        else if (val == 9) 
        {
            statuses[i] = 1; // Set status to WARNING
            gpas[i] -= 0.2f;
            fl = fl | 2; // Setting bit 1
        }
        else if (val == 10) 
        {
            statuses[i] = 2; // Set status to GRADUATING
            fl = fl | 4; // Setting bit 2
        }

        // Keep GPA within realistic bounds (0.0 - 4.0)
        if (gpas[i] > 4.0f) 
        {
            gpas[i] = 4.0f;
        }
        if (gpas[i] < 0.0f) 
        {
            gpas[i] = 0.0f;
        }

        // Handling Graduation: Removal logic
        if (fl & 4) 
        {
            // First, find and empty their room in the 3D grid
            bool found = false;
            for (int d = 0; d < nd && !found; d++)
            {
                for (int f = 0; f < nf && !found; f++)
                {
                    for (int r = 0; r < nr && !found; r++)
                    {
                        if (*(*(*(grid + d) + f) + r) == ids[i]) 
                        {
                            *(*(*(grid + d) + f) + r) = 0;
                            found = true;
                        }
                    }
                }
            }

            cout << "  GRADUATED: " << names[i] << " (ID " << ids[i] << ")\n";

            delete[] names[i]; // Free string memory for that student

            // Shift array elements left to overwrite the graduated student
            for (int j = i; j < numStudents - 1; j++) 
            {
                names[j]    = names[j + 1];
                ids[j]      = ids[j + 1];
                gpas[j]     = gpas[j + 1];
                statuses[j] = statuses[j + 1];
            }

            // SHRINKING THE ARRAYS: Reallocate memory to a smaller size
            int nSz = numStudents - 1;
            if (nSz > 0) 
            {
                char** n2 = new char*[nSz];
                int* i2 = new int[nSz];
                float* g2 = new float[nSz];
                int* s2 = new int[nSz];

                for (int k = 0; k < nSz; k++) 
                {
                    n2[k] = names[k];
                    i2[k] = ids[k];
                    g2[k] = gpas[k];
                    s2[k] = statuses[k];
                }

                delete[] names; 
                delete[] ids; 
                delete[] gpas; 
                delete[] statuses;
                names = n2; ids = i2; gpas = g2; statuses = s2;
            } 
            else 
            {
                // If last student leaves, reset to a 1-size empty pointer
                delete[] names; 
                delete[] ids; 
                delete[] gpas; 
                delete[] statuses;
                names    = new char*[1];
                ids      = new int[1];
                gpas     = new float[1];
                statuses = new int[1];
            }

            numStudents--;
            i--; // Adjust index because the next student shifted into 'i'
            continue;
        }
    }
}

// Renders the current state of a specific floor to the console
// Shows physical memory addresses of rooms alongside student data
void displayDashboard(int*** grid, char** names, int* ids, float* gpas, int* statuses, int numStudents,int nd, int nf, int nr, int* currentDept, int* currentFloor, int step)
{
#ifdef _WIN32
    system("cls"); // Windows clear
#else
    system("clear"); // Linux/Mac clear
#endif

    // Clamp views to ensure we don't look outside the campus array bounds
    if (*currentDept  < 0 || *currentDept  >= nd) 
    {
        *currentDept  = 0;
    }
    if (*currentFloor < 0 || *currentFloor >= nf) 
    {
        *currentFloor = 0;
    }

    int d = *currentDept;
    int f = *currentFloor;

    cout << right;
    cout << "===========================================================================\n";
    cout << "            UniVault  .  LIVE RESOURCE DASHBOARD\n";
    cout << "===========================================================================\n";
    cout << "[DEPT: "  << setw(2) << setfill('0') << d + 1
         << "]     [FLOOR: " << setw(2) << setfill('0') << f + 1
         << "]     [ROOMS: " << setw(2) << setfill('0') << nr
         << "]     [STEP: "  << setw(3) << setfill('0') << step << "]\n";
    cout << setfill(' ');
    cout << "---------------------------------------------------------------------------\n";
    cout << left << setw(16) << "ROOM ADDR" << setw(8) << "ID" << setw(22) << "NAME" << setw(7) << "GPA" << "STATUS\n";
    cout << "---------------------------------------------------------------------------\n";

    for (int r = 0; r < nr; r++) 
    {
        // Pointer arithmetic to access the room's occupant ID
        int* rp = *(*(grid + d) + f) + r;
        int  occ = *rp;

        // Display the hex memory address of the room pointer
        cout << left << setw(16) << (void*)rp;

        if (occ == 0) 
        {
            cout << setw(8) << "----" << setw(22) << "----------" << setw(7) << "0.00" << "EMPTY\n";
        } 
        else 
        {
            // Find student details by matching the ID found in the grid
            int idx = -1;
            for (int s = 0; s < numStudents; s++)
                if (ids[s] == occ) { idx = s; break; }

            if (idx != -1) 
            {
                cout << setw(8) << ids[idx] << setw(22) << names[idx]
                     << setw(7) << fixed << setprecision(2) << gpas[idx];
                if      (statuses[idx] == 0) cout << "STUDYING\n";
                else if (statuses[idx] == 1) cout << "WARNING\n";
                else                          cout << "GRADUATING\n";
            } 
            else 
            {
                // Edge case: student ID in grid but not in array (during deletion)
                cout << setw(8) << occ << setw(22) << "(leaving)" << setw(7) << "0.00" << "GONE\n";
            }
        }
    }
    cout << "---------------------------------------------------------------------------\n";
    cout << "COMMANDS: [ENTER] Step | [E] Enroll | [J] Jump | [F] Find | [S] Save | [X] Exit\n";
    cout << "===========================================================================\n";
}

// Locates a student by ID and prints their full profile + physical location
void findStudent(int*** grid, char** names, int* ids, float* gpas, int* statuses,int numStudents, int nd, int nf, int nr) 
{
    int sID;
    cout << "Enter Student ID to find: ";

    if (!(cin >> sID)) 
    {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input. Please enter a number.\n";
        return;
    }

    char dummy;
    while (cin.get(dummy) && dummy != '\n'); // Clear buffer

    int idx = -1;
    for (int i = 0; i < numStudents; i++)
    {
        if (ids[i] == sID) 
        { 
            idx = i; 
            break; 
        }
    }
    if (idx == -1) 
    {
        cout << "NOT FOUND: No student with ID " << sID << ".\n";
        return;
    }

    // Scan grid to find where this student is physically located
    int fD = -1, fF = -1, fR = -1;
    for (int d = 0; d < nd; d++)
    {
        for (int f = 0; f < nf; f++)
        {
            for (int r = 0; r < nr; r++)
            {
                if (*(*(*(grid + d) + f) + r) == sID)
                { 
                    fD = d; 
                    fF = f; 
                    fR = r; 
                }
            }
        }
    }

    cout << "\n--- Student Record ---\n";
    cout << "ID     : " << ids[idx] << "\n";
    cout << "Name   : " << names[idx] << "\n";
    cout << "GPA    : " << fixed << setprecision(2) << gpas[idx] << "\n";
    cout << "Status : ";
    if      (statuses[idx] == 0) 
    {
        cout << "STUDYING\n";
    }
    else if (statuses[idx] == 1) 
    {
        cout << "WARNING\n";
    }
    else                          
    {
        cout << "GRADUATING\n";
    }
    if (fD != -1)
    {
        cout << "Room   : Dept " << fD + 1 << "  Floor " << fF + 1 << "  Room " << fR + 1 << "\n";
    }
    cout << "----------------------\n";
}

// Allows the user to switch the dashboard view to a different Dept/Floor
void jumpView(int* currentDept, int* currentFloor, int nd, int nf) 
{
    int tmpD, tmpF;

    cout << "Enter Department index (0 to " << (nd - 1) << "): ";
    if (!(cin >> tmpD)) 
    {
        cin.clear(); 
        cin.ignore(1000, '\n');
        cout << "Invalid input.\n"; 
        return;
    }

    cout << "Enter Floor index     (0 to " << (nf - 1) << "): ";
    if (!(cin >> tmpF)) 
    {
        cin.clear(); 
        cin.ignore(1000, '\n');
        cout << "Invalid input.\n"; 
        return;
    }

    char dummy;
    while (cin.get(dummy) && dummy != '\n');

    if (tmpD < 0 || tmpD >= nd) 
    {
        tmpD = 0;
    }
    if (tmpF < 0 || tmpF >= nf) 
    {
        tmpF = 0;
    }

    *currentDept  = tmpD;
    *currentFloor = tmpF;

    cout << "View -> Dept " << *currentDept << "  Floor " << *currentFloor << "\n";
}

// Serializes all student data and grid occupancy into database.txt
void saveDatabase(int*** grid, char** names, int* ids, float* gpas, int* statuses,int numStudents, int nd, int nf, int nr) {
    ofstream out("database.txt");
    if (!out) 
    {
        cout << "Error Occured : File named as database.txt does not exists.\n";
        return;
    }

    out << numStudents << endl; // Total count for easy loading

    char token[200];
    for (int i = 0; i < numStudents; i++) 
    {
        int dP = -1, fP = -1, rP = -1;
        // Search grid for student location to save coordinates
        for (int d = 0; d < nd; d++)
        {
            for (int f = 0; f < nf; f++)
            {
                for (int r = 0; r < nr; r++)
                {
                    if (*(*(*(grid + d) + f) + r) == ids[i])
                    { 
                        dP = d; 
                        fP = f; 
                        rP = r; 
                    }
                }
            }
        }

        nameToToken(token, names[i]);
        out << ids[i] << " " << token << " " << gpas[i] << " "<< statuses[i] << " " << dP << " " << fP << " " << rP << endl;
    }

    // Additional block to save the raw occupancy of the grid
    for (int d = 0; d < nd; d++)
    {
        for (int f = 0; f < nf; f++)
        {
            for (int r = 0; r < nr; r++) 
            {
                int occ = *(*(*(grid + d) + f) + r);
                if (occ != 0)
                    out << d << " " << f << " " << r << " " << occ << endl;
            }
        }
    }

    out.close();
    cout << "Saved.\n";
}

// Reconstructs the system state from database.txt during startup
void loadDatabase(int*** grid, char** &names, int* &ids, float* &gpas,int* &statuses, int &numStudents, int nd, int nf, int nr) 
{
    ifstream in("database.txt");
    if (!in) 
    {
        // First-time run: Initialize empty parallel arrays
        names    = new char*[1];
        ids      = new int[1];
        gpas     = new float[1];
        statuses = new int[1];
        numStudents = 0;
        return;
    }

    in >> numStudents;
    int aSz = (numStudents > 0) ? numStudents : 1;

    names    = new char*[aSz];
    ids      = new int[aSz];
    gpas     = new float[aSz];
    statuses = new int[aSz];

    for (int i = 0; i < numStudents; i++) 
    {
        char buffer[200];
        int d, f, r;
        in >> ids[i] >> buffer >> gpas[i] >> statuses[i] >> d >> f >> r;

        char realName[200];
        tokenToName(realName, buffer);

        int len = getStrLen(realName);
        names[i] = new char[len + 1];
        for (int j = 0; j <= len; j++)
        {
            *(names[i] + j) = realName[j];
        }

        // Restore the student to their saved room position
        if (d != -1 && d < nd && f < nf && r < nr)
        {
            *(*(*(grid + d) + f) + r) = ids[i];
        }
    }

    // Read remaining lines to ensure grid synchronization
    int gd, gf, gr, gOcc;
    while (in >> gd >> gf >> gr >> gOcc) 
    {
        if (gd >= 0 && gd < nd && gf >= 0 && gf < nf && gr >= 0 && gr < nr)
        {
            *(*(*(grid + gd) + gf) + gr) = gOcc;
        }
    }

    in.close();
    cout << "Loaded " << numStudents << " student(s) from database.txt.\n";
}

int main() {
    srand((unsigned int)time(0)); // Seed random for simulation outcomes

    int numDepts = 0, numFloors = 0, numRooms = 0;
    readConfig(numDepts, numFloors, numRooms);
    
    // Setup Campus
    int*** CampusGrid = initCampus(numDepts, numFloors, numRooms);
    
    // Student Data Arrays
    char** names    = nullptr;
    int* ids      = nullptr;
    float* gpas     = nullptr;
    int* statuses = nullptr;
    int    numStudents = 0;
    int    nextID      = 1001;

    // Restore previous state
    loadDatabase(CampusGrid, names, ids, gpas, statuses,numStudents, numDepts, numFloors, numRooms);
    
    // Auto-increment ID based on existing records
    if (numStudents > 0) 
    {
        int maxID = ids[0];
        for (int i = 1; i < numStudents; i++)
        {
            if (ids[i] > maxID) 
            {
                maxID = ids[i];
            }
        }
        nextID = maxID + 1;
    }

    int currentDept  = 0;
    int currentFloor = 0;
    int step         = 0;

    // Initial Screen
    displayDashboard(CampusGrid, names, ids, gpas, statuses, numStudents, numDepts, numFloors, numRooms, &currentDept, &currentFloor, step);

    bool running = true;
    while (running) 
    {
        cout << "\nCommand > ";
        char line[20];
        cin.getline(line, 20);

        if (cin.fail()) 
        {
            cin.clear(); 
            running = false; 
            continue;
        }

        if (getStrLen(line) == 0) 
        {
            // No input + Enter = Proceed with academic simulation
            runSimulation(CampusGrid, names, ids, gpas, statuses, numStudents, numDepts, numFloors, numRooms, step);
            displayDashboard(CampusGrid, names, ids, gpas, statuses, numStudents, numDepts, numFloors, numRooms, &currentDept, &currentFloor, step);
        } 
        else 
        {
            char ch = line[0];
            if (ch == 'E' || ch == 'e') 
            {
                enrollStudent(CampusGrid, names, ids, gpas, statuses, numStudents, nextID, numDepts, numFloors, numRooms);
                displayDashboard(CampusGrid, names, ids, gpas, statuses, numStudents, numDepts, numFloors, numRooms, &currentDept, &currentFloor, step);
            } 
            else if (ch == 'J' || ch == 'j') 
            {
                jumpView(&currentDept, &currentFloor, numDepts, numFloors);
                displayDashboard(CampusGrid, names, ids, gpas, statuses, numStudents, numDepts, numFloors, numRooms, &currentDept, &currentFloor, step);
            } 
            else if (ch == 'F' || ch == 'f') 
            {
                findStudent(CampusGrid, names, ids, gpas, statuses, numStudents, numDepts, numFloors, numRooms);
            } 
            else if (ch == 'S' || ch == 's') 
            {
                saveDatabase(CampusGrid, names, ids, gpas, statuses, numStudents, numDepts, numFloors, numRooms);
            } 
            else if (ch == 'X' || ch == 'x') 
            {
                saveDatabase(CampusGrid, names, ids, gpas, statuses, numStudents, numDepts, numFloors, numRooms);
                running = false;
            } 
            else 
            {
                cout << "Invalid Command : Type (X,S,F,J,E) \n";
            }
        }
    }
    
    // FINAL CLEANUP: Releasing all heap memory before exit
    for (int i = 0; i < numStudents; i++)
    {
        delete[] names[i];
    }
    delete[] names;
    delete[] ids;
    delete[] gpas;
    delete[] statuses;
    freeCampus(CampusGrid, numDepts, numFloors);
    cout << "Uni-Vault Is Closed.\n";
}
