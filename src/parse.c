#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "common.h"
#include "parse.h"

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addString) {
    if(dbhdr == NULL) { return STATUS_ERROR; }
    if(employees == NULL) { return STATUS_ERROR; }
    if(*employees == NULL) { return STATUS_ERROR; }
    if(addString == NULL) { return STATUS_ERROR; }

    char *name = strtok(addString, ",");
    if(name == NULL) { return STATUS_ERROR; }

    char *addr = strtok(NULL, ",");
    if(addr == NULL) { return STATUS_ERROR; }

    char *hours = strtok(NULL, ",");
    if(hours == NULL) { return STATUS_ERROR; }


    struct employee_t *e = *employees;
    e = realloc(e, sizeof(struct employee_t) * (dbhdr->count + 1));
    if(e == NULL) {
        return STATUS_ERROR;
    }

    dbhdr->count++;

    strncpy(e[dbhdr->count - 1].name, name, sizeof(e[dbhdr->count - 1].name) - 1);
    strncpy(e[dbhdr->count - 1].address, addr, sizeof(e[dbhdr->count - 1].address) - 1);
    e[dbhdr->count - 1].hours = atoi(hours);

    *employees = e;

    return STATUS_SUCCESS;
}

int remove_employee(struct dbheader_t* dbhdr, struct employee_t ** employeesOut, char *employeeName) {
    if(dbhdr == NULL) {
        return STATUS_ERROR;
    }
    if(employeesOut == NULL) {
        return STATUS_ERROR;
    }

    struct employee_t *employees = *employeesOut;

    if(employees == NULL) {
        return STATUS_ERROR;
    }


    for(int i = 0; i < dbhdr->count; i++) {
        while(strcmp(employeeName, employees[i].name) == 0) {
            bool shifted = false;

            for(int j = i; j < dbhdr->count - 1; j++) {
                employees[j] = employees[j + 1];
                shifted = true;
            }

            dbhdr->count--;

            if(!shifted) {
                break;
            }
        }
    }

    return STATUS_SUCCESS;
}

int list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
    if(dbhdr == NULL) {
        return STATUS_ERROR;
    }
    if(employees == NULL) {
        return STATUS_ERROR;
    }

    for(int i = 0; i < dbhdr->count; i++) {
        printf("Employee %d\n", i);
        printf("\tName: %s\n", employees[i].name);
        printf("\tAddress: %s\n", employees[i].address);
        printf("\tHours: %u\n", employees[i].hours);
    }

    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    if(fd < 0) {
        printf("Got a bad file descriptor from the user\n");
        return STATUS_ERROR;
    }

    int count = dbhdr->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if(employees == NULL) {
        printf("Malloc failed\n");
        return STATUS_ERROR;
    }

    read(fd, employees, count * sizeof(struct employee_t));

    for(int i = 0; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;
    return STATUS_SUCCESS;
}


int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    if(fd < 0) {
        printf("Got a bad file descriptor from the user\n");
        return STATUS_ERROR;
    }

    int realCount = dbhdr->count;

    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->version = htons(dbhdr->version);
    dbhdr->count = htons(dbhdr->count);
    dbhdr->filesize = htonl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * realCount);

    lseek(fd, 0, SEEK_SET);
    write(fd, dbhdr, sizeof(struct dbheader_t));

    for(int i = 0; i < realCount; i++) {
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
    }

    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if(fd < 0) {
        printf("Got a bad file descriptor from the user\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if(header == NULL) {
        printf("Malloc failed to create database header\n");
        return STATUS_ERROR;
    }

    if(read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->magic = ntohl(header->magic);
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->filesize = ntohl(header->filesize);

    if(header->magic != HEADER_MAGIC) {
        printf("Improper header magic\n");
        free(header);
        return STATUS_ERROR;
    }

    if(header->version != 1) {
        printf("Improper header version\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;

    return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **headerOut) {
	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if(header == NULL) {
        printf("Malloc failed to create database header\n");
        return STATUS_ERROR;
    }

    header->magic = HEADER_MAGIC;
    header->version = 0x1;
    header->count = 0;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;

    return STATUS_SUCCESS;
}


