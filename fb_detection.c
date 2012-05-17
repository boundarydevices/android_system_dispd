
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
#define DISPLAY_MAX 6
static struct uevent_device g_display[DISPLAY_MAX];

extern int handle_display(int fbid);

int fb_detection_bootstrap()
{
    char fb_path[255];
    char value[255];
    char tmp[255];
    char *uevent_params[2];
    FILE *fp;

    int i;

    for(i=0; i<DISPLAY_MAX; i++) {
        g_display[i].fb_id = i;
        g_display[i].fb_path = NULL;//"/sys/class/graphics/fb";
        g_display[i].dev_path = NULL;
        g_display[i].dev_name = NULL;
        g_display[i].dev_event = NULL;
        g_display[i].found = 0;
    }

    for(i=0; i<DISPLAY_MAX; i++) {
        memset(fb_path,    0, 255);
        snprintf(fb_path, 250, "/sys/class/graphics/fb%d", i);

        //first to check if the device exist
        if(!(fp = fopen(fb_path, "r"))) {
            LOGW("warning: fb%d %s device does not exist", i, fb_path);
            continue;
        }
        fclose(fp);

        //second to check if it is a BG or FG device
        //FG device is overlay, BG is real device.
        memset(tmp, 0, 255);
        strcpy(tmp, fb_path);
        strcat(tmp, "/name");
        if(!(fp = fopen(tmp, "r"))) {
            LOGE("error: open fb%d name path '%s' (%s) failed", i, tmp, strerror(errno));
            continue;
        }
        memset(value,  0, 255);
        if (!fgets(value, sizeof(value), fp)) {
            LOGE("Unable to read fb%d name %s", i, tmp);
            fclose(fp);
            continue;
        }
        if(strstr(value, "FG")) {
            LOGI("fb%d is overlay device", i);
            fclose(fp);
            continue;
        }
        fclose(fp);

        //read fb device name
        memset(tmp, 0, 255);
        strcpy(tmp, fb_path);
        strcat(tmp, "/fsl_disp_dev_property");
        if(!(fp = fopen(tmp, "r"))) {
            LOGE("Unable to open fb%d, %s", i, tmp);
            //if open failed make default name to ldb.
            g_display[i].dev_name = "ldb";
            LOGI("fb%d is device %s", i, g_display[i].dev_name);
        } else {
            memset(value,  0, 255);
            if (!fgets(value, sizeof(value), fp)) {
                LOGE("Unable to read fb%d value %s", i, tmp);
                //if read failed make default name to ldb.
                g_display[i].dev_name = "ldb";
            } else {
                g_display[i].dev_name = (char *) strdup(value);
                char *pc = g_display[i].dev_name;
                while(*pc != '\0')  {
                    if(*pc == '\n') {
                        *pc = '\0';
                    }
                    pc ++;
                }
            }
            LOGI("fb%d is device %s", i, g_display[i].dev_name);
            fclose(fp);
        }

        //third to check if it is a plugable device.
        memset(tmp, 0, 255);
        strcpy(tmp, fb_path);
        strcat(tmp, "/disp_dev/cable_state");
        if(!(fp = fopen(tmp, "r"))) {
            //if it is not a plugable device.
            LOGI("fb%d is not plugable device", i);
            g_display[i].dev_event = "EVENT=plugin";
            g_display[i].found = 1;
            continue;
        }

        //plugable device flag.
        g_display[i].dev_path = g_display[i].dev_name;
        memset(value,  0, 255);
        if (!fgets(value, sizeof(value), fp)) {
            LOGE("Unable to read fb%d value %s", i, tmp);
            fclose(fp);
        } else {
            if(strstr(value, "plugout")) {
                LOGI("fb%d device plugout", i);
            } else {
                g_display[i].found = 1;
                g_display[i].dev_event = "EVENT=plugin";
            }
            fclose(fp);
        }
    }

    for(i=0; i<DISPLAY_MAX; i++) {
        if(g_display[i].found == 1) {
            handle_display(g_display[i].fb_id);
        }
    }

    return 0;
}

int getDisplayfbid(char *path)
{
    int i;

    for(i=0; i<DISPLAY_MAX; i++) {
        if((g_display[i].dev_path != NULL) && strstr(path, g_display[i].dev_path)) {
            return g_display[i].fb_id;
        }
    }
    return -1;
}

char* getDisplayName(int fbid)
{
    if(fbid < 0 || fbid >= DISPLAY_MAX) {
        return NULL;
    }

    return g_display[fbid].dev_name;
}
