#include <iostream>
#include <string.h>
#include <algorithm>
#include <vector>
#include <queue>
#include <math.h>
using namespace std;
typedef struct
{
    char name; // process name
    int index; // order of the process in input file
    int arrival_time;
    int starting_time;
    int service_time;
    int waiting_time;
    int finish_time;
    int turnaround_time;
    float normTurn;
    int remaining_time;
    int current_fb_queue;
    int initial_priority; // used for aging scheduling.
    int current_priority;
    int arrivalToqueue; // the time when the process arrived to priority queue.

} Process; //structure that stores the associated values for each process


// The following structures are used for the priority queues that will be used for the different scheduling techniques.
struct CompareProcessByArrival
{
    bool operator()(Process const& p1, Process const& p2)
    {
        return p1.arrival_time > p2.arrival_time;
    }
};
struct CompareProcessByService
{
    // Used for SPN policy.
    bool operator()(Process const& p1, Process const& p2)
    {
        // If they have the same service time, compare by arrival time.
        if (p1.service_time == p2.service_time)
            return p1.arrival_time > p2.arrival_time;
        return p1.service_time > p2.service_time; //returns true if p1's service time is greater than p2's
    }
};
struct CompareProcessByHRRN
{
    // Used for HRRN policy.
    bool operator()(Process const& p1, Process const& p2)
    {
        // If they have the same HRRN compare by arrival time.
        float p1_ResponseRatio = (float)(p1.waiting_time + p1.service_time)/ p1.service_time;
        float p2_ResponseRatio = (float)(p2.waiting_time + p2.service_time)/ p2.service_time;
        if (p1_ResponseRatio == p2_ResponseRatio)
            return p1.arrival_time > p2.arrival_time;
        return p1_ResponseRatio < p2_ResponseRatio; //returns true if p2's response ratio is greater than p1s
    }
};
struct CompareProcessByRemaining
{
    bool operator()(Process const& p1, Process const& p2)
    {
        // Used for SRT policy
        // If they have the same remaininig time, compare by arrival time.
        if (p1.remaining_time == p2.remaining_time)
            return p1.arrivalToqueue> p2.arrivalToqueue;
        return p1.remaining_time > p2.remaining_time;  //returns true if p1's remaining time is greater than p2's
    }
};

struct CompareProcessByPriority
{
    // Used for aging policy.
    bool operator()(Process const& p1, Process const& p2)
    {
        if (p1.current_priority == p2.current_priority)
            return p1.arrivalToqueue > p2.arrivalToqueue;
        return p1.current_priority < p2.current_priority; //returns true if p2's priority is greater than p1's
    }
};
using namespace std;


//This method is used to print the trace of any non preemptive policy: FCFS, SPN, HRRN.
void printTrace_nonpreemtive(char* policy, Process p[],  int n, int last_instant)
{
    printf("%s", policy);
    int len = strlen(policy);
    int i  = 0;
    while (i < 6-len)
    {
        printf(" ");
        i++;
    }

    for (int i = 0; i <= last_instant; i++)
    {
        printf("%d ", i%10);
    }
    printf("\n");
    for (int i = 0; i < 6; i++)
        printf("-");
    for (int i = 0; i <= last_instant; i++)
    {
        printf("--");
    }

    for (int i = 0; i < n; i++)
    {
        printf("\n%c     |", p[i].name);
        for (int time = 0; time < last_instant; time++)
        {
            if (time >= p[i].starting_time && time < p[i].finish_time)
                printf("*|");
            else if (time >= p[i].arrival_time && time <= p[i].starting_time)
                printf(".|");
            else
                printf(" |");
        }
        printf(" ");
    }
    printf("\n");

    for (int i = 0; i < 6; i++)
        printf("-");
    for (int i = 0; i <= last_instant; i++)
    {
        printf("--");
    }
    printf("\n");
    printf("\n");
}

// This one is used to print the trace for any preemptive policy.
void printTrace_preemtive(char* policy, Process p[],  int n, int last_instant,char* states,int digit)
{
    int space;
    if(!digit)
    {
        printf("%s", policy);
        space = 6;
    }
    else
    {
        printf("%s-%d", policy,digit);
        space = 4;
    }
    int len = strlen(policy);
    int i  = 0;
    while (i < space-len)
    {
        printf(" ");
        i++;
    }
    for (int i = 0; i <= last_instant; i++)
    {
        printf("%d ", i%10);
    }
    printf("\n");
    for (int i = 0; i < 6; i++)
        printf("-");
    for (int i = 0; i <= last_instant; i++)
    {
        printf("--");
    }

    int time;
    for(i=0; i<n; i++)
    {
        printf("\n%c     |", p[i].name);
        for(time=0; time<last_instant; time++)
        {
            printf("%c|",*((states+i*last_instant) + time));
        }
        printf(" ");
    }
    printf("\n");

    for (int i = 0; i < 6; i++)
        printf("-");
    for (int i = 0; i <= last_instant; i++)
    {
        printf("--");
    }
    printf("\n");
    printf("\n");
}

// This method print the statisctics for any scheduling policy.
void printStats(char* policy, Process p[],  int n, int digit)
{
    char c = '|';
    if(!digit)
    {
        printf("%s\n", policy);
    }
    else
    {
        printf("%s-%d\n", policy,digit);
    }
    printf("Process%5c", c);
    for (int i = 0; i < n; i++)
        printf("%3c%3c", p[i].name, c);
    printf("\nArrival    |");
    for (int i = 0; i < n; i++)
    {
        printf("%3d%3c", p[i].arrival_time, c);
    }
    printf("\nService    |");
    for (int i = 0; i < n; i++)
        printf("%3d%3c", p[i].service_time,c);
    printf(" Mean|");
    printf("\nFinish     |");
    for (int i = 0; i < n; i++)
        printf("%3d%3c", p[i].finish_time,c);
    printf("-----|");
    printf("\nTurnaround |");
    float s=0;
    for (int i = 0; i < n; i++)
    {
        printf("%3d%3c", p[i].turnaround_time, c);
        s+=p[i].turnaround_time;
    }
    printf("%5.2f%c", s/n, c);
    s=0;
    printf("\nNormTurn   |");
    for (int i = 0; i < n; i++)
    {
        printf("%5.2f%c", p[i].normTurn,c);
        s+=p[i].normTurn;
    }
    printf("%5.2f%c\n", s/n, c);
    printf("\n");
}

void HRRN(Process p[], int numOfProcesses, int last_instant, int mode)
{
    priority_queue<Process, vector<Process>,CompareProcessByHRRN> readyQueue;
    priority_queue<Process, vector<Process>,CompareProcessByHRRN> tempQueue;
    priority_queue<Process, vector<Process>,CompareProcessByArrival> firstComeQueue;
    for (int i = 0; i < numOfProcesses; i++)
    {
        firstComeQueue.push(p[i]); //first all processes are arranged according to arrival
    }
    int current, next;
    bool first = true;
    Process temp;
    int time = 0;
    while(time < last_instant)
    {
        while (true)
        {
            if (firstComeQueue.empty())
                break; //No processes
            temp = firstComeQueue.top(); //process that arrived first
            if (temp.arrival_time <= time)
            {
                readyQueue.push(temp); //add process to ready queue(arranged by response ratio) if it's time has come and remove from pq
                firstComeQueue.pop();
            }
            else
                break;
        }
        int next_time = -1;
        while (!readyQueue.empty())
        {
            next_time = time;
            while(!readyQueue.empty())
            {
                temp = readyQueue.top(); //the one with highest response ratio from the ready processes
                readyQueue.pop();
                temp.waiting_time = time - temp.arrival_time; //adjust waiting time
                tempQueue.push(temp); //add to a temporary queue

            }
            //after adjusting waiting times, return to ready queue
            while(!tempQueue.empty())
            {
                temp = tempQueue.top();
                tempQueue.pop();
                readyQueue.push(temp);
            }
            if (first == true) //it's the first
            {
                temp = readyQueue.top();
                current = temp.index;
                readyQueue.pop();
                p[current].waiting_time = 0; //hasn't waited
                p[current].starting_time = p[current].arrival_time; //start time is same as arrival time due to no wait
                p[current].finish_time = p[current].service_time; // because non preemtive
                p[current].turnaround_time = p[current].service_time + p[current].waiting_time;
                p[current].normTurn = 1; //because waiting time = 0
                first = false;
            }
            else
            {
                temp = readyQueue.top();
                next = temp.index;
                readyQueue.pop();
                p[next].starting_time = time;//starts after the previous finishes
                p[next].finish_time = time + p[next].service_time;
                p[next].waiting_time = p[current].finish_time - p[next].arrival_time; // the time it was unable to be executed
                p[next].turnaround_time = p[next].service_time + p[next].waiting_time;
                p[next].normTurn = (float) (p[next].turnaround_time) / p[next].service_time;
                current = next;
            }
            time += p[current].service_time; //update time to start next
        }
        if (next_time == -1)
            time++;
    }

    char *policy = (char*)malloc(5);
    strcpy(policy, "HRRN");
    if (mode == 1)
        printStats(policy, p, numOfProcesses,0);
    else
        printTrace_nonpreemtive(policy, p, numOfProcesses, last_instant);

}

void SPN(Process p[], int numOfProcesses, int last_instant, int mode)
{
    priority_queue<Process, vector<Process>,CompareProcessByService> readyQueue;//min heap for processes according to service time
    priority_queue<Process, vector<Process>,CompareProcessByArrival> firstComeQueue; //min heap for processes according to arrival time
    for (int i = 0; i < numOfProcesses; i++)
    {
        firstComeQueue.push(p[i]); //arrange processes according to arrival time
    }
    int current, next;
    bool first = true;
    Process temp;
    int time = 0;
    int next_time;
    while(time < last_instant)
    {
        while (true)
        {
            if (firstComeQueue.empty()) //No processes
                break;
            temp = firstComeQueue.top();
            if (temp.arrival_time <= time)
            {
                readyQueue.push(temp); //Process has already arrived so move to Ready
                firstComeQueue.pop();
            }
            else
                break;
        }
        next_time = -1;
        while (!readyQueue.empty())
        {
            next_time = time;
            if (first == true) //first process
            {
                temp = readyQueue.top();
                current = temp.index;
                readyQueue.pop();
                p[current].waiting_time = 0; //hasnt waited
                p[current].starting_time = p[current].arrival_time; //start time is same as arrival time due to no wait
                p[current].finish_time = p[current].service_time; //becasue non preemtive
                p[current].turnaround_time = p[current].service_time + p[current].waiting_time;
                p[current].normTurn = 1; //as waiting time = 0
                first = false;
            }
            else
            {
                temp = readyQueue.top();
                next = temp.index;
                readyQueue.pop();
                p[next].starting_time = time; //starts after the previous finishes
                p[next].finish_time = time + p[next].service_time;
                p[next].waiting_time = p[current].finish_time - p[next].arrival_time; // time it was unable to execute due to previous one
                p[next].turnaround_time = p[next].service_time + p[next].waiting_time;
                p[next].normTurn = (float) (p[next].turnaround_time) / p[next].service_time;
                current = next;
            }
            time += p[current].service_time; //update next time
        }
        if (next_time == -1)
                time++;
    }

    char *policy = (char*)malloc(5);
    strcpy(policy, "SPN");
    if (mode == 1)
        printStats(policy, p, numOfProcesses,0);
    else
        printTrace_nonpreemtive(policy, p, numOfProcesses, last_instant);

}

void FCFS(Process p[], int numOfProcesses, int last_instant, int mode)
{
    priority_queue<Process, vector<Process>,CompareProcessByArrival> pq; //min heap by arrival time
    int time = 0;
    for (int i = 0; i < numOfProcesses; i++)
    {
        pq.push(p[i]);
    }
    Process temp = pq.top();
    int current = temp.index;
    pq.pop();
    p[current].waiting_time = 0; //first process
    p[current].starting_time = p[current].arrival_time; //No waiting
    p[current].finish_time = p[current].service_time;
    time += p[current].service_time;
    p[current].turnaround_time = p[current].service_time + p[current].waiting_time;
    p[current].normTurn = 1; //Because no waiting
    while (!pq.empty()) // for all remaining processes
    {
        temp = pq.top();
        int next = temp.index;
        if (p[next].arrival_time > time)
            {
                time++;
            }
        else
        { pq.pop();
        p[next].starting_time = time;
        p[next].finish_time = time + p[next].service_time;
        p[next].waiting_time = p[current].finish_time - p[next].arrival_time;
        p[next].turnaround_time = p[next].service_time + p[next].waiting_time;
        p[next].normTurn = (float) (p[next].turnaround_time) / p[next].service_time;
        current = next;
        time += p[current].service_time;
        }

    }
    char *policy = (char*)malloc(5);
    strcpy(policy, "FCFS");
    if (mode == 1)
        printStats(policy, p, numOfProcesses,0);
    else
        printTrace_nonpreemtive(policy, p, numOfProcesses, last_instant);
}

void SRT(Process p[], int numOfProcesses, int last_instant, int mode)
{
    int i,j;
    int id = 0;
    Process temp;
    char state[numOfProcesses][last_instant]; //2d array to trace state of each process at each time
    priority_queue<Process, vector<Process>,CompareProcessByArrival> pq;//min heap according to arrival time
    priority_queue<Process, vector<Process>,CompareProcessByRemaining> ready;//min heap according to remaining time
    queue <Process> tempQ;


    for(i=0; i<numOfProcesses; i++)
    {
        for(j=0; j<last_instant; j++)
        {
            state[i][j]=' ';
        }
    }
    for (int i = 0; i < numOfProcesses; i++)
    {
        p[i].starting_time = -1; //all processes havent started yet
        pq.push(p[i]);
    }
    int time;
    bool remaining = false;
    int to_be_pushed;
    int pq_size;
    for(time=0; time<last_instant; time++)
    {
        pq_size = pq.size();
        for(i=0; i<pq_size; i++)
        {
            temp=pq.top();

            if(p[temp.index].arrival_time==time) //check for processes that have arrived and are ready
            {
                pq.pop();
                p[temp.index].arrivalToqueue = id;
                id++;
                ready.push(p[temp.index]);
                state[temp.index][time]='.'; //at first the process is assumed to be blocked
            }
        }
        if(remaining) //if a process hasn't finished yet, push it to ready queue
        {
            p[to_be_pushed].arrivalToqueue = id;
            id++;
            ready.push(p[to_be_pushed]);
            remaining=false;
        }
        if(!ready.empty())
        {
            Process executing = ready.top();
            ready.pop();
            if(executing.starting_time==-1) //first time to enter ready queue
            {
                p[executing.index].starting_time=time;
                p[executing.index].waiting_time = p[executing.index].starting_time- p[executing.index].arrival_time;
            }
            if(executing.remaining_time>1)
            {
                p[executing.index].remaining_time=p[executing.index].remaining_time-1;
                remaining= true;  // still has to finish after this time quantum
            }
            else
            {
                //completed execution
                p[executing.index].finish_time = time + 1;
                p[executing.index].turnaround_time = p[executing.index].finish_time-p[executing.index].arrival_time;
                p[executing.index].normTurn = (float)(p[executing.index].turnaround_time)/p[executing.index].service_time;
                p[executing.index].remaining_time = 0;
            }

            state[executing.index][time]='*'; //currently executing
            if(!ready.empty())
            {
                while (!ready.empty())
                {
                    temp = ready.top();
                    ready.pop();
                    tempQ.push(temp);
                    state[temp.index][time]='.'; //all other processes in ready queue are blocked
                }
                while (!tempQ.empty())
                {
                    temp = tempQ.front();
                    tempQ.pop();
                    ready.push(temp);
                }
            }
            if(remaining)
            {
                to_be_pushed = executing.index;
            }
        }
    }
    char *policy = (char*)malloc(5);
    strcpy(policy, "SRT");
    if (mode == 1)
        printStats(policy, p, numOfProcesses,0);
    else
        printTrace_preemtive(policy, p, numOfProcesses, last_instant,(char *)state,0);
}

void RR(Process p[], int numOfProcesses, int last_instant, int mode, int quantum)
{
    int i,j;
    priority_queue<Process, vector<Process>,CompareProcessByArrival> pq; // PQ which prioritizes the processes by their arrival time.
    queue <Process> ready; // A queue containing the processes that are ready to be executed.
    char state[numOfProcesses][last_instant]; // 2d array to trace the state of each process.

    for(i=0; i<numOfProcesses; i++)
    {
        for(j=0; j<last_instant; j++)
        {
            state[i][j]=' ';
        }
    }
    for (int i = 0; i < numOfProcesses; i++)
    {
        p[i].starting_time = -1; // All processes have not started yet.
        pq.push(p[i]);
    }
    int time = 0;
    int next_time;
    Process temp;
    bool remaining = false; // will be used to indicate whether a process needs to be pushed back to the ready queue after it has executed for a quantum of time.
    Process to_be_pushed;
    int pq_size;
    while(time < last_instant)
    {
        pq_size = pq.size();
        for(i = 0; i < pq_size; i++)
        {
            temp = pq.top(); // The process that arrived first.

            if(p[temp.index].arrival_time <= time) // Check if this process has already arrived.
            {
                pq.pop(); // Remove it from the pq and add it to ready queue.
                ready.push(temp);

                for(j=temp.arrival_time; j<time; j++)
                {
                    state[temp.index][j]='.'; // Set the state of this process as waiting from its arrival time to the current time.
                }
            }
        }
        if(remaining)
        {
            ready.push(to_be_pushed);
            remaining=false;
        }
        if (pq.empty() && ready.empty())
            break;
        if(!ready.empty())
        {
            Process executing = ready.front(); // The process to be executed.
            ready.pop();
            if(executing.starting_time==-1) // This is its first time to be selected.
            {
                // Update its starting and waiting time.
                p[executing.index].starting_time=time;
                p[executing.index].waiting_time = p[executing.index].starting_time- p[executing.index].arrival_time;
            }

            // If the process will need another quantum to continue its execution.
            if(executing.remaining_time>quantum)
            {
                next_time= time + quantum; // Because this process will be on the processor for all the quantum.
                p[executing.index].remaining_time=p[executing.index].remaining_time-quantum;
                remaining= true;
            }
            // The process will finish its execution during this quantum.
            else
            {
                next_time = time + p[executing.index].remaining_time; // Update the time by adding the period of time the process executed.
                p[executing.index].finish_time=time+p[executing.index].remaining_time;
                p[executing.index].turnaround_time= p[executing.index].finish_time-p[executing.index].arrival_time;
                p[executing.index].normTurn= (float)(p[executing.index].turnaround_time)/p[executing.index].service_time;
                p[executing.index].remaining_time = 0;
            }
            int imax = next_time;
            if (next_time > last_instant)
                imax = last_instant;
            for(i=time; i<imax; i++)
            {
                state[executing.index][i]='*'; // process executing.
                if(!ready.empty())
                {
                    for(j=0; j<ready.size(); j++)
                    {
                        temp = ready.front();
                        state[temp.index][i]='.'; // During the period of time where a process executed, all other processes in ready queue were waiting.
                        ready.pop();
                        ready.push(p[temp.index]);
                    }
                }
            }
            if(remaining)
            {
                to_be_pushed = p[executing.index]; // push the process again to the ready queue if it didn't complete its execution.
            }
            time =  next_time;
        }
        else
            time++;

    }
    char *policy = (char*)malloc(5);
    strcpy(policy, "RR");
    if (mode == 1)
        printStats(policy, p, numOfProcesses,quantum);
    else
        printTrace_preemtive(policy, p, numOfProcesses, last_instant,(char *)state,quantum);
}

void Aging(Process p[], int numOfProcesses, int last_instant, int mode, int quantum)
{
    int i,j;
    int id = 0;
    priority_queue<Process, vector<Process>,CompareProcessByArrival> pq; // PQ which prioritizes the processes by their arrival time.
    priority_queue<Process, vector<Process>,CompareProcessByPriority> ready; // A queue containing the processes that are ready to b/e executed.
    queue<Process> tempQ;
    char state[numOfProcesses][last_instant]; // 2d array to trace the state of each process.

    for (i = 0; i < numOfProcesses; i++)
    {
        p[i].initial_priority = p[i].service_time;
        p[i].current_priority = p[i].initial_priority;
        pq.push(p[i]);
    }
    for(i=0; i<numOfProcesses; i++)
    {
        for(j=0; j<last_instant; j++)
        {
            state[i][j]=' ';
        }
    }
    Process temp;
    int to_be_pushed;
    bool preemtion_occured = false;
    int time;
    for(time = 0; time < last_instant; time += quantum)
    {
        for(i = 0; i < pq.size(); i++)
        {
            temp = pq.top(); // The process that arrived first.

            if(p[temp.index].arrival_time <= time) // Check if this process has already arrived.
            {
                pq.pop(); // Remove it from the pq and add it to ready queue.
                p[temp.index].arrivalToqueue = id;
                id++;
                ready.push(p[temp.index]);
                for(j=temp.arrival_time; j<time; j++)
                {
                    state[temp.index][j]='.'; // Set the state of this process as waiting from its arrival time to the current time.
                }
            }
        }
        int numOfReady = ready.size();
        // Increment the current priority of each process in the ready queue by 1.
        while (!ready.empty())
        {
            temp = ready.top();
            ready.pop();
            p[temp.index].current_priority += 1;
            tempQ.push(p[temp.index]);
        }
        while (!tempQ.empty())
        {
            temp = tempQ.front();
            tempQ.pop();
            ready.push(p[temp.index]);
        }
        // Preempt the currently running process.
        if (preemtion_occured)
        {
            p[to_be_pushed].arrivalToqueue = id; // Update the id of the process so the priority queue knows when it arrived.
            id++;
            ready.push(p[to_be_pushed]);
        }
        preemtion_occured=false;
        if(!ready.empty())
        {
            Process executing = ready.top(); // The process to be executed.
            ready.pop();
            int zmax = time+quantum;
            if (time+quantum > last_instant)
                zmax = last_instant;
            for(int z=time; z<zmax; z++)
            {
                state[executing.index][z]='*'; // process executing.
                if(!ready.empty())
                {
                    while (!ready.empty())
                    {
                        temp = ready.top();
                        ready.pop();
                        tempQ.push(p[temp.index]);
                        state[temp.index][z]='.'; // During the period of time where a process executed, all other processes in ready queue were waiting.
                    }
                    while (!tempQ.empty())
                    {
                        temp = tempQ.front();
                        tempQ.pop();
                        ready.push(p[temp.index]);
                    }
                }
            }
            p[executing.index].current_priority = p[executing.index].initial_priority;
            preemtion_occured = true;
            to_be_pushed = executing.index;
        }
    }
    char *policy = (char*)malloc(10);
    strcpy(policy, "Aging");
    printTrace_preemtive(policy, p, numOfProcesses, last_instant,(char *)state,0);
}

void FB_1(Process p[], int numOfProcesses, int last_instant, int mode)
{
    int i,j;

    char state[numOfProcesses][last_instant]; // 2d array to store the state of each process at each instant.
    priority_queue<Process, vector<Process>,CompareProcessByArrival> pq; // pq that prioritize processes by arrival time.
    int n = 5; // maximum number of ready queues.
    queue <Process> ready_queues[n];
    for(i=0; i<numOfProcesses; i++)
    {
        for(j=0; j<last_instant; j++)
        {
            state[i][j]=' ';
        }
    }
    for (int i = 0; i < numOfProcesses; i++)
    {
        p[i].starting_time = -1;
        pq.push(p[i]);
    }
    int time = 0;
    Process temp;
    int queues_index;
    int other_index;
    bool remaining = false;
    bool all_empty = true;
    Process to_be_pushed;
    int prev_insertion;
    int pq_size;
    for(time=0; time<last_instant; time++)
    {
        pq_size = pq.size();
        for(i=0; i<pq_size; i++)
        {
            // check if any of the processes has arrived.
            temp=pq.top();

            if(p[temp.index].arrival_time==time)
            {
                pq.pop();
                ready_queues[0].push(temp); // push it to the first ready queue.
                state[temp.index][time]='.';
            }
        }
        for(queues_index=0; queues_index < n; queues_index++)
        {
            if(!ready_queues[queues_index].empty())
            {
                all_empty =false; // all ready queues are empty.
                break;
            }
        }
        if(remaining) // if the preempted process still has remaining time, it will be push back to the appropriate ready queue.
        {
            if(all_empty || prev_insertion == n-1)
            {
                ready_queues[prev_insertion].push(to_be_pushed); // push the process back to the same ready queue if all queues are empty or if it was in the last queue.
            }
            else
            {
                ready_queues[prev_insertion + 1].push(to_be_pushed); // push the process back to the next ready queue.
            }
            remaining= false;
        }
        for(queues_index = 0; queues_index < n; queues_index++)
        {
            // Selecting the next process to be executed.
            if(!ready_queues[queues_index].empty())
            {
                Process executing = ready_queues[queues_index].front();
                ready_queues[queues_index].pop();
                if(executing.starting_time==-1)
                {
                    p[executing.index].starting_time=time;
                    p[executing.index].waiting_time = p[executing.index].starting_time - p[executing.index].arrival_time;
                }
                if(executing.remaining_time>1)
                {
                    p[executing.index].remaining_time=p[executing.index].remaining_time-1;
                    remaining= true;
                }
                else
                {
                    p[executing.index].finish_time=time+1;
                    p[executing.index].turnaround_time= p[executing.index].finish_time-p[executing.index].arrival_time;
                    p[executing.index].normTurn= (float)(p[executing.index].turnaround_time)/p[executing.index].service_time;
                    p[executing.index].remaining_time=0;
                }

                state[executing.index][time]='*';
                for(other_index=0; other_index<n; other_index++)
                {
                    if(!ready_queues[other_index].empty())
                    {
                        for(j=0; j<ready_queues[other_index].size(); j++)
                        {
                            temp = ready_queues[other_index].front();
                            state[temp.index][time]='.';
                            ready_queues[other_index].pop();
                            ready_queues[other_index].push(p[temp.index]);

                        }
                    }
                }
                if(remaining)
                {
                    to_be_pushed=p[executing.index];
                    prev_insertion = queues_index;
                }
                break;
            }
        }
        all_empty = true;
    }
    char *policy = (char*)malloc(5);
    strcpy(policy, "FB");
    if (mode == 1)
        printStats(policy, p, numOfProcesses,1);
    else
        printTrace_preemtive(policy, p, numOfProcesses, last_instant,(char *)state,1);
}

void FB_2i(Process p[], int numOfProcesses, int last_instant, int mode)
{
    int i,j;

    char state[numOfProcesses][last_instant]; // 2d array to store the state of each process at each instant.
    priority_queue<Process, vector<Process>,CompareProcessByArrival> pq; // pq that prioritize processes by arrival time.
    int n = 5; // maximum number of ready queues.
    queue <Process> ready_queues[n];
    int quantums[] = {1, 2, 4, 8, 16};
    for(i=0; i<numOfProcesses; i++)
    {
        for(j=0; j < last_instant; j++)
        {
            state[i][j]=' ';
        }
    }
    for (int i = 0; i < numOfProcesses; i++)
    {
        p[i].starting_time = -1;
        pq.push(p[i]);
    }
    int time = 0;
    int next_time;
    Process temp;
    int queues_index;
    int other_index;
    bool remaining = false;
    bool all_empty = true;
    Process to_be_pushed;
    int prev_insertion;
    int pq_size;
    while(time < last_instant)
    {
        pq_size = pq.size();
        for(i=0; i<pq_size; i++)
        {
            // check if any of the processes has arrived.
            temp=pq.top();

            if(p[temp.index].arrival_time<=time)
            {
                pq.pop();
                ready_queues[0].push(temp); // push it to the first ready queue.
                for(j=temp.arrival_time; j<time; j++)
                {
                    state[temp.index][j]='.'; // Set the state of this process as waiting from its arrival time to the current time.
                }
            }
        }
        for(queues_index=0; queues_index < n; queues_index++)
        {
            if(!ready_queues[queues_index].empty())
            {
                all_empty =false; // all ready queues are empty.
                break;
            }
        }
        if(remaining) // if the preempted process still has remaining time, it will be push back to the appropriate ready queue.
        {
            if(all_empty || prev_insertion == n-1)
            {
                ready_queues[prev_insertion].push(to_be_pushed); // push the process back to the same ready queue if all queues are empty or if it was in the last queue.
            }
            else
            {
                ready_queues[prev_insertion + 1].push(to_be_pushed); // push the process back to the next ready queue.
            }
            remaining= false;
        }
        for(queues_index = 0; queues_index < n; queues_index++)
        {
            // Selecting the next process to be executed.
            if(!ready_queues[queues_index].empty())
            {
                Process executing = ready_queues[queues_index].front();
                ready_queues[queues_index].pop();
                if(executing.starting_time==-1)
                {
                    p[executing.index].starting_time = time;
                    p[executing.index].waiting_time = p[executing.index].starting_time - p[executing.index].arrival_time;
                }
                if(executing.remaining_time > quantums[queues_index])
                {
                    next_time= time + quantums[queues_index]; // Because this process will be on the processor for all the quantum.
                    p[executing.index].remaining_time=p[executing.index].remaining_time - quantums[queues_index];
                    remaining= true;
                }
                else
                {
                    next_time = time + p[executing.index].remaining_time; // Update the time by adding the period of time the process executed.
                    p[executing.index].finish_time = time + p[executing.index].remaining_time;
                    p[executing.index].turnaround_time= p[executing.index].finish_time-p[executing.index].arrival_time;
                    p[executing.index].normTurn= (float)(p[executing.index].turnaround_time)/p[executing.index].service_time;
                    p[executing.index].remaining_time = 0;
                }
                int imax = next_time;
                if (next_time > last_instant)
                    imax = last_instant;
                for(i=time; i<imax; i++)
                {
                    state[executing.index][i]='*'; // process executing.
                    for(other_index=0; other_index<n; other_index++)
                    {
                        if(!ready_queues[other_index].empty())
                        {
                            for(j=0; j<ready_queues[other_index].size(); j++)
                            {
                                temp = ready_queues[other_index].front();
                                state[temp.index][i]='.';
                                ready_queues[other_index].pop();
                                ready_queues[other_index].push(p[temp.index]);
                            }
                        }
                    }
                }
                if(remaining)
                {
                    to_be_pushed=p[executing.index];
                    prev_insertion = queues_index;
                }
                break;
            }
        }
        if (next_time != time)
            time =  next_time;
        else
            time++;
        all_empty = true;
    }
    char *policy = (char*)malloc(10);
    strcpy(policy, "FB-2i");
    if (mode == 1)
        printStats(policy, p, numOfProcesses,0);
    else
        printTrace_preemtive(policy, p, numOfProcesses, last_instant,(char *)state,0);
}

int main()
{
    char *buffer;
    int mode; // 0 means trace, 1 means stat;
    int last_instant;
    int numOfProcesses;
    int numberOfSchedulings = 0;
    vector <int> scheduling_policies;
    size_t len = 0;

    //Read the output format required.
    getline(&buffer, &len, stdin);

    if (strcasecmp(buffer, "trace\n") == 0)
        mode = 0;
    else
        mode = 1;


    //Read Scheduling Policies.
    getline(&buffer, &len, stdin);

    char* tok = strtok(buffer, ",");
    int i =0;
    while (tok)
    {
        if (strlen(tok) > 1 && *(tok+1) == '-')
        {
            scheduling_policies.push_back(*tok - 48);
            scheduling_policies.push_back(*(tok+2) - 48);
            i++;
        }
        else
            scheduling_policies.push_back(atoi(tok));
        numberOfSchedulings++;
        tok = strtok(NULL, ",");
        i++;
    }

    // Read the last instant.
    getline(&buffer, &len, stdin);
    last_instant = atoi(buffer);
    // Read the number of processes.
    getline(&buffer, &len, stdin);
    numOfProcesses = atoi(buffer);
    Process p[numOfProcesses];

    int c = 0;

    while (c < numOfProcesses)
    {
        len = 0;
        char *line;
        getline(&line, &len, stdin);
        char *tok=strtok(line,",");
        p[c].name = *tok;
        tok=strtok(NULL,",");
        p[c].arrival_time=atoi(tok);
        tok=strtok(NULL,"");
        p[c].service_time=atoi(tok);
        p[c].remaining_time= p[c].service_time;
        p[c].index = c;
        c++;
    }
    for (int i = 0; i < scheduling_policies.size(); i++)
    {
        Process copiedProcesses[numOfProcesses];
        for (int i = 0; i < numOfProcesses; i++)
        {
            copiedProcesses[i] = p[i];
        }
        int policy = scheduling_policies[i];
        switch(policy)
        {
        case 1:
            FCFS(copiedProcesses, numOfProcesses,  last_instant, mode);
            break;
        case 2:
            RR(copiedProcesses, numOfProcesses,  last_instant, mode,scheduling_policies[i+1]);
            i++;
            break;
        case 3:
            SPN(copiedProcesses, numOfProcesses, last_instant, mode);
            break;
        case 4:
            SRT(copiedProcesses, numOfProcesses, last_instant, mode);
            break;
        case 5:
            HRRN(copiedProcesses, numOfProcesses, last_instant, mode);
            break;
        case 6:
            FB_1(copiedProcesses, numOfProcesses,  last_instant, mode);
            break;
        case 7:
            FB_2i(copiedProcesses, numOfProcesses,  last_instant, mode);
            break;
        case 8:
            Aging(copiedProcesses, numOfProcesses,  last_instant, 0,scheduling_policies[i+1]);
            i++;
            break;
        }
    }
    return 0;
}
