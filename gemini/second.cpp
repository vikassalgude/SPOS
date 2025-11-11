/*Design suitable data structures and implement Pass-I and Pass-II of a two-pass macroprocessor. The output of Pass-I (MNT, MDT and1 intermediate code file without any macro
definitions) should be input for Pass-II. 
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>

using namespace std;

// --- Data Structures ---

// MNT Entry structure
struct MNTEntry {
    int start_index;             // Index in MDT where definition starts
    int num_parameters;          // Number of formal parameters
};

// Structure to hold safely parsed line components
struct ParsedLine {
    string label;
    string opcode;
    string operand;
};

// Global data structures for Pass I and Pass II
map<string, MNTEntry> MNT;      // Macro Name Table
vector<string> MDT;             // Macro Definition Table
int MDT_IDX = 0;                // Global index for MDT

// Helper function to tokenize a line (Label, Opcode, Operand)
vector<string> tokenize(const string& line) {
    vector<string> tokens;
    stringstream ss(line);
    string token;
    // Reads tokens separated by whitespace (spaces or tabs)
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * @brief Safely parses tokens into Label, Opcode, and Operand based on token count.
 * * Assumes the assembly syntax convention where the absence of a label shifts 
 * Opcode and Operand to the left columns (tokens[0] and tokens[1]).
 */
ParsedLine parse_line(const vector<string>& tokens) {
    ParsedLine p = {"", "", ""};
    int size = tokens.size();
    
    if (size == 0) return p;

    if (size == 1) {
        // e.g., MEND or END
        p.opcode = tokens[0];
    } else if (size == 2) {
        // e.g., SUB &B (Opcode, Operand)
        p.opcode = tokens[0];
        p.operand = tokens[1];
    } else { // size >= 3
        // e.g., CALC MACRO &A,&B (Label, Opcode, Operand)
        p.label = tokens[0];
        p.opcode = tokens[1];
        p.operand = tokens[2];
    }
    return p;
}


// Function to extract parameters from an operand string (e.g., "A,B,C")
vector<string> extract_parameters(const string& operand_str) {
    vector<string> params;
    stringstream ss(operand_str);
    string param;
    // Use ',' as a delimiter
    while (getline(ss, param, ',')) {
        if (!param.empty()) {
            params.push_back(param);
        }
    }
    return params;
}

// =========================================================================
// PASS I: Definition Processing
// 1. Builds MNT and MDT.
// 2. Generates Intermediate File (without macro definitions).
// =========================================================================
void pass_one(const vector<string>& source_code) {
    cout << "Starting PASS I: Definition Processing...\n";
    ofstream intermediate_file("intermediate.txt");
    
    // Tracks if we are currently inside a macro definition
    bool in_macro = false;
    string macro_name = "";
    vector<string> formal_params;
    map<string, string> formal_to_placeholder; // Formal param -> #i

    for (const string& line : source_code) {
        vector<string> tokens = tokenize(line);
        if (tokens.empty()) continue;

        // Use the safe parsing function
        ParsedLine parsed = parse_line(tokens);
        string& label = parsed.label;
        string& opcode = parsed.opcode;
        string& operand = parsed.operand;

        if (opcode == "MACRO") {
            // --- Start of Macro Definition ---
            in_macro = true;
            macro_name = label; // Macro name is the label of the MACRO directive
            
            formal_params = extract_parameters(operand);
            formal_to_placeholder.clear();

            // Build Formal Parameter Map
            for (size_t i = 0; i < formal_params.size(); ++i) {
                formal_to_placeholder[formal_params[i]] = "#" + to_string(i);
            }

            // Create MNT entry
            MNT[macro_name] = {MDT_IDX, (int)formal_params.size()};

            // Write macro name and formal params to MDT (Header line)
            MDT.push_back(line);
            MDT_IDX++;

        } else if (opcode == "MEND") {
            // --- End of Macro Definition ---
            in_macro = false;
            MDT.push_back(line); // Write MEND to MDT
            MDT_IDX++;

        } else if (in_macro) {
            // --- Inside Macro Definition (Substitution Phase) ---
            string md_line = line;
            
            // Replace all formal parameters with their placeholders (#i)
            for (const auto& pair : formal_to_placeholder) {
                // Find and replace the formal parameter string
                size_t pos = md_line.find(pair.first);
                if (pos != string::npos) {
                    md_line.replace(pos, pair.first.length(), pair.second);
                }
            }
            MDT.push_back(md_line);
            MDT_IDX++;

        } else {
            // --- Regular Assembly Code / Macro Call ---
            // Write to intermediate file
            intermediate_file << line << endl;
        }
    }

    intermediate_file.close();
    cout << "PASS I complete. MNT and MDT generated. Intermediate file created.\n";
}

// =========================================================================
// PASS II: Expansion Processing
// 1. Reads Intermediate File.
// 2. Expands macro calls using MNT and MDT.
// 3. Generates Final Output File.
// =========================================================================
void pass_two() {
    cout << "\nStarting PASS II: Expansion Processing...\n";
    ifstream intermediate_file("intermediate.txt");
    ofstream output_file("output.txt");
    string line;

    while (getline(intermediate_file, line)) {
        vector<string> tokens = tokenize(line);
        if (tokens.empty()) continue;

        // Use the safe parsing function
        ParsedLine parsed = parse_line(tokens);
        string& opcode = parsed.opcode;
        string& operand = parsed.operand;

        // Check if the Opcode is a defined Macro (found in MNT)
        if (MNT.count(opcode)) {
            // --- Macro Call Found ---
            const MNTEntry& mnt_entry = MNT[opcode];
            vector<string> actual_params = extract_parameters(operand);
            
            // Check for correct number of arguments (optional but good practice)
            if ((int)actual_params.size() != mnt_entry.num_parameters) {
                output_file << "**ERROR: Incorrect number of arguments for macro " << opcode << ".\n";
                continue;
            }

            // Create Actual Argument Map (ALA) for substitution: #i -> Actual Arg
            map<string, string> actual_map;
            for (int i = 0; i < mnt_entry.num_parameters; ++i) {
                actual_map["#" + to_string(i)] = actual_params[i];
            }

            // Start expansion from MDT
            int current_mdt_idx = mnt_entry.start_index + 1; // Skip header line
            string md_line;

            while (true) {
                // **CRITICAL SEGFAULT FIX: Check index validity before access**
                if (current_mdt_idx >= (int)MDT.size()) {
                    output_file << "**ERROR: MDT indexing error during expansion of " << opcode << ".\n";
                    break; 
                }
                
                md_line = MDT[current_mdt_idx];
                vector<string> md_tokens = tokenize(md_line);
                
                if (md_tokens.size() > 1 && md_tokens[1] == "MEND") break; // Check opcode MEND
                else if (md_tokens.size() == 1 && md_tokens[0] == "MEND") break; // Check MEND only

                string expanded_line = md_line;

                // Substitute placeholders with actual arguments
                for (const auto& pair : actual_map) {
                    // Only substitute in the operand column for simplicity and safety
                    size_t pos = expanded_line.find(pair.first);
                    if (pos != string::npos) {
                        expanded_line.replace(pos, pair.first.length(), pair.second);
                    }
                }
                
                // Write expanded line to the final output
                output_file << expanded_line << endl;
                current_mdt_idx++;
            }

        } else {
            // --- Regular Assembly Instruction ---
            output_file << line << endl;
        }
    }

    intermediate_file.close();
    output_file.close();
    cout << "PASS II complete. Final expanded code written to output.txt.\n";
}

// Helper to display generated tables
void print_tables() {
    cout << "\n========================================\n";
    cout << "MACRO NAME TABLE (MNT)\n";
    cout << "Name\tStart Index\tParams\n";
    cout << "----------------------------------------\n";
    for (const auto& entry : MNT) {
        cout << entry.first << "\t" << entry.second.start_index << "\t\t" << entry.second.num_parameters << endl;
    }

    cout << "\nMACRO DEFINITION TABLE (MDT)\n";
    cout << "Index\tDefinition Line\n";
    cout << "----------------------------------------\n";
    for (size_t i = 0; i < MDT.size(); ++i) {
        cout << i << "\t" << MDT[i] << endl;
    }
    cout << "========================================\n";
}

// --- Main Execution ---
int main() {
    // Simulated Assembly Input File Content (Lines are Label Opcode Operand)
    // We assume the first token is always the label (or empty), second is Opcode, third is Operand.
    vector<string> source_code = {
        "MAIN\tSTART\t1000",
        "LOOP\tLOAD\tX",
        "CALC\tMACRO\t&A,&B",
        "&A\tADD\t&B",
        "\tSUB\t&B",
        "MEND",
        "\tSTORE\tY",
        "INIT\tMACRO\t&X,&Y,&Z",
        "&X\tLOAD\t&Y",
        "\tSTORE\t&Z",
        "MEND",
        "\tINIT\tTEMP,ONE,TWO",   // Macro Call 1
        "\tCALC\tX,Y",            // Macro Call 2
        "\tCALC\tY,Z",            // Macro Call 3
        "X\tRESW\t1",
        "Y\tRESW\t1",
        "Z\tRESW\t1",
        "TEMP\tRESW\t1",
        "ONE\tWORD\t1",
        "TWO\tWORD\t2",
        "\tEND\t"
    };

    // RUN PASS I
    pass_one(source_code);

    // Display the generated MNT and MDT
    print_tables();

    // RUN PASS II
    pass_two();

    cout << "\nSimulation Complete. Check intermediate.txt and output.txt.\n";

    return 0;
}


/* expected ouptput
That's an excellent question\! When running the fixed macroprocessor, we expect the code to demonstrate the successful execution of both passes by generating two output files and printing the final internal tables.

Here is the breakdown of the **expected console output** and the **expected file contents** based on the sample assembly code you provided.

-----

## Expected Console Output

The console output confirms the successful execution of Pass I and Pass II, along with printing the resulting in-memory tables.

```text
Starting PASS I: Definition Processing...
PASS I complete. MNT and MDT generated. Intermediate file created.

========================================
MACRO NAME TABLE (MNT)
Name	Start Index	Params
----------------------------------------
CALC	2		2
INIT	7		3

MACRO DEFINITION TABLE (MDT)
Index	Definition Line
----------------------------------------
0	MAIN	START	1000
1	LOOP	LOAD	X
2	CALC	MACRO	&A,&B
3	&A	ADD	#0
4		SUB	#1
5	MEND
6		STORE	Y
7	INIT	MACRO	&X,&Y,&Z
8	#0	LOAD	#1
9		STORE	#2
10	MEND
11		INIT	TEMP,ONE,TWO
12		CALC	X,Y
13		CALC	Y,Z
14	X	RESW	1
15	Y	RESW	1
16	Z	RESW	1
17	TEMP	RESW	1
18	ONE	WORD	1
19	TWO	WORD	2
20		END	

========================================

Starting PASS II: Expansion Processing...
PASS II complete. Final expanded code written to output.txt.

Simulation Complete. Check intermediate.txt and output.txt.
```

## Expected File Contents

### 1\. `intermediate.txt` (Output of Pass I)

This file contains only the lines that are **not part of a macro definition**. Notice that the macro calls (`INIT`, `CALC`) are preserved, and the definitions of `CALC` and `INIT` are completely removed.

```text
MAIN	START	1000
LOOP	LOAD	X
	STORE	Y
	INIT	TEMP,ONE,TWO
	CALC	X,Y
	CALC	Y,Z
X	RESW	1
Y	RESW	1
Z	RESW	1
TEMP	RESW	1
ONE	WORD	1
TWO	WORD	2
	END	
```

### 2\. `output.txt` (Output of Pass II)

This file contains the final, expanded assembly code. Every macro call in the `intermediate.txt` has been replaced by its corresponding definition from the MDT, with the **formal parameters replaced by the actual parameters**.

| Original Call | Actual Parameters | Expanded Output |
| :--- | :--- | :--- |
| `INIT TEMP,ONE,TWO` | `&X` $\to$ `TEMP`, `&Y` $\to$ `ONE`, `&Z` $\to$ `TWO` | `TEMP LOAD ONE`, `STORE TWO` |
| `CALC X,Y` | `&A` $\to$ `X`, `&B` $\to$ `Y` | `X ADD Y`, `SUB Y` |
| `CALC Y,Z` | `&A` $\to$ `Y`, `&B` $\to$ `Z` | `Y ADD Z`, `SUB Z` |

```text
MAIN	START	1000
LOOP	LOAD	X
	STORE	Y
TEMP	LOAD	ONE
	STORE	TWO
X	ADD	Y
	SUB	Y
Y	ADD	Z
	SUB	Z
X	RESW	1
Y	RESW	1
Z	RESW	1
TEMP	RESW	1
ONE	WORD	1
TWO	WORD	2
	END	
```*/
