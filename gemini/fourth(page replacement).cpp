/*
Write a program to simulate Page replacement algorithm
*/
#include <bits/stdc++.h>
using namespace std;

// Function to display page frames
void display(vector<int> frames) {
    for (int f : frames)
        cout << f << " ";
    cout << "\n";
}

// FIFO Page Replacement
void fifo(vector<int> pages, int capacity) {
    cout << "\n=== FIFO Page Replacement ===\n";
    vector<int> frames;
    queue<int> q;
    int faults = 0;

    for (int p : pages) {
        if (find(frames.begin(), frames.end(), p) == frames.end()) {
            if (frames.size() < capacity) {
                frames.push_back(p);
                q.push(p);
            } else {
                int victim = q.front();
                q.pop();
                q.push(p);
                replace(frames.begin(), frames.end(), victim, p);
            }
            faults++;
            cout << "Page " << p << " -> ";
            display(frames);
        } else {
            cout << "Page " << p << " -> No page fault\n";
        }
    }

    cout << "Total Page Faults = " << faults << "\n";
}

// LRU Page Replacement
void lru(vector<int> pages, int capacity) {
    cout << "\n=== LRU Page Replacement ===\n";
    vector<int> frames;
    unordered_map<int, int> lastUsed;
    int faults = 0, time = 0;

    for (int p : pages) {
        time++;
        if (find(frames.begin(), frames.end(), p) == frames.end()) {
            if (frames.size() < capacity)
                frames.push_back(p);
            else {
                int lruPage = frames[0], minTime = INT_MAX;
                for (int f : frames) {
                    if (lastUsed[f] < minTime) {
                        minTime = lastUsed[f];
                        lruPage = f;
                    }
                }
                replace(frames.begin(), frames.end(), lruPage, p);
            }
            faults++;
            cout << "Page " << p << " -> ";
            display(frames);
        } else {
            cout << "Page " << p << " -> No page fault\n";
        }
        lastUsed[p] = time;
    }

    cout << "Total Page Faults = " << faults << "\n";
}

// Optimal Page Replacement
void optimal(vector<int> pages, int capacity) {
    cout << "\n=== Optimal Page Replacement ===\n";
    vector<int> frames;
    int faults = 0;

    for (int i = 0; i < pages.size(); i++) {
        int p = pages[i];
        if (find(frames.begin(), frames.end(), p) == frames.end()) {
            if (frames.size() < capacity)
                frames.push_back(p);
            else {
                int victim = -1, farthest = i + 1;
                for (int f : frames) {
                    int j;
                    for (j = i + 1; j < pages.size(); j++)
                        if (pages[j] == f) break;
                    if (j == pages.size()) {
                        victim = f;
                        break;
                    }
                    if (j > farthest) {
                        farthest = j;
                        victim = f;
                    }
                }
                replace(frames.begin(), frames.end(), victim, p);
            }
            faults++;
            cout << "Page " << p << " -> ";
            display(frames);
        } else {
            cout << "Page " << p << " -> No page fault\n";
        }
    }

    cout << "Total Page Faults = " << faults << "\n";
}

// Main Function
int main() {
    vector<int> pages = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};
    int capacity = 3;

    fifo(pages, capacity);
    lru(pages, capacity);
    optimal(pages, capacity);

    return 0;
}
/*
Sure 👍 — here’s a **complete C++ program** that simulates the three main **Page Replacement Algorithms** used in operating systems:

👉 **FIFO**, **LRU**, and **Optimal**.

---

## 🧠 Concept Summary

When a process needs a page that’s not in memory (a **page fault**), the OS must **replace** one of the existing pages in memory.

| Algorithm                     | Strategy                                                                   |
| ----------------------------- | -------------------------------------------------------------------------- |
| **FIFO (First In First Out)** | Replace the page that entered memory first.                                |
| **LRU (Least Recently Used)** | Replace the page that has not been used for the longest time.              |
| **Optimal**                   | Replace the page that will not be used for the longest time in the future. |

---

## ✍️ Short Explanation

This program simulates three page replacement algorithms (FIFO, LRU, Optimal).
It uses a predefined sequence of page references and a fixed number of memory frames (3).
Each algorithm tracks **page faults** and replaces pages according to its strategy:

* **FIFO** removes the oldest loaded page.
* **LRU** removes the least recently used page.
* **Optimal** removes the page that will not be used for the longest time in the future.

At the end, the total number of **page faults** is displayed for performance comparison.
*/
