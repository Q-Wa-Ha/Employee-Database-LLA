#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n  - create new database file\n");
    printf("\t -f  - (required) path to database file\n");
    printf("\t -l  - list the employees\n");
    printf("\t -a  - add via CSV list of (name,address,salary)\n");
    printf("\t -r  - remove via name\n");
    printf("\t -h  - edit hours via CSV list of (name,hours)\n");
}

int main(int argc, char *argv[]) { 
    bool newFile = false;
    char *filepath = NULL;
    char *addString = NULL;
    char *removeName = NULL;
    char *editHours = NULL;
    bool list = false;
	int cmdArg;

    int dbfd = -1;
    struct dbheader_t *dbhdr = NULL;
    struct employee_t *employees = NULL;

    while((cmdArg = getopt(argc, argv, "nf:a:r:h:l")) != -1) {
        switch(cmdArg) {
            case 'n':
                newFile = true;
                break;
            
            case 'f':
                filepath = optarg;
                break;

            case 'a':
                addString = optarg;
                break;

            case 'r':
                removeName = optarg;
                break;

            case 'h':
                editHours = optarg;
                break;

            case 'l':
                list = true;
                break;

            case '?':
                break;
            
            default:
                return EXIT_FAILURE;
        }
    }

    if(filepath == NULL) {
        printf("Filepath is a required argument\n");
        print_usage(argv);

        return EXIT_FAILURE;
    }
    
    if(newFile) {
        dbfd = create_db_file(filepath);
        if(dbfd == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return EXIT_FAILURE;
        }

        if(create_db_header(&dbhdr) == STATUS_ERROR) {
            printf("Failed to create database header\n");
            return EXIT_FAILURE;
        }
    }
    else {
        dbfd = open_db_file(filepath);
        if(dbfd == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return EXIT_FAILURE;
        }

        if(validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
            printf("Failed to validate database header\n");
            return EXIT_FAILURE;
        }
    }

    if(read_employees(dbfd, dbhdr, &employees) == STATUS_ERROR) {
        printf("Failed to read employees\n");
        return EXIT_FAILURE;
    }

    if(addString) {
        add_employee(dbhdr, &employees, addString);
    }

    if(removeName) {
        remove_employee(dbhdr, &employees, removeName);
    }

    if(editHours) {
        edit_employee_hours(dbhdr, &employees, editHours);
    }

    if(list) {
        list_employees(dbhdr, employees);
    }

    output_file(dbfd, dbhdr, employees);

    return EXIT_SUCCESS;
}