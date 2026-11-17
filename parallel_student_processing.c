#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define NUM_CHILDREN 3
#define NUM_SUBJECTS 3

typedef struct student{ // Structure of a student
    int id;
    char name[32];
    float marks[NUM_SUBJECTS];
    float total;
    float average;
    char grade;
} Student;

char calculate_grade(float avg);

void process_child(int child_id, Student *records, int start, int count);

int generate_final_report(int num_children);

int main(void) {
    Student studentList[] = { //Preloaded student data
        {101, "Badal",   {85, 90, 88}, 0, 0, ' '},
        {102, "Prakhar", {55, 60, 58}, 0, 0, ' '},
        {103, "Aryan",   {92, 95, 94}, 0, 0, ' '},
        {104, "Sumit",   {40, 45, 38}, 0, 0, ' '},
        {105, "Dhruv",   {72, 68, 75}, 0, 0, ' '},
        {106, "Roni",    {80, 82, 85}, 0, 0, ' '}
    };

    int total_students = sizeof(studentList) / sizeof(studentList[0]);
    pid_t pids[NUM_CHILDREN];

    int chunk = total_students / NUM_CHILDREN; // Number of students each child will process
    int rem = total_students % NUM_CHILDREN; // Remaining students to distribute among the first few children

    // Creating child processes
    for (int i = 0; i < NUM_CHILDREN; i++) {
        int start = i * chunk + (i < rem ? i : rem); // Which student index to start processing from
        int count = chunk + (i < rem ? 1 : 0); // How many students to process

        pid_t pid = fork();

        if (pid < 0) {
            printf("Fork failed, %dth child process could not be created\n", i);
            exit(1);
        } else if (pid == 0) {
            // Inside child process
            process_child(i, studentList, start, count);
        } else {
            // Inside parent process
            pids[i] = pid; // Storing child PID for synchronization
        }
    }

    // Process synchronization
    for (int i = 0; i < NUM_CHILDREN; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // Generating the final report
    if (generate_final_report(NUM_CHILDREN) != 0) 
        return 1;

    printf("\nReport successfully generated at final_report.txt\n");

    return 0;
}

char calculate_grade(float avg) {
    if (avg >= 85.0f) return 'A';
    if (avg >= 70.0f) return 'B';
    if (avg >= 50.0f) return 'C';
    return 'F';
}

void process_child(int child_id, Student *records, int start, int count) {
    char filename[32];
    snprintf(filename, sizeof(filename), "temp_child_%d.txt", child_id); // Naming the temporary file for this child process

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("Failed to open temporary output file for %dth child\n", child_id);
        exit(1);
    }

    for (int i = start; i < start + count; i++) { // For all the students assigned to this process
        records[i].total = 0.0f;
        for (int j = 0; j < NUM_SUBJECTS; j++) {
            records[i].total += records[i].marks[j];
        }
        records[i].average = records[i].total / NUM_SUBJECTS;
        records[i].grade = calculate_grade(records[i].average);

        // Writing processed record: ID, Name, Total, Average, Grade
        fprintf(fp, "%d %s %.2f %.2f %c\n",
                records[i].id,
                records[i].name,
                records[i].total,
                records[i].average,
                records[i].grade);
    }

    fclose(fp);
    exit(EXIT_SUCCESS); // Terminate child process cleanly after processing its assigned records
}

int generate_final_report(int num_children) {
    FILE *report = fopen("final_report.txt", "w");
    if (!report) {
        printf("Failed to create the final report\n");
        return 1;
    }
//Writing the final report in clean tabular format
    fprintf(report, "===============================================\n");
    fprintf(report, "           CONSOLIDATED REPORT                \n");
    fprintf(report, "===============================================\n");
    fprintf(report, "%-6s %-15s %-8s %-8s %-6s\n", "ID", "Name", "Total", "Average", "Grade");// Padded data for better alignment
    fprintf(report, "-----------------------------------------------\n");

    for (int i = 0; i < num_children; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "temp_child_%d.txt", i);

        FILE *part = fopen(filename, "r");
        if (!part) continue;

        int id;
        char name[32];
        float total, avg;
        char grade;

        while (fscanf(part, "%d %s %f %f %c", &id, name, &total, &avg, &grade) == 5) {
            fprintf(report, "%-6d %-15s %-8.2f %-8.2f %-6c\n", id, name, total, avg, grade);
            printf("[Loaded] ID: %d | %s | Avg: %.2f | Grade: %c\n", id, name, avg, grade);
        }

        fclose(part);
        remove(filename); // Clean up temporary intermediate file
    }

    fprintf(report, "===============================================\n");
    fclose(report);
    return 0;
}