
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

#ifndef _HDMI_DETECTION_H
#define _HDMI_DETECTION_H

#include "dispd.h"

#define SYSFS_CLASS_FB_DETECTION_PATH_STATE     "/disp_dev/cable_state"
#define SYSFS_CLASS_FB_DETECTION_PATH_MODALIAS  "/disp_dev/modalias"


#define SYSFS_CLASS_HDMI_DETECTION_PATH "/devices/platform/mxc_hdmi"
#define SYSFS_CLASS_LVDS_DETECTION_PATH "/devices/platform/ldb"


struct uevent_device {
    int fb_id;
    char *fb_path;
    char *dev_path;
    char *dev_name;
    char *dev_event;
    int found;
};

static struct uevent_device fb_table[] = {
{0, "/sys/class/graphics/fb0", NULL, NULL, NULL, 0},
{2, "/sys/class/graphics/fb2", NULL, NULL, NULL, 0},
{-1, NULL, NULL, NULL, NULL, 0}
};

struct display_device {
    char *dev_path;
    char *dev_name;
    char *dev_modalias;
};

static struct display_device device_table[] = {
{"/devices/platform/ldb", "mxc_ldb","i2c:mxc_ldb_i2c"},
{"/devices/platform/mxc_hdmi", "mxc_hdmi", "platform:mxc_hdmi"},
{NULL,NULL,NULL}
};


int fb_detection_bootstrap();
int getDisplayfbid(char *path);
int flush_fb_detection();

#endif
