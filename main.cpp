//
//
//
// CMPE 142 Lab3
// Author: Yongming Li  李涌铭
//
//
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <climits>
#include <cstdlib>
#include <ctime>

#include "ins.h"
#include "proc.h"

using namespace std;

// main function
void show(ins ins_set[], int ins_size);
void FIFO(ins ins_set[], int ins_size);
void LRU(ins ins_set[], int ins_size);
void Random(ins ins_set[], int ins_size);

// tool function
int find_proc_location(vector<proc> process, int proc_id); // return index
int find_action_page(vector<proc> process, int location, int viradd); // return index
int find_swap_page(vector<proc> process, int location, int phyadd); // return index
int check_free(physical phy_pages[20]); // return index
void print_result(vector<proc> process,vector<keep_swap> swap,vector<int> kill,physical phy_pages[20]);
int random_point(); // find swap index for Random only
int LRU_point(physical phy_pages[20]); // find swap index for LRU only

int main() {
    int choice = -1;
    int ins_size = 0;

    //start reading from file
    ifstream in;

    //in.open("memory.txt");
    in.open("memory.dat");

    if (in.fail()) {
        cout << "Unable to open memory.dat files. CANNOT continue!(1)" << endl;
        exit(1);
    }

    // determine the instruction size
    string unused;
    while ( std::getline(in, unused)) ++ins_size;
    in.close();

    //in.open("memory.txt");
    in.open("memory.dat");
    if (in.fail()) {
        cout << "Unable to open memory.dat files. CANNOT continue!(2)" << endl;
        exit(1);
    }

    ins ins_set[ins_size]; //instruction array

    string nextline;
    for (int i=0;i<ins_size && getline(in >> ws,nextline); ++i)
    {
        int int_proc_id;
        char int_action;
        int int_page;

        stringstream(nextline) >> int_proc_id >> int_action;
        if (int_action == 'C' ||int_action == 'T') int_page = -1;
        else stringstream(nextline) >> int_proc_id >> int_action >> int_page;

        ins_set[i].proc_id = int_proc_id;
        ins_set[i].action = int_action;
        ins_set[i].page = int_page;
    }

    cout    <<endl
            << "Hello, Welcome to use LYM Memory Management program." << endl
            << "Successfully read from memory.dat file!\n" << endl;

    while (choice != 0) {
        cout << "Please choose from following option:" << endl
             << "-------------ENJOY-----------------------------------------------------" <<endl
             << "-----------------------------------------------------------------------" << endl

             << "1. Show instructions " << endl
             << "2. FIFO " << endl
             << "3. LRU " << endl
             << "4. Random " << endl
             << "0. EXIT! " << endl;

        cin >> choice;

        if (choice == 1) show(ins_set, ins_size);
        else if (choice == 2) FIFO(ins_set, ins_size);
        else if (choice == 3) LRU(ins_set, ins_size);
        else if (choice == 4) Random(ins_set, ins_size);
        else if (choice == 0) cout <<"Program exiting......" <<endl;
        else cout << "Wrong choice! Please choose again!" << endl;
    }

    in.close();
    cout << "Program exit!!!" << endl;
}

void show(ins ins_set[], int ins_size)
{
    cout << "There are "<< ins_size << " instructions in the memory.dat file." << endl;
    cout << "Process ID" << "\t" <<"Action" << "\t" <<"Page" << endl;
    for (int i=0; i<ins_size; i++)
    {
        cout << ins_set[i].proc_id << "\t\t" << ins_set[i].action << "\t";
        if (ins_set[i].page != -1) cout <<ins_set[i].page << endl;
        else cout<<"\n";
    }
    cout <<"\n";
}
void FIFO(ins ins_set[], int ins_size){
    vector<proc> process; // keep all process
    vector<keep_swap> swap; // keep all swap
    vector<int> kill; // keep all kill
    // contain the process id

    physical phy_pages[20]; //physical addresses
    for (int i=0; i<20; i++){

        phy_pages[i].process_id = -1; // FREE page
        phy_pages[i].dirty = false;
        phy_pages[i].time = 0;

    }// build physical pages

    vector <int> pointer;
    // keep update the swap location for FIFO only

    // main loop for all instructions
    for (int i=0; i<ins_size; i++)
    {
        int location; // find the location of the process
        location = find_proc_location(process, ins_set[i].proc_id);
        if (location == -1) // could not find the process
        {
         if (ins_set[i].action != 'C')
             cout << "Could NOT find process" << ins_set[i].proc_id <<endl
             << "Instruction "<< i << " is an invalid instruction, that could cause page fault!!!\n"<<endl;
         else {
             proc temp = {ins_set[i].proc_id};
             process.push_back(temp);
         }
        }
        else
        {
            // find the process
            if (ins_set[i].action == 'T'){

                //clean the physical address accordingly
                for (int x=0; x<20; x++)
                {
                    if (phy_pages[x].process_id == process[location].proc_id) {
                        phy_pages[x].process_id = -1;
                        phy_pages[x].time = 0;
                        phy_pages[x].dirty = false;
                    }
                }

                //clean the swap!!!
                vector<keep_swap> ::iterator clean_swap = swap.begin();
                for (int x = 0; x < swap.size()+1; x++)
                {
                    if (swap[x].proc_id == process[location].proc_id)
                    {
                        swap.erase(clean_swap);
                        x = 0;
                        clean_swap = swap.begin();
                    }
                    else clean_swap++;
                }

                //• ‘T’ means the process terminated (‘PAGE’ field not present)
                vector<proc>::iterator it = process.begin();
                for (int x =0; x<location; x++) it++;
                process.erase(it);
            }
            else if (ins_set[i].action == 'R'){

                int page_location;
                page_location = find_action_page(process, location, ins_set[i].page);
                if (page_location == -1)
                {
                    //wrong page number
                    //kill the process
                    kill.push_back(process[location].proc_id); // update kill
                    //clean the physical address accordingly
                    for (int x=0; x<20; x++)
                    {
                        if (phy_pages[x].process_id == process[location].proc_id) {
                            phy_pages[x].process_id = -1;
                            phy_pages[x].time = 0;
                            phy_pages[x].dirty = false;
                        }
                    }

                    //clean the swap!!!
                    vector<keep_swap> ::iterator clean_swap = swap.begin();
                    for (int x = 0; x < swap.size()+1; x++)
                    {
                        if (swap[x].proc_id == process[location].proc_id)
                        {
                            swap.erase(clean_swap);
                            x = 0;
                            clean_swap = swap.begin();
                        }
                        else clean_swap++;
                    }

                    vector<proc>::iterator it = process.begin();
                    for (int x =0; x<location; x++) it++;
                    process.erase(it);
                }
                else //find the page location in process
                {
                    int index = process[location].proc_add[page_location].phyadd;

                    if (index == -1 ) // Read a SWAP page
                    {
                        // clean (update) swap first
                        vector<keep_swap> ::iterator clean_swap = swap.begin();
                        for (int x = 0; x < swap.size(); x++)
                        {
                            if (swap[x].proc_id == process[location].proc_id
                            && swap[x].viradd == ins_set[i].page)
                            {
                                swap.erase(clean_swap);
                            }
                            else clean_swap++;
                        }

                        int free_page = check_free(phy_pages);
                        if (free_page != -1)
                        {
                            // there is a free page, and free_page is its index
                            phy_pages[free_page].process_id = process[location].proc_id; // physical address assigned

                            process[location].proc_add[page_location].phyadd = free_page;
                            // process addresses update
                            // physical address
                            pointer.push_back(free_page); //update pointer
                        }
                        else
                            // there is no free page, need swap
                        {
                            int point;
                            if (pointer.size() == 0) point = 0;
                            else point = pointer[0];
                            // swap the point value in physical address
                            // point is the physical address

                            // find the current process of the swap page
                            int object_id = phy_pages[point].process_id;

                            // find the process location
                            int object_locaton = find_proc_location(process, object_id); // process location index

                            // find the page location
                            int object_page = find_swap_page(process,object_locaton, point); // page location index

                            // update the old page owner
                            process[object_locaton].proc_add[object_page].phyadd = -1; // means: swap

                            //put the new (swapped) page for 'A'
                            phy_pages[point].process_id = process[location].proc_id; // physical address assigned
                            phy_pages[point].dirty = false;  // update
                            phy_pages[point].time = 1;  // R update

                            process[location].proc_add[page_location].phyadd = point;

                            //update swap
                            keep_swap temp_swap = {object_id,process[object_locaton].proc_add[object_page].viradd};
                            swap.push_back(temp_swap); // keep track of the swap

                            pointer.erase(pointer.begin());
                            pointer.push_back(object_locaton); //update pointer
                        }
                    }
                    else phy_pages[index].time++;
                }
            }
            else if (ins_set[i].action == 'W'){

                //have NOT consider the swap situation
                // when physical address = -1

                int page_location;
                page_location = find_action_page(process, location, ins_set[i].page);
                if (page_location == -1)
                {
                    //wrong page number
                    //kill the process
                    kill.push_back(process[location].proc_id); // update kill
                    //clean the physical address accordingly
                    for (int x=0; x<20; x++)
                    {
                        if (phy_pages[x].process_id == process[location].proc_id) {
                            phy_pages[x].process_id = -1;
                            phy_pages[x].time = 0;
                            phy_pages[x].dirty = false;
                        }
                    }

                    //clean the swap!!!
                    vector<keep_swap> ::iterator clean_swap = swap.begin();
                    for (int x = 0; x < swap.size()+1; x++)
                    {
                        if (swap[x].proc_id == process[location].proc_id)
                        {
                            swap.erase(clean_swap);
                            x = 0;
                            clean_swap = swap.begin();
                        }
                        else clean_swap++;
                    }

                    vector<proc>::iterator it = process.begin();
                    for (int x =0; x<location; x++) it++;
                    process.erase(it);
                }
                else //find the page location in process
                {
                    int index = process[location].proc_add[page_location].phyadd;

                    if (index == -1 ) // Read a SWAP page
                    {
                        // clean (update) swap first
                        vector<keep_swap> ::iterator clean_swap = swap.begin();
                        for (int x = 0; x < swap.size(); x++)
                        {
                            if (swap[x].proc_id == process[location].proc_id
                                && swap[x].viradd == ins_set[i].page)
                            {
                                swap.erase(clean_swap);
                            }
                            else clean_swap++;
                        }

                        int free_page = check_free(phy_pages);
                        if (free_page != -1)
                        {
                            // there is a free page, and free_page is its index
                            phy_pages[free_page].process_id = process[location].proc_id; // physical address assigned

                            process[location].proc_add[page_location].phyadd = free_page;
                            // process addresses update
                            // physical address
                            pointer.push_back(free_page); //update pointer
                        }
                        else
                            // there is no free page, need swap
                        {
                            int point;
                            if (pointer.size() == 0) point = 0;
                            else point = pointer[0];
                            // swap the point value in physical address
                            // point is the physical address

                            // find the current process of the swap page
                            int object_id = phy_pages[point].process_id;

                            // find the process location
                            int object_locaton = find_proc_location(process, object_id); // process location index

                            // find the page location
                            int object_page = find_swap_page(process,object_locaton, point); // page location index

                            // update the old page owner
                            process[object_locaton].proc_add[object_page].phyadd = -1; // means: swap

                            //put the new (swapped) page for 'A'
                            phy_pages[point].process_id = process[location].proc_id; // physical address assigned
                            phy_pages[point].dirty = true;  // update
                            phy_pages[point].time = 1;  // W update

                            process[location].proc_add[page_location].phyadd = point;

                            //update swap
                            keep_swap temp_swap = {object_id,process[object_locaton].proc_add[object_page].viradd};
                            swap.push_back(temp_swap); // keep track of the swap

                            pointer.erase(pointer.begin());
                            pointer.push_back(object_locaton); //update pointer
                        }
                    }
                    else {
                        phy_pages[index].dirty = true;
                        phy_pages[index].time++;
                    }
                }
            }
            else if (ins_set[i].action == 'F'){

                //have NOT consider the swap situation
                // when physical address = -1
                // ned to clean the swap

                int page_location;
                page_location = find_action_page(process, location, ins_set[i].page);
                if (page_location == -1)
                {
                    //wrong page number
                    //kill the process
                    kill.push_back(process[location].proc_id); // update kill
                    //clean the physical address accordingly
                    for (int x=0; x<20; x++)
                    {
                        if (phy_pages[x].process_id == process[location].proc_id) {
                            phy_pages[x].process_id = -1;
                            phy_pages[x].time = 0;
                            phy_pages[x].dirty = false;
                        }
                    }

                    //clean the swap!!!
                    vector<keep_swap> ::iterator clean_swap = swap.begin();
                    for (int x = 0; x < swap.size()+1; x++)
                    {
                        if (swap[x].proc_id == process[location].proc_id)
                        {
                            swap.erase(clean_swap);
                            x = 0;
                            clean_swap = swap.begin();
                        }
                        else clean_swap++;
                    }

                    vector<proc>::iterator it = process.begin();
                    for (int x =0; x<location; x++) it++;
                    process.erase(it);
                }
                else //find the page location in process
                {
                    int index = process[location].proc_add[page_location].phyadd;

                    if (index == -1 ) // Free the SWAP page
                    {
                        // clean (update) swap
                        vector<keep_swap> ::iterator clean_swap = swap.begin();
                        for (int x = 0; x < swap.size(); x++)
                        {
                            if (swap[x].proc_id == process[location].proc_id
                                && swap[x].viradd == ins_set[i].page)
                            {
                                swap.erase(clean_swap);
                            }
                            else clean_swap++;
                        }
                    }
                    else
                    {
                        // free the physical address first
                        phy_pages[index].process_id = -1; // FREE page
                        phy_pages[index].dirty = false;
                        phy_pages[index].time = 0;

                        // then free the pointer
                        int one;
                        for (int x = 0; x<pointer.size();x++){
                            if (pointer[x]==index) one = x; // find location in pointer
                        }
                        vector<int>::iterator it1 = pointer.begin();
                        for (int x =0; x<one; x++) it1++;
                        pointer.erase(it1);
                    }

                    // finally free the virtual and physical address in process
                    vector<address>::iterator it = process[location].proc_add.begin();
                    for (int x =0; x<page_location; x++) it++;
                    process[location].proc_add.erase(it);
                }
            }
            else if (ins_set[i].action == 'A'){
                int page_location;
                page_location = find_action_page(process, location, ins_set[i].page);
                if (page_location != -1)
                {
                    //allocate the same page as previous
                    //kill the process
                    kill.push_back(process[location].proc_id); // update kill
                    //clean the physical address accordingly
                    for (int x=0; x<20; x++)
                    {
                        if (phy_pages[x].process_id == process[location].proc_id) {
                            phy_pages[x].process_id = -1;
                            phy_pages[x].time = 0;
                            phy_pages[x].dirty = false;
                        }
                    }

                    //clean the swap!!!
                    vector<keep_swap> ::iterator clean_swap = swap.begin();
                    for (int x = 0; x < swap.size()+1; x++)
                    {
                        if (swap[x].proc_id == process[location].proc_id)
                        {
                            swap.erase(clean_swap);
                            x = 0;
                            clean_swap = swap.begin();
                        }
                        else clean_swap++;
                    }

                    vector<proc>::iterator it = process.begin();
                    for (int x =0; x<location; x++) it++;
                    process.erase(it);
                }
                else// need to allocate a new page:
                {
                    int free_page = check_free(phy_pages);
                    if (free_page != -1)
                    {
                        // there is a free page, and free_page is its index
                        phy_pages[free_page].process_id = process[location].proc_id; // physical address assigned

                        address temp = {ins_set[i].page, free_page};
                        process[location].proc_add.push_back(temp);
                        // process addresses update
                        // virtual address
                        // physical address
                        pointer.push_back(free_page); //update pointer
                    }
                    else
                        // there is no free page, need swap
                    {
                        int point;
                        if (pointer.size() == 0) point = 0;
                        else point = pointer[0];
                        // swap the point value in physical address
                        // point is the physical address

                        // find the current process of the swap page
                        int object_id = phy_pages[point].process_id;

                        // find the process location
                        int object_locaton = find_proc_location(process, object_id); // process location index

                        // find the page location
                        int object_page = find_swap_page(process,object_locaton, point); // page location index

                        // update the old page owner
                        process[object_locaton].proc_add[object_page].phyadd = -1; // means: swap

                        //put the new (swapped) page for 'A'
                        phy_pages[point].process_id = process[location].proc_id; // physical address assigned
                        phy_pages[point].dirty = false;  // update
                        phy_pages[point].time = 0;  // update

                        address temp = {ins_set[i].page, point};
                        process[location].proc_add.push_back(temp); // process addresses update

                        //update swap
                        keep_swap temp_swap = {object_id,process[object_locaton].proc_add[object_page].viradd};
                        swap.push_back(temp_swap); // keep track of the swap

                        pointer.erase(pointer.begin());
                        pointer.push_back(object_locaton); //update pointer
                    }
                }
            }
            else{
                // wrong action
                // kill the process
                kill.push_back(process[location].proc_id); // update kill
                //clean the physical address accordingly
                for (int x=0; x<20; x++)
                {
                    if (phy_pages[x].process_id == process[location].proc_id) {
                        phy_pages[x].process_id = -1;
                        phy_pages[x].time = 0;
                        phy_pages[x].dirty = false;
                    }
                }

                //clean the swap!!!
                vector<keep_swap> ::iterator clean_swap = swap.begin();
                for (int x = 0; x < swap.size()+1; x++)
                {
                    if (swap[x].proc_id == process[location].proc_id)
                    {
                        swap.erase(clean_swap);
                        x = 0;
                        clean_swap = swap.begin();
                    }
                    else clean_swap++;
                }

                vector<proc>::iterator it = process.begin();
                for (int x =0; x<location; x++) it++;

                process.erase(it);
            }
        }
    } // all done
    // Display the results next
    cout << "\nThere are the results of FIFO swapping: "<<endl;
    print_result(process, swap, kill, phy_pages);
}
void LRU(ins ins_set[], int ins_size){
    vector<proc> process; // keep all process
    vector<keep_swap> swap; // keep all swap
    vector<int> kill; // keep all kill
    // contain the process id

    physical phy_pages[20]; //physical addresses
    for (int i=0; i<20; i++){

        phy_pages[i].process_id = -1; // FREE page
        phy_pages[i].dirty = false;
        phy_pages[i].time = 0;

    }// build physical pages

    // main loop for all instructions
    for (int i=0; i<ins_size; i++)
    {
        int location; // find the location of the process
        location = find_proc_location(process, ins_set[i].proc_id);
        if (location == -1) // could not find the process
        {
            if (ins_set[i].action != 'C')
                cout << "Could NOT find process" << ins_set[i].proc_id <<endl
                     << "Instruction "<< i << " is an invalid instruction, that could cause page fault!!!\n"<<endl;
            else {
                proc temp = {ins_set[i].proc_id};
                process.push_back(temp);
            }
        }
        else
        {
            // find the process
            if (ins_set[i].action == 'T'){

                //clean the physical address accordingly
                for (int x=0; x<20; x++)
                {
                    if (phy_pages[x].process_id == process[location].proc_id) {
                        phy_pages[x].process_id = -1;
                        phy_pages[x].time = 0;
                        phy_pages[x].dirty = false;
                    }
                }

                //clean the swap!!!
                vector<keep_swap> ::iterator clean_swap = swap.begin();
                for (int x = 0; x < swap.size()+1; x++)
                {
                    if (swap[x].proc_id == process[location].proc_id)
                    {
                        swap.erase(clean_swap);
                        x = 0;
                        clean_swap = swap.begin();
                    }
                    else clean_swap++;
                }

                //• ‘T’ means the process terminated (‘PAGE’ field not present)
                vector<proc>::iterator it = process.begin();
                for (int x =0; x<location; x++) it++;
                process.erase(it);
            }
            else if (ins_set[i].action == 'R'){

                int page_location;
                page_location = find_action_page(process, location, ins_set[i].page);
                if (page_location == -1)
                {
                    //wrong page number
                    //kill the process
                    kill.push_back(process[location].proc_id); // update kill
                    //clean the physical address accordingly
                    for (int x=0; x<20; x++)
                    {
                        if (phy_pages[x].process_id == process[location].proc_id) {
                            phy_pages[x].process_id = -1;
                            phy_pages[x].time = 0;
                            phy_pages[x].dirty = false;
                        }
                    }

                    //clean the swap!!!
                    vector<keep_swap> ::iterator clean_swap = swap.begin();
                    for (int x = 0; x < swap.size()+1; x++)
                    {
                        if (swap[x].proc_id == process[location].proc_id)
                        {
                            swap.erase(clean_swap);
                            x = 0;
                            clean_swap = swap.begin();
                        }
                        else clean_swap++;
                    }

                    vector<proc>::iterator it = process.begin();
                    for (int x =0; x<location; x++) it++;
                    process.erase(it);
                }
                else //find the page location in process
                {
                    int index = process[location].proc_add[page_location].phyadd;

                    if (index == -1 ) // Read a SWAP page
                    {
                        // clean (update) swap first
                        vector<keep_swap> ::iterator clean_swap = swap.begin();
                        for (int x = 0; x < swap.size(); x++)
                        {
                            if (swap[x].proc_id == process[location].proc_id
                                && swap[x].viradd == ins_set[i].page)
                            {
                                swap.erase(clean_swap);
                            }
                            else clean_swap++;
                        }

                        int free_page = check_free(phy_pages);
                        if (free_page != -1)
                        {
                            // there is a free page, and free_page is its index
                            phy_pages[free_page].process_id = process[location].proc_id; // physical address assigned

                            process[location].proc_add[page_location].phyadd = free_page;
                            // process addresses update
                            // physical address
                        }
                        else
                            // there is no free page, need swap
                        {
                            int point = LRU_point(phy_pages);
                            // swap the point value in physical address
                            // point is the physical address

                            // find the current process of the swap page
                            int object_id = phy_pages[point].process_id;

                            // find the process location
                            int object_locaton = find_proc_location(process, object_id); // process location index

                            // find the page location
                            int object_page = find_swap_page(process,object_locaton, point); // page location index

                            // update the old page owner
                            process[object_locaton].proc_add[object_page].phyadd = -1; // means: swap

                            //put the new (swapped) page for 'A'
                            phy_pages[point].process_id = process[location].proc_id; // physical address assigned
                            phy_pages[point].dirty = false;  // update
                            phy_pages[point].time = 1;  // R update

                            process[location].proc_add[page_location].phyadd = point;

                            //update swap
                            keep_swap temp_swap = {object_id,process[object_locaton].proc_add[object_page].viradd};
                            swap.push_back(temp_swap); // keep track of the swap
                        }
                    }
                    else phy_pages[index].time++;
                }
            }
            else if (ins_set[i].action == 'W'){

                //have NOT consider the swap situation
                // when physical address = -1

                int page_location;
                page_location = find_action_page(process, location, ins_set[i].page);
                if (page_location == -1)
                {
                    //wrong page number
                    //kill the process
                    kill.push_back(process[location].proc_id); // update kill
                    //clean the physical address accordingly
                    for (int x=0; x<20; x++)
                    {
                        if (phy_pages[x].process_id == process[location].proc_id) {
                            phy_pages[x].process_id = -1;
                            phy_pages[x].time = 0;
                            phy_pages[x].dirty = false;
                        }
                    }

                    //clean the swap!!!
                    vector<keep_swap> ::iterator clean_swap = swap.begin();
                    for (int x = 0; x < swap.size()+1; x++)
                    {
                        if (swap[x].proc_id == process[location].proc_id)
                        {
                            swap.erase(clean_swap);
                            x = 0;
                            clean_swap = swap.begin();
                        }
                        else clean_swap++;
                    }

                    vector<proc>::iterator it = process.begin();
                    for (int x =0; x<location; x++) it++;
                    process.erase(it);
                }
                else //find the page location in process
                {
                    int index = process[location].proc_add[page_location].phyadd;

                    if (index == -1 ) // Read a SWAP page
                    {
                        // clean (update) swap first
                        vector<keep_swap> ::iterator clean_swap = swap.begin();
                        for (int x = 0; x < swap.size(); x++)
                        {
                            if (swap[x].proc_id == process[location].proc_id
                                && swap[x].viradd == ins_set[i].page)
                            {
                                swap.erase(clean_swap);
                            }
                            else clean_swap++;
                        }

                        int free_page = check_free(phy_pages);
                        if (free_page != -1)
                        {
                            // there is a free page, and free_page is its index
                            phy_pages[free_page].process_id = process[location].proc_id; // physical address assigned

                            process[location].proc_add[page_location].phyadd = free_page;
                            // process addresses update
                            // physical address
                        }
                        else
                            // there is no free page, need swap
                        {
                            int point = LRU_point(phy_pages);
                            // swap the point value in physical address
                            // point is the physical address

                            // find the current process of the swap page
                            int object_id = phy_pages[point].process_id;

                            // find the process location
                            int object_locaton = find_proc_location(process, object_id); // process location index

                            // find the page location
                            int object_page = find_swap_page(process,object_locaton, point); // page location index

                            // update the old page owner
                            process[object_locaton].proc_add[object_page].phyadd = -1; // means: swap

                            //put the new (swapped) page for 'A'
                            phy_pages[point].process_id = process[location].proc_id; // physical address assigned
                            phy_pages[point].dirty = true;  // update
                            phy_pages[point].time = 1;  // W update

                            process[location].proc_add[page_location].phyadd = point;

                            //update swap
                            keep_swap temp_swap = {object_id,process[object_locaton].proc_add[object_page].viradd};
                            swap.push_back(temp_swap); // keep track of the swap
                        }
                    }
                    else {
                        phy_pages[index].dirty = true;
                        phy_pages[index].time++;
                    }
                }
            }
            else if (ins_set[i].action == 'F'){

                //have NOT consider the swap situation
                // when physical address = -1
                // ned to clean the swap

                int page_location;
                page_location = find_action_page(process, location, ins_set[i].page);
                if (page_location == -1)
                {
                    //wrong page number
                    //kill the process
                    kill.push_back(process[location].proc_id); // update kill
                    //clean the physical address accordingly
                    for (int x=0; x<20; x++)
                    {
                        if (phy_pages[x].process_id == process[location].proc_id) {
                            phy_pages[x].process_id = -1;
                            phy_pages[x].time = 0;
                            phy_pages[x].dirty = false;
                        }
                    }

                    //clean the swap!!!
                    vector<keep_swap> ::iterator clean_swap = swap.begin();
                    for (int x = 0; x < swap.size()+1; x++)
                    {
                        if (swap[x].proc_id == process[location].proc_id)
                        {
                            swap.erase(clean_swap);
                            x = 0;
                            clean_swap = swap.begin();
                        }
                        else clean_swap++;
                    }

                    vector<proc>::iterator it = process.begin();
                    for (int x =0; x<location; x++) it++;
                    process.erase(it);
                }
                else //find the page location in process
                {
                    int index = process[location].proc_add[page_location].phyadd;

                    if (index == -1 ) // Free the SWAP page
                    {
                        // clean (update) swap
                        vector<keep_swap> ::iterator clean_swap = swap.begin();
                        for (int x = 0; x < swap.size(); x++)
                        {
                            if (swap[x].proc_id == process[location].proc_id
                                && swap[x].viradd == ins_set[i].page)
                            {
                                swap.erase(clean_swap);
                            }
                            else clean_swap++;
                        }
                    }
                    else
                    {
                        // free the physical address first
                        phy_pages[index].process_id = -1; // FREE page
                        phy_pages[index].dirty = false;
                        phy_pages[index].time = 0;
                    }

                    // finally free the virtual and physical address in process
                    vector<address>::iterator it = process[location].proc_add.begin();
                    for (int x =0; x<page_location; x++) it++;
                    process[location].proc_add.erase(it);
                }
            }
            else if (ins_set[i].action == 'A'){
                int page_location;
                page_location = find_action_page(process, location, ins_set[i].page);
                if (page_location != -1)
                {
                    //allocate the same page as previous
                    //kill the process
                    kill.push_back(process[location].proc_id); // update kill
                    //clean the physical address accordingly
                    for (int x=0; x<20; x++)
                    {
                        if (phy_pages[x].process_id == process[location].proc_id) {
                            phy_pages[x].process_id = -1;
                            phy_pages[x].time = 0;
                            phy_pages[x].dirty = false;
                        }
                    }

                    //clean the swap!!!
                    vector<keep_swap> ::iterator clean_swap = swap.begin();
                    for (int x = 0; x < swap.size()+1; x++)
                    {
                        if (swap[x].proc_id == process[location].proc_id)
                        {
                            swap.erase(clean_swap);
                            x = 0;
                            clean_swap = swap.begin();
                        }
                        else clean_swap++;
                    }

                    vector<proc>::iterator it = process.begin();
                    for (int x =0; x<location; x++) it++;
                    process.erase(it);
                }
                else// need to allocate a new page:
                {
                    int free_page = check_free(phy_pages);
                    if (free_page != -1)
                    {
                        // there is a free page, and free_page is its index
                        phy_pages[free_page].process_id = process[location].proc_id; // physical address assigned

                        address temp = {ins_set[i].page, free_page};
                        process[location].proc_add.push_back(temp);
                        // process addresses update
                        // virtual address
                        // physical address
                    }
                    else
                        // there is no free page, need swap
                    {
                        int point = LRU_point(phy_pages);
                        // swap the point value in physical address
                        // point is the physical address

                        // find the current process of the swap page
                        int object_id = phy_pages[point].process_id;

                        // find the process location
                        int object_locaton = find_proc_location(process, object_id); // process location index

                        // find the page location
                        int object_page = find_swap_page(process,object_locaton, point); // page location index

                        // update the old page owner
                        process[object_locaton].proc_add[object_page].phyadd = -1; // means: swap

                        //put the new (swapped) page for 'A'
                        phy_pages[point].process_id = process[location].proc_id; // physical address assigned
                        phy_pages[point].dirty = false;  // update
                        phy_pages[point].time = 0;  // update

                        address temp = {ins_set[i].page, point};
                        process[location].proc_add.push_back(temp); // process addresses update

                        //update swap
                        keep_swap temp_swap = {object_id,process[object_locaton].proc_add[object_page].viradd};
                        swap.push_back(temp_swap); // keep track of the swap
                    }
                }
            }
            else{
                // wrong action
                // kill the process
                kill.push_back(process[location].proc_id); // update kill
                //clean the physical address accordingly
                for (int x=0; x<20; x++)
                {
                    if (phy_pages[x].process_id == process[location].proc_id) {
                        phy_pages[x].process_id = -1;
                        phy_pages[x].time = 0;
                        phy_pages[x].dirty = false;
                    }
                }

                //clean the swap!!!
                vector<keep_swap> ::iterator clean_swap = swap.begin();
                for (int x = 0; x < swap.size()+1; x++)
                {
                    if (swap[x].proc_id == process[location].proc_id)
                    {
                        swap.erase(clean_swap);
                        x = 0;
                        clean_swap = swap.begin();
                    }
                    else clean_swap++;
                }

                vector<proc>::iterator it = process.begin();
                for (int x =0; x<location; x++) it++;

                process.erase(it);
            }
        }
    } // all done
    // Display the results next
    cout << "\nThere are the results of Random swapping: "<<endl;
    print_result(process, swap, kill, phy_pages);
}
void Random(ins ins_set[], int ins_size){
    vector<proc> process; // keep all process
    vector<keep_swap> swap; // keep all swap
    vector<int> kill; // keep all kill
    // contain the process id

    physical phy_pages[20]; //physical addresses
    for (int i=0; i<20; i++){

        phy_pages[i].process_id = -1; // FREE page
        phy_pages[i].dirty = false;
        phy_pages[i].time = 0;

    }// build physical pages

    // main loop for all instructions
    for (int i=0; i<ins_size; i++)
    {
        int location; // find the location of the process
        location = find_proc_location(process, ins_set[i].proc_id);
        if (location == -1) // could not find the process
        {
            if (ins_set[i].action != 'C')
                cout << "Could NOT find process" << ins_set[i].proc_id <<endl
                     << "Instruction "<< i << " is an invalid instruction, that could cause page fault!!!\n"<<endl;
            else {
                proc temp = {ins_set[i].proc_id};
                process.push_back(temp);
            }
        }
        else
        {
            // find the process
            if (ins_set[i].action == 'T'){

                //clean the physical address accordingly
                for (int x=0; x<20; x++)
                {
                    if (phy_pages[x].process_id == process[location].proc_id) {
                        phy_pages[x].process_id = -1;
                        phy_pages[x].time = 0;
                        phy_pages[x].dirty = false;
                    }
                }

                //clean the swap!!!
                vector<keep_swap> ::iterator clean_swap = swap.begin();
                for (int x = 0; x < swap.size()+1; x++)
                {
                    if (swap[x].proc_id == process[location].proc_id)
                    {
                        swap.erase(clean_swap);
                        x = 0;
                        clean_swap = swap.begin();
                    }
                    else clean_swap++;
                }

                //• ‘T’ means the process terminated (‘PAGE’ field not present)
                vector<proc>::iterator it = process.begin();
                for (int x =0; x<location; x++) it++;
                process.erase(it);
            }
            else if (ins_set[i].action == 'R'){

                int page_location;
                page_location = find_action_page(process, location, ins_set[i].page);
                if (page_location == -1)
                {
                    //wrong page number
                    //kill the process
                    kill.push_back(process[location].proc_id); // update kill
                    //clean the physical address accordingly
                    for (int x=0; x<20; x++)
                    {
                        if (phy_pages[x].process_id == process[location].proc_id) {
                            phy_pages[x].process_id = -1;
                            phy_pages[x].time = 0;
                            phy_pages[x].dirty = false;
                        }
                    }

                    //clean the swap!!!
                    vector<keep_swap> ::iterator clean_swap = swap.begin();
                    for (int x = 0; x < swap.size()+1; x++)
                    {
                        if (swap[x].proc_id == process[location].proc_id)
                        {
                            swap.erase(clean_swap);
                            x = 0;
                            clean_swap = swap.begin();
                        }
                        else clean_swap++;
                    }

                    vector<proc>::iterator it = process.begin();
                    for (int x =0; x<location; x++) it++;
                    process.erase(it);
                }
                else //find the page location in process
                {
                    int index = process[location].proc_add[page_location].phyadd;

                    if (index == -1 ) // Read a SWAP page
                    {
                        // clean (update) swap first
                        vector<keep_swap> ::iterator clean_swap = swap.begin();
                        for (int x = 0; x < swap.size(); x++)
                        {
                            if (swap[x].proc_id == process[location].proc_id
                                && swap[x].viradd == ins_set[i].page)
                            {
                                swap.erase(clean_swap);
                            }
                            else clean_swap++;
                        }

                        int free_page = check_free(phy_pages);
                        if (free_page != -1)
                        {
                            // there is a free page, and free_page is its index
                            phy_pages[free_page].process_id = process[location].proc_id; // physical address assigned

                            process[location].proc_add[page_location].phyadd = free_page;
                            // process addresses update
                            // physical address
                        }
                        else
                            // there is no free page, need swap
                        {
                            int point = random_point();
                            // swap the point value in physical address
                            // point is the physical address

                            // find the current process of the swap page
                            int object_id = phy_pages[point].process_id;

                            // find the process location
                            int object_locaton = find_proc_location(process, object_id); // process location index

                            // find the page location
                            int object_page = find_swap_page(process,object_locaton, point); // page location index

                            // update the old page owner
                            process[object_locaton].proc_add[object_page].phyadd = -1; // means: swap

                            //put the new (swapped) page for 'A'
                            phy_pages[point].process_id = process[location].proc_id; // physical address assigned
                            phy_pages[point].dirty = false;  // update
                            phy_pages[point].time = 1;  // R update

                            process[location].proc_add[page_location].phyadd = point;

                            //update swap
                            keep_swap temp_swap = {object_id,process[object_locaton].proc_add[object_page].viradd};
                            swap.push_back(temp_swap); // keep track of the swap
                        }
                    }
                    else phy_pages[index].time++;
                }
            }
            else if (ins_set[i].action == 'W'){

                //have NOT consider the swap situation
                // when physical address = -1

                int page_location;
                page_location = find_action_page(process, location, ins_set[i].page);
                if (page_location == -1)
                {
                    //wrong page number
                    //kill the process
                    kill.push_back(process[location].proc_id); // update kill
                    //clean the physical address accordingly
                    for (int x=0; x<20; x++)
                    {
                        if (phy_pages[x].process_id == process[location].proc_id) {
                            phy_pages[x].process_id = -1;
                            phy_pages[x].time = 0;
                            phy_pages[x].dirty = false;
                        }
                    }

                    //clean the swap!!!
                    vector<keep_swap> ::iterator clean_swap = swap.begin();
                    for (int x = 0; x < swap.size()+1; x++)
                    {
                        if (swap[x].proc_id == process[location].proc_id)
                        {
                            swap.erase(clean_swap);
                            x = 0;
                            clean_swap = swap.begin();
                        }
                        else clean_swap++;
                    }

                    vector<proc>::iterator it = process.begin();
                    for (int x =0; x<location; x++) it++;
                    process.erase(it);
                }
                else //find the page location in process
                {
                    int index = process[location].proc_add[page_location].phyadd;

                    if (index == -1 ) // Read a SWAP page
                    {
                        // clean (update) swap first
                        vector<keep_swap> ::iterator clean_swap = swap.begin();
                        for (int x = 0; x < swap.size(); x++)
                        {
                            if (swap[x].proc_id == process[location].proc_id
                                && swap[x].viradd == ins_set[i].page)
                            {
                                swap.erase(clean_swap);
                            }
                            else clean_swap++;
                        }

                        int free_page = check_free(phy_pages);
                        if (free_page != -1)
                        {
                            // there is a free page, and free_page is its index
                            phy_pages[free_page].process_id = process[location].proc_id; // physical address assigned

                            process[location].proc_add[page_location].phyadd = free_page;
                            // process addresses update
                            // physical address
                        }
                        else
                            // there is no free page, need swap
                        {
                            int point = random_point();
                            // swap the point value in physical address
                            // point is the physical address

                            // find the current process of the swap page
                            int object_id = phy_pages[point].process_id;

                            // find the process location
                            int object_locaton = find_proc_location(process, object_id); // process location index

                            // find the page location
                            int object_page = find_swap_page(process,object_locaton, point); // page location index

                            // update the old page owner
                            process[object_locaton].proc_add[object_page].phyadd = -1; // means: swap

                            //put the new (swapped) page for 'A'
                            phy_pages[point].process_id = process[location].proc_id; // physical address assigned
                            phy_pages[point].dirty = true;  // update
                            phy_pages[point].time = 1;  // W update

                            process[location].proc_add[page_location].phyadd = point;

                            //update swap
                            keep_swap temp_swap = {object_id,process[object_locaton].proc_add[object_page].viradd};
                            swap.push_back(temp_swap); // keep track of the swap
                        }
                    }
                    else {
                        phy_pages[index].dirty = true;
                        phy_pages[index].time++;
                    }
                }
            }
            else if (ins_set[i].action == 'F'){

                //have NOT consider the swap situation
                // when physical address = -1
                // ned to clean the swap

                int page_location;
                page_location = find_action_page(process, location, ins_set[i].page);
                if (page_location == -1)
                {
                    //wrong page number
                    //kill the process
                    kill.push_back(process[location].proc_id); // update kill
                    //clean the physical address accordingly
                    for (int x=0; x<20; x++)
                    {
                        if (phy_pages[x].process_id == process[location].proc_id) {
                            phy_pages[x].process_id = -1;
                            phy_pages[x].time = 0;
                            phy_pages[x].dirty = false;
                        }
                    }

                    //clean the swap!!!
                    vector<keep_swap> ::iterator clean_swap = swap.begin();
                    for (int x = 0; x < swap.size()+1; x++)
                    {
                        if (swap[x].proc_id == process[location].proc_id)
                        {
                            swap.erase(clean_swap);
                            x = 0;
                            clean_swap = swap.begin();
                        }
                        else clean_swap++;
                    }

                    vector<proc>::iterator it = process.begin();
                    for (int x =0; x<location; x++) it++;
                    process.erase(it);
                }
                else //find the page location in process
                {
                    int index = process[location].proc_add[page_location].phyadd;

                    if (index == -1 ) // Free the SWAP page
                    {
                        // clean (update) swap
                        vector<keep_swap> ::iterator clean_swap = swap.begin();
                        for (int x = 0; x < swap.size(); x++)
                        {
                            if (swap[x].proc_id == process[location].proc_id
                                && swap[x].viradd == ins_set[i].page)
                            {
                                swap.erase(clean_swap);
                            }
                            else clean_swap++;
                        }
                    }
                    else
                    {
                        // free the physical address first
                        phy_pages[index].process_id = -1; // FREE page
                        phy_pages[index].dirty = false;
                        phy_pages[index].time = 0;
                    }

                    // finally free the virtual and physical address in process
                    vector<address>::iterator it = process[location].proc_add.begin();
                    for (int x =0; x<page_location; x++) it++;
                    process[location].proc_add.erase(it);
                }
            }
            else if (ins_set[i].action == 'A'){
                int page_location;
                page_location = find_action_page(process, location, ins_set[i].page);
                if (page_location != -1)
                {
                    //allocate the same page as previous
                    //kill the process
                    kill.push_back(process[location].proc_id); // update kill
                    //clean the physical address accordingly
                    for (int x=0; x<20; x++)
                    {
                        if (phy_pages[x].process_id == process[location].proc_id) {
                            phy_pages[x].process_id = -1;
                            phy_pages[x].time = 0;
                            phy_pages[x].dirty = false;
                        }
                    }

                    //clean the swap!!!
                    vector<keep_swap> ::iterator clean_swap = swap.begin();
                    for (int x = 0; x < swap.size()+1; x++)
                    {
                        if (swap[x].proc_id == process[location].proc_id)
                        {
                            swap.erase(clean_swap);
                            x = 0;
                            clean_swap = swap.begin();
                        }
                        else clean_swap++;
                    }

                    vector<proc>::iterator it = process.begin();
                    for (int x =0; x<location; x++) it++;
                    process.erase(it);
                }
                else// need to allocate a new page:
                {
                    int free_page = check_free(phy_pages);
                    if (free_page != -1)
                    {
                        // there is a free page, and free_page is its index
                        phy_pages[free_page].process_id = process[location].proc_id; // physical address assigned

                        address temp = {ins_set[i].page, free_page};
                        process[location].proc_add.push_back(temp);
                        // process addresses update
                        // virtual address
                        // physical address
                    }
                    else
                        // there is no free page, need swap
                    {
                        int point = random_point();
                        // swap the point value in physical address
                        // point is the physical address

                        // find the current process of the swap page
                        int object_id = phy_pages[point].process_id;

                        // find the process location
                        int object_locaton = find_proc_location(process, object_id); // process location index

                        // find the page location
                        int object_page = find_swap_page(process,object_locaton, point); // page location index

                        // update the old page owner
                        process[object_locaton].proc_add[object_page].phyadd = -1; // means: swap

                        //put the new (swapped) page for 'A'
                        phy_pages[point].process_id = process[location].proc_id; // physical address assigned
                        phy_pages[point].dirty = false;  // update
                        phy_pages[point].time = 0;  // update

                        address temp = {ins_set[i].page, point};
                        process[location].proc_add.push_back(temp); // process addresses update

                        //update swap
                        keep_swap temp_swap = {object_id,process[object_locaton].proc_add[object_page].viradd};
                        swap.push_back(temp_swap); // keep track of the swap
                    }
                }
            }
            else{
                // wrong action
                // kill the process
                kill.push_back(process[location].proc_id); // update kill
                //clean the physical address accordingly
                for (int x=0; x<20; x++)
                {
                    if (phy_pages[x].process_id == process[location].proc_id) {
                        phy_pages[x].process_id = -1;
                        phy_pages[x].time = 0;
                        phy_pages[x].dirty = false;
                    }
                }

                //clean the swap!!!
                vector<keep_swap> ::iterator clean_swap = swap.begin();
                for (int x = 0; x < swap.size()+1; x++)
                {
                    if (swap[x].proc_id == process[location].proc_id)
                    {
                        swap.erase(clean_swap);
                        x = 0;
                        clean_swap = swap.begin();
                    }
                    else clean_swap++;
                }

                vector<proc>::iterator it = process.begin();
                for (int x =0; x<location; x++) it++;

                process.erase(it);
            }
        }
    } // all done
    // Display the results next
    cout << "\nThere are the results of Random swapping: "<<endl;
    print_result(process, swap, kill, phy_pages);
}

// tool function
int find_proc_location(vector<proc> process, int proc_id)
{
    int proc_size = process.size();
    for (int i=0; i<proc_size; i++)
    {
        if (process[i].proc_id == proc_id) return i;
    }
    return -1;
    // could not find the process
}
int find_action_page(vector<proc> process, int location, int viradd){
    int pagesize = process[location].proc_add.size();
    for (int i = 0; i<pagesize; i++)
    {
        if (process[location].proc_add[i].viradd == viradd) return i;
    }
    return -1;
    // could not find the page
}
int find_swap_page(vector<proc> process, int location, int phyadd){
    int pagesize = process[location].proc_add.size();
    for (int i = 0; i<pagesize; i++)
    {
        if (process[location].proc_add[i].phyadd == phyadd) return i;
    }
    return -1;
    // could not find the page
}
int check_free(physical phy_pages[20]){
    // will return the index of the first free page
    for (int i=0; i<20;i++)
    {
        if (phy_pages[i].process_id == -1) // means: there is a free page
            return i;
    }
    return -1;
}
void print_result(vector<proc> process,vector<keep_swap> swap,vector<int> kill,physical phy_pages[20])
{
    cout<<"\n";
    for (int i = 0; i < process.size(); i++)
    {
        cout << "\nPROCESS " << process[i].proc_id << endl;
        for (int x = 0; x < process[i].proc_add.size(); x++)
        {
            cout <<"\tVirtual \t" << process[i].proc_add[x].viradd << "\t\t"
            << "Physical \t";
            if (process[i].proc_add[x].phyadd == -1) cout<<"Swap" <<endl;
            else cout << process[i].proc_add[x].phyadd << endl;
        }

    } // print process

    cout <<"\n"<<"SWAP"<<endl;
    for (int i = 0; i<swap.size(); i++)
    {
        cout<<"\tProcess \t" << swap[i].proc_id << "\t\t"
        <<"Virtual\t" << swap[i].viradd << endl;

    } // print swap

    cout <<"\n"<<"PHYSICAL"<<endl;
    for (int i = 0; i<20; i++ )
    {
        cout<<"\t"<<i<< "\t\t";
        if (phy_pages[i].process_id == -1) cout << "FREE" << endl;
        else {
            cout << "Process " << phy_pages[i].process_id
            // for debug purpose
            << "\tDirty: ";
            if (phy_pages[i].dirty == true) cout << "Yes";
            else cout << "No";
            cout << "\tAccess Time: "<< phy_pages[i].time
                 <<endl;
        }
    } // print physical

    cout <<"\n"<<"KILL"<<endl;
    for (int i = 0; i<kill.size(); i++)
    {
        cout<<"\tPROCESS " << kill[i] << "\t\t" << "KILLED" << endl;
    } // print kill

    cout<<"\n\n";
    cout<<"DONE\n"<<endl;
}
int random_point(){
    int i;
    srand(time(NULL));
    i = rand() %20;
    return i;
}
int LRU_point(physical phy_pages[20]){
    int min_time = phy_pages[0].time;
    int index = 0;
    for (int i = 1; i < 20; i++)
    {
        if (phy_pages[i].time < min_time) {
            min_time = phy_pages[i].time;
            index = i;
        }
    }
    return index;
}
