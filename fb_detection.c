
/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Copyright (C) 2012 Freescale Semiconductor, Inc. */

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include <sys/types.h>

#include "dispd.h"
#include "fb_detection.h"
#include "uevent.h"

#define DEBUG_BOOTSTRAP 0

int fb_detection_bootstrap()
{
    char path_state[255];
    char path_modalias[255];   
     
    char event_state[255];
    char event_device[255];

    char tmp[255];
    char *uevent_params[2];
    FILE *fp;
    boolean  found;


    LOGI("fb_detection_bootstrap IN");
    int i,j;
    
    for(i = 0; fb_table[i].fb_path != NULL; i++) {
        memset(path_state,    0, 255);
        memset(path_modalias, 0, 255);
        memset(event_state,   0, 255);
        memset(event_device,  0, 255);
        found = false;
        
            strcpy(path_state,fb_table[i].fb_path);
            strcat(path_state,SYSFS_CLASS_FB_DETECTION_PATH_STATE);
            if (!(fp = fopen(path_state, "r"))) {
                LOGE("Error opening fb name path '%s' (%s)", path_state, strerror(errno));
                continue;
            }
            if (!fgets(event_state, sizeof(event_state), fp)) {
                LOGE("Unable to read device cable_state");
                fclose(fp);
                continue;
            }
            fclose(fp);        
    
            strcpy(path_modalias, fb_table[i].fb_path);
            strcat(path_modalias, SYSFS_CLASS_FB_DETECTION_PATH_MODALIAS);
            if (!(fp = fopen(path_modalias, "r"))) {
                LOGE("Error opening fb name path '%s' (%s)", path_modalias, strerror(errno));
                continue;
            }
            if (!fgets(event_device, sizeof(event_device), fp)) {
                LOGE("Unable to read device cable_state");
                fclose(fp);
                continue;
            }
            fclose(fp);
        
        event_device[strlen(event_device) -1] = '\0';

        for(j =0; device_table[j].dev_path!=NULL; j++) {
            if(!strncmp(device_table[j].dev_modalias, event_device, strlen(event_device))) {
                fb_table[i].dev_path =(char *) strdup(device_table[j].dev_path);
                fb_table[i].dev_name =(char *) strdup(device_table[j].dev_name);
                found = true;
                break;
            }
        }
        
        if(found) {
            event_state[strlen(event_state) -1] = '\0';
            sprintf(tmp, "EVENT=%s", event_state);
            uevent_params[0] = (char *) strdup(tmp);
            uevent_params[1] = (char *) NULL;
            if (simulate_uevent(fb_table[i].dev_name, fb_table[i].dev_path, "add", uevent_params) < 0) {
                LOGE("Error simulating uevent (%s)", strerror(errno));
                return -errno;
            }
        }
    }
    return 0;
}



int getDisplayfbid(char *path)
{
    
    int i;
    for(i = 0; fb_table[i].fb_path != NULL; i++) {
        if(!strcmp(fb_table[i].dev_path, path))
        {
            return fb_table[i].fb_id;
        }
    }
    return -1;
}



int flush_fb_detection()
{
    char path_state[255];
    char path_modalias[255];   
     
    char event_state[255];
    char event_device[255];

    char tmp[255];
    char *uevent_params[2];
    FILE *fp;
    boolean  found;


    LOGI("fb_detection_bootstrap IN");
    int i,j;
    
    for(i = 0; fb_table[i].fb_path != NULL; i++) {
        memset(path_state,    0, 255);
        memset(path_modalias, 0, 255);
        memset(event_state,   0, 255);
        memset(event_device,  0, 255);
        found = false;
        
            strcpy(path_state,fb_table[i].fb_path);
            strcat(path_state,SYSFS_CLASS_FB_DETECTION_PATH_STATE);
            if (!(fp = fopen(path_state, "r"))) {
                LOGE("Error opening fb name path '%s' (%s)", path_state, strerror(errno));
                continue;
            }
            if (!fgets(event_state, sizeof(event_state), fp)) {
                LOGE("Unable to read device cable_state");
                fclose(fp);
                continue;
            }
            fclose(fp);        
    
            strcpy(path_modalias, fb_table[i].fb_path);
            strcat(path_modalias, SYSFS_CLASS_FB_DETECTION_PATH_MODALIAS);
            if (!(fp = fopen(path_modalias, "r"))) {
                LOGE("Error opening fb name path '%s' (%s)", path_modalias, strerror(errno));
                continue;
            }
            if (!fgets(event_device, sizeof(event_device), fp)) {
                LOGE("Unable to read device cable_state");
                fclose(fp);
                continue;
            }
            fclose(fp);
        
        event_device[strlen(event_device) -1] = '\0';

        for(j =0; device_table[j].dev_path!=NULL; j++) {
            if(!strncmp(device_table[j].dev_modalias, event_device, strlen(event_device))) {
                fb_table[i].dev_path =(char *) strdup(device_table[j].dev_path);
                fb_table[i].dev_name =(char *) strdup(device_table[j].dev_name);
                found = true;
                break;
            }
        }
    }
    return 0;
}
