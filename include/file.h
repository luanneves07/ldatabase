#ifndef FILE_H
#define FILE_H

int open_db_file(const char *filename);
int close_db_file(int fd);
int create_db_file(const char *filename);

#endif