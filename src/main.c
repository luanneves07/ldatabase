#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

#include "file.h"
#include "parse.h"
#include "common.h"

void usage(char * const argv[]) {
    printf("Reads the header of a binary file and prints the number of employees stored.\n");
    printf("Usage: %s  -f <filename> [-n] -a <name,address,hours> [-l]\n", argv[0]);
    printf("Options:\n");
    printf("\t -f <filename> (required)  The binary file to read.\n");
    printf("\t -a <name,address,hours>   Add an employee to the database.\n");
    printf("\t -n                        Create a new file if it does not exist.\n");
    printf("\t -l                        List all employees in the database.\n");
}

int main(int argc, char * const argv[]) {
    int opt;
    char *filepath = NULL;
    char *employeeToAdd = NULL;
    bool listEmployees = false;
    bool createNewFile = false;
    
    while ((opt = getopt(argc, argv, "nf:a:l")) != -1) {
        switch (opt) {
            case 'n':
                createNewFile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case 'a':
                employeeToAdd = optarg;
                break;
            case 'l':
                listEmployees = true;
                break;
            default:
                printf("Invalid option -c %c\n", opt);
                usage(argv);
                return STATUS_ERROR;
        }
    }

    if (filepath == NULL) {
        usage(argv);
        return -1;
    }

    int dbfd = -1;
    dbheader_t *dbheader = NULL;
    if (createNewFile) {
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to create database file %s\n", filepath);
            return STATUS_ERROR;
        }
        int err = create_db_header(dbfd, &dbheader);
        if (err == STATUS_ERROR) {
            close_db_file(dbfd);
            free(dbheader);
            printf("Unable to create database header\n");
            return STATUS_ERROR;
        }
    } 

    if (dbfd == -1) {
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to open database file %s\n", filepath);
            return STATUS_ERROR;
        }
        int err = validate_db_header(dbfd, &dbheader);
        if (err == STATUS_ERROR) {
            close_db_file(dbfd);
            free(dbheader);
            printf("Unable to validate database header\n");
            return STATUS_ERROR;
        }
    }

    employee_t *employees = NULL;
    int err = read_employess(dbfd, dbheader, &employees);
    if (err == STATUS_ERROR) {
        close_db_file(dbfd);
        free(dbheader);
        free(employees);
        printf("Unable to read employees\n");
        return STATUS_ERROR;
    }

    if (employeeToAdd != NULL) {
        dbheader->count++;
        employees = realloc(employees, sizeof(employee_t) * dbheader->count);
        int err = add_employee(dbheader, employees, employeeToAdd);
        if (err == STATUS_ERROR) {
            close_db_file(dbfd);
            free(dbheader);
            free(employees);
            printf("Unable to add employee\n");
            return STATUS_ERROR;
        }
    }

    err = write_db(dbfd, dbheader, employees);
    if (err == STATUS_ERROR) {
        close_db_file(dbfd);
        free(dbheader);
        free(employees);
        printf("Unable to persist employees\n");
        return STATUS_ERROR;
    }

    if (listEmployees) {
        list_employees(dbheader, employees);
    }

    free(dbheader);
    free(employees);
    close_db_file(dbfd);
    return STATUS_SUCCESS;
}