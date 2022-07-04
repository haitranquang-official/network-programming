#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <dirent.h>
#include "scan_dir.h"

void append(char* data, char** presp)
{
    int oldlen = (*presp == NULL ? 0 : strlen(*presp));
    *presp = (char*)realloc(*presp, oldlen + strlen(data) + 1);
    memset(*presp + oldlen, 0, strlen(data) + 1);
    sprintf(*presp + strlen(*presp), "%s", data);
}

void scan(char* path, char** presp)
{
    struct dirent** entries = NULL;
    int n = scandir(path, &entries, NULL, alphasort);
    int i = 0;
    if (entries != NULL)
    {
        while (i < n && entries[i] != NULL)
        {
            char tmp[1024];
            memset(tmp, 0, sizeof(tmp));
            if (entries[i]->d_type == DT_REG)
            {
                char fullpath[1024];
                memset(fullpath, 0, sizeof(fullpath));
                sprintf(fullpath, "%s%s", path, entries[i]->d_name);
                FILE* f = fopen(fullpath, "rb");
                if (f != NULL)
                {
                    fseek(f, 0, SEEK_END);
                    int size = ftell(f);
                    fclose(f);
                    sprintf(tmp, "type=file;size=%d;modify=20220619235756.833;perms=awrfd; %s\r\n", size, entries[i]->d_name);
                }
            }else if (entries[i]->d_type == DT_DIR)
            {
                sprintf(tmp, "type=dir;modify=20211221005930.755;perms=cplemfd; %s\r\n", entries[i]->d_name);        
            }
            
            append(tmp, presp);
            free(entries[i]);
            entries[i] = NULL;
            i += 1;
        }
        free(entries);
    }
}