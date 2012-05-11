
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

#include <fcntl.h>
#include <errno.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include "dispd.h"
#include "disp.h"

#define DEBUG_DISP 0

int disp_connected_set(int fbid, boolean enabled)
{
    LOGI("disp_connected_set(): %d",enabled);
    disp_class_list[fbid].disp_connected = enabled;
    //Disable the connection will also automatically disable the display
    if((disp_class_list[fbid].disp_connected == false) && (disp_class_list[fbid].disp_enabled == true)) {
        disp_class_list[fbid].disp_enabled = false;
        //send_msg(DISPD_EVT_DISP_DISABLED);
		send_msg_with_code(InterfaceDisabled, DISPD_EVT_DISP_DISABLED, fbid);
    }
    
    if(enabled) {
//		read_graphics_fb_mode(fbid);
    }
    else{
        disp_class_list[fbid].disp_mode_length = 0;
    }

    //send_msg(enabled ? DISPD_EVT_DISP_CONNECTED : DISPD_EVT_DISP_DISCONNECTED);
    send_msg_with_code(enabled ? InterfaceConnected: InterfaceDisconnected, 
                    enabled ? DISPD_EVT_DISP_CONNECTED : DISPD_EVT_DISP_DISCONNECTED, fbid);
    return 0;
}

boolean disp_connected_get(int fbid)
{
    return disp_class_list[fbid].disp_connected;
}

int disp_enabled_set(int fbid, boolean enabled)
{
#if DEBUG_DISP
    LOG_DISP("disp_connected_set(): %d",enabled);
#endif
    if((enabled == true)&&(disp_class_list[fbid].disp_connected != true)) {
        LOGE("Error!Please connect extended display connection");
        return -1;
    }
    disp_class_list[fbid].disp_enabled = enabled;

    send_msg_with_code(enabled ? InterfaceEnabled: InterfaceDisabled, 
                    enabled ? DISPD_EVT_DISP_ENABLED : DISPD_EVT_DISP_DISABLED, fbid);

    return 0;
}

boolean disp_enabled_get(int fbid)
{
    return disp_class_list[fbid].disp_enabled;
}

int disp_send_status()
{
    int rc = 0;
    int i;

#if DEBUG_DISP
    LOG_DISP("disp_send_status():");
#endif
    for(i=0; i<MAX_DISP_DEVICE; i++)
    {
        rc = send_msg_with_code(disp_connected_get(i) ? InterfaceConnected: InterfaceDisconnected, 
                    disp_connected_get(i) ? DISPD_EVT_DISP_CONNECTED : DISPD_EVT_DISP_DISCONNECTED, i);
        if (rc < 0)
            return rc;

        rc = send_msg_with_code(disp_enabled_get(i) ? InterfaceEnabled: InterfaceDisabled, 
                    disp_enabled_get(i) ? DISPD_EVT_DISP_ENABLED : DISPD_EVT_DISP_DISABLED, i);
        if (rc < 0)
            return rc;
    }
    return rc;
}


/*****************************************************************************/
#define SINGLE_DISPLAY_CAPABILITY  (1920 * 1080 * 60)
#define DUAL_DISPLAY_CAPABILITY    (1920 * 1080 * 30)


int str2int(char *p, int *len)
{
	int val = 0;
	int length =0;
	if(!p) return -1;

	while(p[0] >= '0' && p[0] <= '9')
	{
		val = val * 10 + p[0] - '0';
		p++;
		length ++;
	}
    *len = length;
	return val;
}

int disp_mode_compare( const void *arg1, const void *arg2)
{
	disp_mode *dm1 = (disp_mode *)arg1;
	disp_mode *dm2 = (disp_mode *)arg2;

    if(dm1->width  > dm2->width)  return -1;
    else if (dm1->width  < dm2->width) return 1;
    else {
        if(dm1->height > dm2->height) return -1;
        else if (dm1->height < dm2->height )  return 1;
        else {
            if (dm1->freq > dm2->freq ) return -1;
            else if (dm1->freq < dm2->freq ) return 1;
            else return 0;
        }
    }

	return 0;
}

int get_available_mode(int fbid, const char *mode_list)
{
	int disp_threshold = 0;
	int i,disp_mode_count = 0;
	read_state state = CHECK_NEXT_STATE;
	char *p = (char *)mode_list;
	char *start = p;
	char *end   = p;
    int len = 0;
    if(!p) return 0;


	while(p[0])
	{
		switch(state)
		{
		case CHECK_NEXT_STATE:
			if(!strncmp(p, "D:", 2)
				|| !strncmp(p, "S:", 2)
				|| !strncmp(p, "U:", 2)
				|| !strncmp(p, "V:", 2))
			{
				start = p;
				state = FIND_WIDTH_STATE;
				p+=2;
			}
			else p++;
			break;
		case FIND_WIDTH_STATE:
			if(p[0]>='0' && p[0]<='9')
			{    
			    len = 0;
				disp_class_list[fbid].disp_mode_list[disp_mode_count].width = str2int(p, &len);
				state = FIND_JOINT_STATE;
				p =  p +len;
			}
			else p++;
			break;
		case FIND_JOINT_STATE:
			if(p[0] == 'x' || p[0] == 'X')
			{
			    p++;
				state = FIND_HEIGHT_STATE;
			}
			else p++;
			break;
		case FIND_HEIGHT_STATE:
			if(p[0]>='0' && p[0]<='9')
			{
			    len = 0;
				disp_class_list[fbid].disp_mode_list[disp_mode_count].height = str2int(p,&len);
				state = PREFIX_FREQ_STATE;
				p =  p +len;
			}
			else p++;
			break;
		case PREFIX_FREQ_STATE:
			if(!strncmp(p, "p-", 2) || !strncmp(p, "i-", 2))
			{
				state = FREQUENCY_STATE;
				p+=2;
			}
			else p++;
			break;
		case  FREQUENCY_STATE:
			if(p[0]>='0' && p[0]<='9')
			{    
			    len = 0;
				disp_class_list[fbid].disp_mode_list[disp_mode_count].freq = str2int(p,&len);
				state = FIND_NEWLINE_STATE;
				p =  p +len;
			}
			else p++;
			break;
		case FIND_NEWLINE_STATE:
			if(p[0] == '\n')
			{
				end = p+1;
				strncpy(disp_class_list[fbid].disp_mode_list[disp_mode_count].mode, start, (size_t)end -(size_t)start); 
				disp_mode_count ++;
				state = CHECK_NEXT_STATE;
				p++;
                if(disp_mode_count >= sizeof(disp_class_list[fbid].disp_mode_list)/sizeof(disp_class_list[fbid].disp_mode_list[0])) goto check_mode_end;
			}
			else p++;
			break;
		default:
			p++;
			break;
		}
	}

check_mode_end:

	qsort(&disp_class_list[fbid].disp_mode_list[0], disp_mode_count, sizeof(disp_mode), disp_mode_compare);

    disp_class_list[fbid].disp_mode_length = disp_mode_count;

    return 0;
}

int read_graphics_fb_mode(int fb) 
{
    int size=0;
    int fp_modes=0;
    char fb_modes[1024];
    char temp_name[256];

    sprintf(temp_name, "/sys/class/graphics/fb%d/modes", fb);
    fp_modes = open(temp_name,O_RDONLY, 0);
    if(fp_modes < 0) {
        LOGI("Error %d! Cannot open %s", fp_modes, temp_name);
        goto set_graphics_fb_mode_error;
    }

    memset(fb_modes, 0, sizeof(fb_modes));
    size = read(fp_modes, fb_modes, sizeof(fb_modes));
    if(size <= 0)
    {
        LOGI("Error! Cannot read %s", temp_name);
        goto set_graphics_fb_mode_error;
    }

    close(fp_modes); fp_modes = 0;

    if(size == sizeof(fb_modes)) fb_modes[size -1] = 0;

    get_available_mode(fb, fb_modes);

    return 0;

set_graphics_fb_mode_error:

    if(fp_modes > 0) close(fp_modes);

    return -1;
}

static disp_mode g_config_mode[MAX_DISP_DEVICE_MODE];
static int g_config_len = 0;

char* disp_get_disp_modelist(int fbid)
{
    LOGI("disp_get_disp_modelist");
    int i;
    char temp_mode[20];
    int pointer =0;
    char mode_send[MAX_DISP_DEVICE_MODE][20];
    int num_send = 0;

    if (g_config_len == 0) {
        char conf_modes[1024];
        int size;
        memset(conf_modes, 0, sizeof(conf_modes));
        memset(&g_config_mode[0], 0, sizeof(g_config_mode));
        int fd = open("/system/etc/display_mode.conf", O_RDONLY, 0);
        if(fd < 0) {
            LOGE("Warning: /system/etc/display_mode.conf not defined");
        }
        else {
            size = read(fd, conf_modes, sizeof(conf_modes));
            if(size > 0) {
                char* m_start = conf_modes;
                int m_len = 0;
                char *pmode = conf_modes;
                while(*pmode != '\0') {
                    if (*pmode == '\n') {
                        m_len = pmode - m_start + 1;
                        strncpy(g_config_mode[g_config_len].mode, m_start, m_len);
                        g_config_len ++;
                        m_start = pmode + 1;
                    }
                    pmode ++;
                }//while
            }
        }//else
    }

    memset(&disp_class_list[fbid].disp_mode_list[0], 0, MAX_DISP_DEVICE_MODE*sizeof(disp_mode));
    read_graphics_fb_mode(fbid);
    memset(mode_send, 0, sizeof(mode_send));
    
    for(i=0; i<disp_class_list[fbid].disp_mode_length; i++)
    {
        //strncpy(temp_mode+pointer, disp_mode_list[i].mode, strlen(disp_mode_list[i].mode)-1);
        //pointer = pointer +  strlen(disp_mode_list[i].mode) -1;
        //strncpy(temp_mode+pointer, " ", 1);
        //pointer = pointer +  1;
        if (g_config_len > 0) {
            int k, j;
            for(k=0; k<g_config_len; k++) {
                if(!strcmp(g_config_mode[k].mode, disp_class_list[fbid].disp_mode_list[i].mode)) {
                    strncpy(temp_mode, disp_class_list[fbid].disp_mode_list[i].mode, strlen(disp_class_list[fbid].disp_mode_list[i].mode)-1);
                    temp_mode[strlen(disp_class_list[fbid].disp_mode_list[i].mode)-1] = '\0';
                    for(j=0; j<num_send; j++) {
                        if(!strcmp(mode_send[j], temp_mode)) {
                            break;
                        }
                    }
                    if(j == num_send) {
                        strcpy(mode_send[num_send], temp_mode);
                        num_send++;
                        send_msg_with_code(InterfaceListResult, temp_mode, -1);
                    }
                }
            }
        }
        else {
            strncpy(temp_mode, disp_class_list[fbid].disp_mode_list[i].mode, strlen(disp_class_list[fbid].disp_mode_list[i].mode)-1);
            temp_mode[strlen(disp_class_list[fbid].disp_mode_list[i].mode)-1] = '\0';
            send_msg_with_code(InterfaceListResult, temp_mode, -1);
        }
    }
    
    //temp_mode[pointer] = '\0';
    //LOGW("modelist = %s", temp_mode);
    
    send_msg_with_code(CommandOkay, "Interface list completed", -1);
    return temp_mode;    
}


int disp_set_disp_mode(int fbid, char *cmd)
{
    LOGI("disp_set_disp_mode %s", cmd);
    
    char *p = (char *)cmd;
    char *start;
    if(!p)  goto set_out;

	while(p[0])
	{
	   if(p[0] == ' ')
	   {
	       start = p+1;
	       break;
       }
       p++;	       
	}
    
    LOGW("disp_mode %s ",start);

set_out:
    send_msg_with_code(CommandOkay, "mode have been set", -1);
    return 0;
}




