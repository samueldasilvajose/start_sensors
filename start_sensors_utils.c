#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#include "start_sensors_utils.h"


int
start_sensors_copy_data(void **dst, const void *src, size_t nbytes)
{
    if (*dst || !src || (nbytes == SIZE_MAX))
    {
        return -1;
    }
    
    *dst = malloc(nbytes);
    start_sensors_test_alloc(*dst);
    memcpy(*dst, src, nbytes);
    return 0;    
}


size_t
extra_keys_read_file(char **buf, const char *file_name)
{
    if (!file_name || !buf)
    {
        return 0; // error
    }
    
    //pointer to the file read stream for file_name
    FILE *file_r = fopen(file_name, "r");

    //file status
    struct stat arq_st;

    //Checks if the file was opened correctly and if it was possible to access the file's data
    if (!file_r || stat(file_name, &arq_st))
    {
        return 0;  // error
    }

    //allocates enough memory to load the data from the file
    *buf = (char *) malloc((size_t) (arq_st.st_size + 1) * sizeof(char));
    start_sensors_test_alloc(*buf);

    //number of bytes processed
    size_t read_bytes = fread(*buf, sizeof(char), (size_t) arq_st.st_size, file_r); //reads the entire file
    if (!read_bytes) //checks the number of elements read
    {
        ss_fatal_errno("Error: the file \"%s\" could not be read\n", file_name);
        start_sensors_free(buf);
    }
    else
    {
        (*buf)[read_bytes] = '\0'; //define end of string
    }
    
    fclose(file_r);

    return read_bytes;
}


//Converts a string to a number, which can be of type int, long int, or double.
int 
start_sensors_convert_str_to_num(void *ptr, const char *num_str, convert_type_t type) 
{
    /*
        Error code:
            Null pointer: -3
            Conversion error: -2
            Limits error: -1
            Non-numeric elements: 1 or 2
    */

    //report error
    int err = 0;

    //stores the pointer to the unconverted part of the string num_str
    char *end_num_str = NULL;

    if ( !num_str || *num_str == '\0' )
    {
        ss_publish_error(SS_ERROR_WARNING, "Error: null pointer.\n");
        return -2; //empty string
    }
    
    switch (type)
    {
        case INT:
        {
            errno = 0; //clean errors

            //converted value
            long val = strtol(num_str, &end_num_str, 10);
            if (val > INT_MAX || val < INT_MIN || errno == ERANGE) //checks if the number exceeded the type limits
            {
                err = -1; 
            }
            else
            {
                *(int*) ptr = (int) val; //saves the value found
            }
            
            break;
        }
        case IHEXA:
        {
            errno = 0; //clean errors

            //converted value
            long val = strtol(num_str, &end_num_str, 16);
            if (val > INT_MAX || val < INT_MIN || errno == ERANGE) //checks if the number exceeded the type limits
            {
                err = -1; 
            }
            else
            {
                *(int*) ptr = (int) val; //saves the value found
            }
            
            break;
        }
        case LONG:
        {
            errno = 0; //clean errors

            //converted value
            long val = strtol(num_str, &end_num_str, 10); //converte string em long int

            if (errno == ERANGE) //checks if the number exceeded the type limits
            {
                err = -1;
            }
            else
            {
                *(long*) ptr = val; //saves the value found
            }

            break;
        }
        case LHEXA:
        {
            errno = 0; //clean errors

            //converted value
            long val = strtol(num_str, &end_num_str, 16); //converte string em long int

            if (errno == ERANGE) //checks if the number exceeded the type limits
            {
                err = -1;
            }
            else
            {
                *(long*) ptr = val; //saves the value found
            }

            break;
        }
        case DOUBLE:
        {
            errno = 0; //clean errors

            //converted value
            double val = strtod(num_str, &end_num_str); //converte string em double

            if (errno == ERANGE) //checks if the number exceeded the type limits
            {
                err = -1;
            }
            else
            {
                *(double*) ptr = val; //saves the value found
            }

            break;
        }
    }

    //error messages
    if (!end_num_str)
    {
        ss_publish_error(SS_ERROR_WARNING, "Error: conversion (%d) not available.\n", type);
        err = 1;
    }
    else if (*end_num_str != '\0')
    {
        ss_publish_error(SS_ERROR_WARNING, "Error: number (%s) with non-numeric part (%s).\n", num_str, end_num_str);
        err = 2;
    }
    else if (err)
    {
        ss_publish_error(SS_ERROR_WARNING, "Error: number (%s) outside the acceptable limit.\n", num_str);
    }
    
    return err;
}
