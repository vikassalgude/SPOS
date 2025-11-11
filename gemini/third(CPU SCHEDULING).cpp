/*
  Write a program to simulate CPU Scheduling Algorithms: FCFS, SJF (Preemptive), Priority
(Non-Preemptive) and Round Robin (Preemptive).
*/
#include <bits/stdc++.h>
using namespace std;

struct Process {
    int pid, at, bt, prio, ct, tat, wt, rt;
};

// Function to display results
void display(vector<Process> &p) {
    int n = p.size();
    float avgTAT = 0, avgWT = 0;
    cout << "\nPID\tAT\tBT\tPR\tCT\tTAT\tWT\n";
    for (auto &x : p) {
        x.tat = x.ct - x.at;
        x.wt = x.tat - x.bt;
        avgTAT += x.tat;
        avgWT += x.wt;
        cout << x.pid << "\t" << x.at << "\t" << x.bt << "\t" << x.prio << "\t"
             << x.ct << "\t" << x.tat << "\t" << x.wt << "\n";
    }
    cout << "Average TAT: " << avgTAT / n << "\n";
    cout << "Average WT : " << avgWT / n << "\n";
}

// 1️⃣ FCFS
void fcfs(vector<Process> p) {
    sort(p.begin(), p.end(), [](auto &a, auto &b) { return a.at < b.at; });
    int t = 0;
    for (auto &x : p) {
        t = max(t, x.at);
        t += x.bt;
        x.ct = t;
    }
    cout << "\n=== FCFS Scheduling ===\n";
    display(p);
}

// 2️⃣ SJF (Preemptive)
void sjf(vector<Process> p) {
    int n = p.size(), done = 0, t = 0;
    for (auto &x : p) x.rt = x.bt;

    while (done < n) {
        int idx = -1, mn = 1e9;
        for (int i = 0; i < n; i++) {
            if (p[i].at <= t && p[i].rt > 0 && p[i].rt < mn) {
                mn = p[i].rt;
                idx = i;
            }
        }
        if (idx == -1) {
            t++;
            continue;
        }
        p[idx].rt--;
        t++;
        if (p[idx].rt == 0) {
            p[idx].ct = t;
            done++;
        }
    }
    cout << "\n=== SJF (Preemptive) Scheduling ===\n";
    display(p);
}

// 3️⃣ Priority (Non-Preemptive)
void prioritySched(vector<Process> p) {
    sort(p.begin(), p.end(), [](auto &a, auto &b) {
        return a.at < b.at;
    });
    int t = 0, done = 0;
    vector<int> vis(p.size(), 0);

    while (done < p.size()) {
        int idx = -1, best = 1e9;
        for (int i = 0; i < p.size(); i++) {
            if (!vis[i] && p[i].at <= t && p[i].prio < best) {
                best = p[i].prio;
                idx = i;
            }
        }
        if (idx == -1) {
            t++;
            continue;
        }
        t += p[idx].bt;
        p[idx].ct = t;
        vis[idx] = 1;
        done++;
    }

    cout << "\n=== Priority (Non-Preemptive) Scheduling ===\n";
    display(p);
}

// 4️⃣ Round Robin (Preemptive)
void roundRobin(vector<Process> p, int q) {
    int n = p.size(), t = 0;
    queue<int> ready;
    vector<int> rt(n), vis(n, 0);
    for (int i = 0; i < n; i++) rt[i] = p[i].bt;
    int done = 0;
    while (done < n) {
        for (int i = 0; i < n; i++) {
            if (!vis[i] && p[i].at <= t) {
                ready.push(i);
                vis[i] = 1;
            }
        }

        if (ready.empty()) {
            t++;
            continue;
        }

        int i = ready.front();
        ready.pop();
        int exec = min(q, rt[i]);
        rt[i] -= exec;
        t += exec;

        for (int j = 0; j < n; j++) {
            if (!vis[j] && p[j].at <= t)
                ready.push(j), vis[j] = 1;
        }

        if (rt[i] == 0) {
            p[i].ct = t;
            done++;
        } else
            ready.push(i);
    }

    cout << "\n=== Round Robin Scheduling ===\n";
    display(p);
}

// 🧩 Main Function (no input)
int main() {
    // Predefined process list
    vector<Process> p = {
        {1, 0, 5, 2},   // pid, AT, BT, Priority
        {2, 1, 3, 1},
        {3, 2, 8, 3},
        {4, 3, 6, 2}
    };

    int quantum = 2;  // Fixed time quantum for Round Robin

    fcfs(p);
    sjf(p);
    prioritySched(p);
    roundRobin(p, quantum);

    return 0;
}
/*
EXPLANATION
Here’s a **short and clear code summary** you can include with your explanation 👇

---

### 💻 **Code Summary**

* The program defines a `Process` structure containing process ID, Arrival Time, Burst Time, Priority, Completion Time, Turnaround Time, Waiting Time, and Remaining Time.
* A **vector of processes** is used to store all process data (hardcoded in the program).
* Four separate functions simulate each scheduling algorithm:

  * `fcfs()` → Implements **First Come First Serve** by sorting processes by arrival time.
  * `sjf()` → Implements **Shortest Job First (Preemptive)** by selecting the process with the shortest remaining burst time at each moment.
  * `prioritySched()` → Implements **Priority (Non-Preemptive)** scheduling based on the smallest priority value.
  * `roundRobin()` → Implements **Round Robin (Preemptive)** using a queue and a fixed time quantum.
* Each function calculates and prints **Completion Time**, **Turnaround Time**, **Waiting Time**, and their averages using a common `display()` function.
* The `main()` function initializes process data, sets the quantum, and calls all four scheduling functions sequentially.

---



---

### 🧠 **CPU Scheduling Algorithms Simulation**

This program simulates four major CPU scheduling algorithms:

1. **FCFS (First Come First Serve)** –
   Non-preemptive algorithm.
   Processes are executed in the order they arrive.
   Waiting time depends on the arrival order.

2. **SJF (Shortest Job First - Preemptive)** –
   Also known as *Shortest Remaining Time First (SRTF)*.
   The process with the smallest remaining burst time is executed first.
   It minimizes average waiting time but may cause starvation.

3. **Priority Scheduling (Non-Preemptive)** –
   Each process has a priority value.
   The process with the highest priority (lowest number) is executed first.
   If two processes have the same priority, arrival time decides the order.

4. **Round Robin (Preemptive)** –
   Each process gets a fixed time quantum (e.g., 2 units).
   After its time expires, it goes to the end of the queue.
   Ensures fairness and good response time for all processes.

---

### ⚙️ **Program Summary**

* Uses a structure to store process details: PID, Arrival Time, Burst Time, Priority, etc.
* Each algorithm calculates **Completion Time (CT)**, **Turnaround Time (TAT)**, and **Waiting Time (WT)**.
* Results show the average TAT and WT for performance comparison.
* Process data is predefined (no user input).

---
*/
