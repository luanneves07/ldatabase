#ifndef PARSE_H
#define PARSE_H

typedef struct {
    unsigned int secret;
    unsigned short version;
    unsigned short count;
    unsigned int filesize;
} dbheader_t;

typedef struct {
    char name[256];
    char address[256];
    unsigned int hours;
} employee_t;

int create_db_header(int fd, dbheader_t **header);
int validate_db_header(int fd, dbheader_t **header);
int read_employess(int fd, dbheader_t *header, employee_t **employees);
int write_db(int fd, dbheader_t *header, employee_t *employees);

int add_employee(dbheader_t *header, employee_t *employee, char * const employeeToAdd);
void list_employees(dbheader_t *header, employee_t *employees);

#endif