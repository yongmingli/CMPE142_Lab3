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

#ifndef INS_H
#define INS_H

struct ins{
    // instruction class
    int proc_id;
    char action;

    //• ‘C’ means the process is created (‘PAGE’ field not present)
    //• ‘T’ means the process terminated (‘PAGE’ field not present)
    //• ‘A’ means the process allocated memory at address ‘PAGE’
    //• ‘R’ means the process read ‘PAGE’
    //• ‘W’ means the process wrote to ‘PAGE’
    //• ‘F’ means the process freed memory at address ‘PAGE’

    int page;
    // virtual address for process
    // when action is 'C' or 'T', it is equal to -1
};

#endif //INS_H
