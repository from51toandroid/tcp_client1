

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>


#include <sys/stat.h>
#include <unistd.h>


#define RESULT_MAX_BUFF_SIZE 1024000
void printdir(char *dir, int depth)
{
    DIR *dp;
	char buffer[RESULT_MAX_BUFF_SIZE];
	char cmd_str[RESULT_MAX_BUFF_SIZE];
    struct dirent *entry;
    struct stat statbuf;  
    if( (dp = opendir(dir)) == NULL ){
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return;
    }
    //fprintf(stdout,"open directory: %s\n", dir);
    chdir(dir);
    while((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name,&statbuf);
        if( S_ISDIR(statbuf.st_mode) ){
            if( strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0 ){
                continue;
            }
            //printf("%*s%s/\n",depth,"",entry->d_name);
            if( strcmp("sys",entry->d_name) == 0 || strcmp("proc",entry->d_name) == 0 ){
                continue;
            }
            printdir(entry->d_name,depth+4);
        }else{
            //printf("%*s%s\n",depth,"",entry->d_name);
			
			exec_cmd_and_get_result("pwd", buffer);
			 //printf("result = %s\n", buffer);

			 strcat(buffer, "/");
			 strcat(buffer, entry->d_name);

			 printf("path = %s\n", buffer);


			 strcpy( cmd_str, "cat ");
			 strcat(cmd_str, buffer);

			 exec_cmd_and_get_result(cmd_str, buffer);
			 //printf("result = %s\n", buffer);
               
			 //printf("result1 = %s\n", buffer);

			 if(strlen(buffer)==83){
				 printf("result1 = %s\n", buffer);


			 }

			 printf("result1 len = %ld\n", strlen(buffer) );
			 
			 
        }
    }
    chdir("..");
    closedir(dp);
}

