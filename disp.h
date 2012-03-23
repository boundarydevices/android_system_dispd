
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

/* Copyright (c) 2010-2012 Freescale Semiconductor, Inc. */

#ifndef _DISP_H
#define _DISP_H

#define DISPD_EVT_DISP_CONNECTED            "disp_connected"
#define DISPD_EVT_DISP_DISCONNECTED         "disp_disconnected"
#define DISPD_EVT_DISP_ENABLED            "disp_enabled"
#define DISPD_EVT_DISP_DISABLED         "disp_disabled"
#define MAX_DISP_DEVICE                  4
#define MAX_DISP_DEVICE_MODE                  128

static const int InterfaceListResult       = 110;

static const int CommandOkay               = 200; 
static const int OperationFailed           = 400;

static const int InterfaceConnected        = 600;
static const int InterfaceDisconnected     = 601;
static const int InterfaceEnabled          = 602;
static const int InterfaceDisabled         = 603;

typedef struct
{
	char mode[20];
	int width;
	int height;
	int freq;
}
disp_mode;

typedef enum {
    CHECK_NEXT_STATE,
    FIND_WIDTH_STATE,
    FIND_JOINT_STATE,
    FIND_HEIGHT_STATE,
    PREFIX_FREQ_STATE,
    FREQUENCY_STATE,
    FIND_NEWLINE_STATE
}
read_state;

typedef struct
{
    boolean disp_connected;
    boolean disp_enabled;
    disp_mode disp_mode_list[MAX_DISP_DEVICE_MODE];
    int disp_mode_length;
}
disp_class;

//most support 4 pluggable display device;
static  disp_class disp_class_list[MAX_DISP_DEVICE];


//static boolean disp_connected = false;
//static boolean disp_enabled = false;

//static int disp_mode_length  = 0;
//static disp_mode disp_mode_list[128];
//static disp_mode disp_mode_req;



// internel operation , not relate with the fbid
int   str2int(char *p, int *len);
int   disp_mode_compare( const void *arg1, const void *arg2);

int     get_available_mode(int fbid, const char *mode_list);
int     read_graphics_fb_mode(int fbid);

int     disp_send_status();
boolean disp_connected_get(int fbid);
int     disp_connected_set(int fbid, boolean enabled);
int     disp_enabled_set(int fbid, boolean enabled);

boolean disp_enabled_get(int fbid);
char*   disp_get_disp_modelist(int fbid);
int     disp_set_disp_mode(int fbid, char* cmd);

#endif
