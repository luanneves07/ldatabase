#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "parse.h"
#include "common.h"

const unsigned int HEADER_SECRET = 0x4c4c4144;

int create_db_header(int fd, dbheader_t **header) {
    if (fd == -1) {
        printf("Invalid file descriptor\n");
        return STATUS_ERROR;
    }

    dbheader_t *h = calloc(1, sizeof(dbheader_t));
    if (h == NULL) {
        perror("calloc");
        exit(STATUS_ERROR);
    }

    h->secret = HEADER_SECRET;
    h->version = 0x1;
    h->count = 0;
    h->filesize = sizeof(dbheader_t);

    *header = h;
    return STATUS_SUCCESS;
}

int validate_db_header(int fd, dbheader_t **header) {
    if (fd == -1) {
        printf("Invalid file descriptor\n");
        return STATUS_ERROR;
    }

    dbheader_t *h = calloc(1, sizeof(dbheader_t));
    if (h == NULL) {
        perror("calloc");
        exit(STATUS_ERROR);
    }

    if (read(fd, h, sizeof(dbheader_t)) != sizeof(dbheader_t)) {
        perror("read");
        free(h);
        return STATUS_ERROR;
    }

    h->version = ntohs(h->version);
    h->count = ntohs(h->count);
    h->secret = ntohl(h->secret);
    h->filesize = ntohl(h->filesize);

    if (h->secret != HEADER_SECRET) {
        printf("Invalid header secret\n");
        free(h);
        return STATUS_ERROR;
    }

    if (h->version != 0x1) {
        printf("Invalid header version\n");
        free(h);
        return STATUS_ERROR;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        free(h);
        return STATUS_ERROR;
    }

    if (st.st_size != h->filesize) {
        printf("Corrupted database file size\n");
        free(h);
        return STATUS_ERROR;
    }

    *header = h;
    return STATUS_SUCCESS;
}

int write_db(int fd, dbheader_t *header, employee_t *employees) {
    if (fd == -1) {
        printf("Invalid file descriptor\n");
        return STATUS_ERROR;
    }

    header->secret = htonl(header->secret);
    header->version = htons(header->version);
    header->count = htons(header->count);
    header->filesize = htonl(sizeof(dbheader_t) + (ntohs(header->count) * sizeof(employee_t)));

    lseek(fd, 0, SEEK_SET);
    if (write(fd, header, sizeof(dbheader_t)) != sizeof(dbheader_t)) {
        perror("write");
        return STATUS_ERROR;
    }

    for (int i = 0; i < ntohs(header->count); i++) {
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(employee_t));
    }
    return STATUS_SUCCESS;
}

int read_employess(int fd, dbheader_t *header, employee_t **employees) {
    if (fd == -1) {
        printf("Invalid file descriptor\n");
        return STATUS_ERROR;
    }

    if (header == NULL) {
        printf("Invalid header\n");
        return STATUS_ERROR;
    }

    int count = ntohs(header->count);
    employee_t *e = calloc(count, sizeof(employee_t));
    if (e == NULL) {
        perror("calloc");
        exit(STATUS_ERROR);
    }

    read(fd, e, count * sizeof(employee_t));
    for (int i = 0; i < count; i++) {
        e[i].hours = ntohl(e[i].hours);
    }

    *employees = e;
    return STATUS_SUCCESS;
}

int add_employee(dbheader_t *header, employee_t *employee, char * const employeeToAdd) {
    char *name = strtok(employeeToAdd, ",");
    char *address = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");
    if (name == NULL || address == NULL || hours == NULL) {
        printf("Invalid employee format\n");
        return STATUS_ERROR;
    }

    strncpy(employee[header->count-1].name, name, sizeof(employee[header->count-1].name));
    strncpy(employee[header->count-1].address, address, sizeof(employee[header->count-1].address));
    employee[header->count-1].hours = atoi(hours);

    printf("Added employee %s %s %d\n", employee[header->count-1].name, employee[header->count-1].address, employee[header->count-1].hours);

    return STATUS_SUCCESS;
}

void list_employees(dbheader_t *header, employee_t *employees) {
    for (int i = 0; i < ntohs(header->count); i++) {
        printf("Employee %d\n", i);
        printf("\tName: %s\n", employees[i].name);
        printf("\tAddress: %s\n", employees[i].address);
        printf("\tHours: %d\n", ntohl(employees[i].hours));
    }
}