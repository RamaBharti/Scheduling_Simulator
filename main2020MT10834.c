#include <stdio.h>
#include <stdlib.h>
#include <string.h> //for memcpy
#include <float.h>

struct Process {
    char PID[10];
    double ArrivalTime;
    double JobTime;
    double StartTime;
    double EndTime;
    double ResponseTime;
    double TurnaroundTime; //STRF
    double prioroty;
    double executionTime;
};

void createGanttChart(struct Process *processes, int numProcesses) {
    printf("Gantt Chart:\n");

    // Determine the total execution time
    double totalTime = 0;
    for (int i = 0; i < numProcesses; i++) {
        if (processes[i].EndTime > totalTime) {
            totalTime = processes[i].EndTime;
        }
    }

    // Allocate memory for the Gantt chart
    char **ganttChart = (char **)malloc(numProcesses * sizeof(char *));
    for (int i = 0; i < numProcesses; i++) {
        ganttChart[i] = (char *)malloc((totalTime + 1) * sizeof(char));
    }

    // Initialize the Gantt chart grid
    for (int i = 0; i < numProcesses; i++) {
        for (int j = 0; j <= totalTime; j++) {
            ganttChart[i][j] = ' ';
        }
    }

    // Fill in the Gantt chart with process names
    for (int i = 0; i < numProcesses; i++) {
        double startTime = processes[i].StartTime;
        double endTime = processes[i].EndTime;
        for (int t = (int)startTime; t < (int)endTime; t++) {
        ganttChart[i][t] = '|'; // Use '|' to represent process execution
        }
    }


    // Print the Gantt chart
    printf("Time |");
    for (double t = 0; t <= totalTime; t++) {
        printf("%.2lf|", t);
    }
    printf("\n");

    for (int i = 0; i < numProcesses; i++) {
        printf("%4s |", processes[i].PID);
        for (double t = 0; t <= totalTime; t++) {
            printf("%c|", ganttChart[i][(int)t]);
        }
        printf("\n");
    }

    // Free dynamically allocated memory
    for (int i = 0; i < numProcesses; i++) {
        free(ganttChart[i]);
    }
    free(ganttChart);
}

void runFCFS(struct Process *processes, int numProcesses) {
    // Sorting processes based on arrival time
    for (int i = 0; i < numProcesses - 1; i++) {
        for (int j = 0; j < numProcesses - i - 1; j++) {
            if (processes[j].ArrivalTime > processes[j + 1].ArrivalTime) {
                struct Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }

    // FCFS algorithm
    double currentTime = 0;

    for (int i = 0; i < numProcesses; i++) {
        if (processes[i].ArrivalTime > currentTime) {
            currentTime = processes[i].ArrivalTime;
        }

        processes[i].StartTime = currentTime;
        processes[i].EndTime = currentTime + processes[i].JobTime;
        currentTime += processes[i].JobTime;
    }
}

void runRoundRobin(struct Process *processes, int numProcesses, double timeSlice, FILE *outputFile) {
    int completed[numProcesses];
    memset(completed, 0, sizeof(completed));

    double currentTime = 0;

    while (1) { // Infinite loop to keep scheduling until all processes are completed
        int allProcessesCompleted = 1; // Assume all processes are completed
        
        for (int i = 0; i < numProcesses; i++) {
            if (!completed[i] && processes[i].ArrivalTime <= currentTime) {

                processes[i].ResponseTime = currentTime - processes[i].ArrivalTime;

                double executeTime = (processes[i].JobTime <= timeSlice) ? processes[i].JobTime : timeSlice;
                processes[i].StartTime = currentTime;
                currentTime += executeTime;
                processes[i].JobTime -= executeTime;

                if (processes[i].JobTime <= 0) {
                    completed[i] = 1;
                    processes[i].EndTime = currentTime; // Update completion time

                }  
                // Append the execution history for this process
                fprintf(outputFile, "%s %f %f ", processes[i].PID, processes[i].StartTime, currentTime);
            }
            
            // Check if all processes are completed
            if (!completed[i]) {
                allProcessesCompleted = 0;
            }
        }

        if (allProcessesCompleted) {
            break; // Exit the loop if all processes are completed
        }
    }
    fprintf(outputFile, "\n");
}

void runSJF(struct Process *processes, int numProcesses, FILE *outputFile) {
    double currentTime=0;
    double remainingTime[numProcesses];
    double completed[numProcesses];
    double totalCompleted=0;
    // Initialize arrays
    for (int i = 0; i < numProcesses; i++) {
        remainingTime[i] = processes[i].JobTime;
        completed[i] = 0;
    }

    while(totalCompleted < numProcesses){
        int shortestJobIndex=-1;
        double shortestJobTime=DBL_MAX;

        //find the shortest job that has arrived
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].ArrivalTime <= currentTime && !completed[i] && processes[i].JobTime < shortestJobTime) {
                shortestJobIndex = i;
                shortestJobTime = processes[i].JobTime;
            }
        }
        if (shortestJobIndex == -1) {
            // No job has arrived yet, increment time
            currentTime++;
        }else{
           //execute the shortest job
           fprintf(outputFile, "%s %f %f ", processes[shortestJobIndex].PID, currentTime, currentTime+shortestJobTime);

           currentTime+=shortestJobTime;
           remainingTime[shortestJobIndex]=0;
           completed[shortestJobIndex]=1;

           //calculate turnaround time and response time
           processes[shortestJobIndex].TurnaroundTime=currentTime-processes[shortestJobIndex].ArrivalTime;
           processes[shortestJobIndex].ResponseTime=currentTime-processes[shortestJobIndex].ArrivalTime-shortestJobTime;

           totalCompleted++;
        }
    }
    fprintf(outputFile, "\n");
}

// Comparator function for sorting processes based on StartTime
int compareByStartTime(const void* a, const void* b) {
    return ((struct Process*)a)->StartTime - ((struct Process*)b)->StartTime;
}
void runSRTF(struct Process *processes, int numProcesses) {
    double job_remaining[100];
    int is_completed[100];
    memset(is_completed, 0, sizeof(is_completed));

    for (int i = 0; i < numProcesses; i++) {
        job_remaining[i] = processes[i].JobTime;
    }

    double currentTime = 0;
    double completed = 0;
    double prev = 0;

    while (completed != numProcesses) {
        int shortestRemainingIndex = -1;
        double shortestRemainingTime = DBL_MAX;
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].ArrivalTime <= currentTime && !is_completed[i]) {
                if (job_remaining[i] < shortestRemainingTime) {
                    shortestRemainingTime = job_remaining[i];
                    shortestRemainingIndex = i;
                }
                if (job_remaining[i] == shortestRemainingTime) {
                    if (processes[i].ArrivalTime < processes[shortestRemainingIndex].ArrivalTime) {
                        shortestRemainingTime = job_remaining[i];
                        shortestRemainingIndex = i;
                    }
                }
            }
        }

        if (shortestRemainingIndex != -1) {
            if (job_remaining[shortestRemainingIndex] == processes[shortestRemainingIndex].JobTime) {
                processes[shortestRemainingIndex].StartTime = currentTime;
            }
            job_remaining[shortestRemainingIndex] -= 1;
            currentTime++;
            prev = currentTime;

            if (job_remaining[shortestRemainingIndex] == 0) {
                processes[shortestRemainingIndex].EndTime = currentTime;
                processes[shortestRemainingIndex].TurnaroundTime = processes[shortestRemainingIndex].EndTime - processes[shortestRemainingIndex].ArrivalTime;
                processes[shortestRemainingIndex].ResponseTime = processes[shortestRemainingIndex].StartTime - processes[shortestRemainingIndex].ArrivalTime;

                is_completed[shortestRemainingIndex] = 1;
                completed++;
            }
        } else {
            currentTime++;
        }
    }
}

// //MLFQ
// struct Queue{
//     Process *queue;
//     double front;
//     double rear;
//     int capacity;
// };
// // creating and initializing three priority queues
// Queue Q1, Q2, Q3;

// void initializeQueues(int capacity) {
//     Q1.queue = (Process *)malloc(sizeof(Process) * capacity);
//     Q2.queue = (Process *)malloc(sizeof(Process) * capacity);
//     Q3.queue = (Process *)malloc(sizeof(Process) * capacity);
//     Q1.capacity = Q2.capacity = Q3.capacity = capacity;
//     Q1.front = Q1.rear = Q2.front = Q2.rear = Q3.front = Q3.rear = -1;
// }

// // Functions to enqueue and dequeue processes in each queue
// void enqueue(Queue *q, Process process) {
//     if (q->rear == q->capacity - 1) {
//         //printf("Queue is full.\n");
//         return;
//     }
//     if (q->front == -1) {
//         q->front = 0;
//     }
//     q->rear++;
//     q->queue[q->rear] = process;
// }

// Process dequeue(Queue *q) {
//     Process process = q->queue[q->front];
//     if (q->front == q->rear) {
//         q->front = q->rear = -1;
//     } else {
//         q->front++;
//     }
//     return process;
// }

int main(int argc, char *argv[]) {
    char *inputFilePath = "input_file.txt";
    char *outputFilePath;
    double timeSlice = 0.0;
    //double timeSliceSJF = 0.0;
    // if(argc==5){
    //     inputFilePath = argv[1];
    //     outputFilePath = argv[2];
    //     timeSlice = atof(argv[3]);
         //timeSliceSJF = atof(argv[4]);
    // }
    if(argc == 4){
        inputFilePath = argv[1];
        outputFilePath = argv[2];
        timeSlice = atof(argv[3]);
    }else if(argc == 3){
        inputFilePath = argv[1];
        outputFilePath = argv[2];
    }else{
        printf("Usage: %s input.txt output.txt TsRR\n", argv[0]);
        return 1;
    }

    // Parse command line arguments and read processes from input file

    FILE *inputFile = fopen(inputFilePath, "r");
    if(inputFile==NULL){
        perror("Error openeing input file");
        return 1;
    }

    //initialize variables
    int numProcesses = 0;
    struct Process *processes = NULL;
    char line[100]; // adjust the size as needed

    //Read processes until the end of the file
    while (fgets(line, sizeof(line), inputFile) != NULL){
        // Resize the processes array to accommodate more processes
        struct Process *temp = realloc(processes, (numProcesses + 1) * sizeof(struct Process));
        if (temp == NULL) {
            perror("Error reallocating memory");
            fclose(inputFile);
            free(processes);
            return 1;
        }
        processes = temp;

        if(sscanf(line, "%s %lf %lf", processes[numProcesses].PID, &processes[numProcesses].ArrivalTime, &processes[numProcesses].JobTime)!=3){
            // Handle error when reading process information
            fprintf(stderr, "Error: Invalid format in the input file.\n");
            fclose(inputFile);
            free(processes);
            return 1;
        }
        numProcesses++;
    }
    fclose(inputFile);

    //create a copy of the initial process
    struct Process *initialProcesses = malloc(numProcesses * sizeof(struct Process));
    memcpy(initialProcesses, processes, numProcesses * sizeof(struct Process));

    // Run FCFS algorithm
    runFCFS(processes, numProcesses);

    // Calculate average turnaround and response times
    double avgTurnaroundFCFS = 0.0, avgResponseFCFS = 0.0;
    double totalTurnaroundFCFS = 0.0, totalResponseFCFS = 0.0;

    for (int i = 0; i < numProcesses; i++) {
        totalTurnaroundFCFS += processes[i].StartTime + processes[i].JobTime - processes[i].ArrivalTime;
        totalResponseFCFS += processes[i].StartTime - processes[i].ArrivalTime;
    }

    avgTurnaroundFCFS = (double)totalTurnaroundFCFS / numProcesses;
    avgResponseFCFS = (double)totalResponseFCFS / numProcesses;

    // Write FCFS results to the output file
    FILE *outputFile = fopen(outputFilePath, "w");
    for(int i=0; i<numProcesses; i++){
        fprintf(outputFile, "%s %f %f ", processes[i].PID, processes[i].StartTime, processes[i].EndTime);
    }
    fprintf(outputFile, "\n%f %f\n", avgTurnaroundFCFS, avgResponseFCFS);

    //free(processes);
    fclose(outputFile);

    //restore the original processes from the copy
    memcpy(processes, initialProcesses, numProcesses * sizeof(struct Process));

    FILE *outputFileAppend = fopen(outputFilePath, "a"); // Append mode

    // Call the Round Robin algorithm
    runRoundRobin(processes, numProcesses, timeSlice, outputFileAppend);

    // Calculate and output average turnaround and response times for Round Robin
    double avgTurnaroundRR = 0.0, avgResponseRR = 0.0;
    double totalTurnaroundRR = 0.0, totalResponseRR = 0.0;

    for (int i = 0; i < numProcesses; i++) {
        totalTurnaroundRR += processes[i].EndTime - processes[i].ArrivalTime;
        totalResponseRR += processes[i].ResponseTime;
    }

    avgTurnaroundRR = (double)totalTurnaroundRR / numProcesses;
    avgResponseRR = (double)totalResponseRR / numProcesses;

    fprintf(outputFileAppend, "%f %f\n", avgTurnaroundRR, avgResponseRR);
    fclose(outputFileAppend);


    // Restore the original processes from the copy
    memcpy(processes, initialProcesses, numProcesses * sizeof(struct Process));

    FILE *outputFileAppendSJF = fopen(outputFilePath, "a"); // Append mode
    // Call the Shortest Job First (SJF) algorithm
    runSJF(processes, numProcesses, outputFileAppendSJF);

    // Calculate and output average turnaround and response times for SJF
    double avgTurnaroundSJF = 0.0, avgResponseSJF = 0.0;
    double totalTurnaroundSJF = 0.0, totalResponseSJF = 0.0;

    for (int i = 0; i < numProcesses; i++) {
        totalTurnaroundSJF += processes[i].TurnaroundTime;
        totalResponseSJF += processes[i].ResponseTime;
    }

    avgTurnaroundSJF = (double)totalTurnaroundSJF / numProcesses;
    avgResponseSJF = (double)totalResponseSJF / numProcesses;

    fprintf(outputFileAppendSJF, "%f %f\n", avgTurnaroundSJF, avgResponseSJF);
    fclose(outputFileAppendSJF);


    // Restore the original processes from the copy
    memcpy(processes, initialProcesses, numProcesses * sizeof(struct Process));

    // Call the Shortest Remaining Time First (SRTF) algorithm
    runSRTF(processes, numProcesses);

    qsort(processes, numProcesses, sizeof(struct Process), compareByStartTime);
    // Calculate and output average turnaround and response times for SRTF
    double avgTurnaroundSRTF = 0.0, avgResponseSRTF = 0.0;
    double totalTurnaroundSRTF = 0.0, totalResponseSRTF = 0.0;

    for (int i = 0; i < numProcesses; i++) {
        totalTurnaroundSRTF += processes[i].TurnaroundTime;
        totalResponseSRTF += processes[i].ResponseTime;
    }

    avgTurnaroundSRTF = (double)totalTurnaroundSRTF / numProcesses;
    avgResponseSRTF = (double)totalResponseSRTF / numProcesses;

   // Append SRTF results to the output file
    FILE *outputFileAppendSRTF = fopen(outputFilePath, "a");
    for (int i = 0; i < numProcesses; i++) {
        fprintf(outputFileAppendSRTF, "%s %f %f ", processes[i].PID, processes[i].StartTime, processes[i].EndTime);
    }
    fprintf(outputFileAppendSRTF, "\n%f %f\n", avgTurnaroundSRTF, avgResponseSRTF);
    fclose(outputFileAppendSRTF);


    free(initialProcesses);
    free(processes);

    createGanttChart(processes, numProcesses);

    return 0;
}

