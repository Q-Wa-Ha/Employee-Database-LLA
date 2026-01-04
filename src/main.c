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
}

int main(int argc, char *argv[]) { 
    bool newFile = false;
    char *filepath = NULL;
	int cmdArg;

    int dbfd = -1;

    while((cmdArg = getopt(argc, argv, "nf:")) != -1) {
        switch(cmdArg) {
            case 'n':
                newFile = true;
                break;
            
            case 'f':
                filepath = optarg;
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
    }
    else {
        dbfd = open_db_file(filepath);
        if(dbfd == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return EXIT_FAILURE;
        }
    }

    printf("newFile = %d\n", newFile);
    printf("filepath = %s\n", filepath);

    return EXIT_SUCCESS;
}