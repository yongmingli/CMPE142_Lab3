//
// Created by 李涌铭
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <climits>
#include <cstdlib>
#include <ctime>

#include "ins.h"

using namespace std;

#ifndef PROC_H
#define PROC_H

struct keep_swap{
    int proc_id;
    //
    int viradd;
    //
}; // keep track of the swapping

struct physical
{
    int process_id; // remember which process is using
    // if it is -1: means FREE

    bool dirty; //if this page has been W (for LRU)
    int time;   //keep track of last access time (for LRU)
    // this time will increment:
    // when W ++
    // when R ++
    // so, for the “oldest” modified or accessed page: it would choose the lowest number.
    // it would be 0 for default (smallest value).
    // --------
    // for LRU, if all pages are clean, it would swap the first page.
};

struct address{
    int viradd; //virtual address
    int phyadd; //physical address
    // if physical address = -1
    // it means "swap"

};

struct proc{
    int proc_id;
    vector<address> proc_add;
};

#endif // PROC_H