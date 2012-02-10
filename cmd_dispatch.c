
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

#include <unistd.h>
#include <errno.h>

#include "dispd.h"
#include "cmd_dispatch.h"
#include "disp.h"
#include "dispmgr.h"

struct cmd_dispatch {
    char *cmd;
    int (* dispatch) (int argc, char **argv);
};

static void dispatch_cmd(char *cmd);
static int do_send_disp_status(int argc, char **argv);
static int do_set_disp_enable(int argc, char **argv);
static char* do_get_disp_modelist(int argc, char **argv);
static int do_set_disp_mode(int argc, char **argv);

#define DISPD_CMD_ENABLE_DISP         "enable_display"
#define DISPD_CMD_DISABLE_DISP        "disable_display"
#define DISPD_CMD_SEND_DISP_STATUS    "send_display_status"
#define DISPD_CMD_GET_MODE_LIST       "get_display_modelist"
#define DISPD_CMD_SET_MODE            "set_display_mode"

static struct cmd_dispatch dispatch_table[] = {
    { DISPD_CMD_ENABLE_DISP,      do_set_disp_enable },
    { DISPD_CMD_DISABLE_DISP,     do_set_disp_enable },
    { DISPD_CMD_SEND_DISP_STATUS, do_send_disp_status },
    { DISPD_CMD_GET_MODE_LIST,    do_get_disp_modelist },
	{ DISPD_CMD_SET_MODE,         do_set_disp_mode },
    { NULL, NULL }
};

int process_framework_command(int socket)
{
    int rc;
    char buffer[101];

    if ((rc = read(socket, buffer, sizeof(buffer) -1)) < 0) {
        LOGE("Unable to read framework command (%s)", strerror(errno));
        return -errno;
    } else if (!rc)
        return -ECONNRESET;

    int start = 0;
    int i;

    buffer[rc] = 0;

    for (i = 0; i < rc; i++) {
        if (buffer[i] == 0) {
            dispatch_cmd(buffer + start);
            start = i + 1;
        }
    }
    return 0;
}
/*
static void dispatch_cmd(char *cmd)
{
    struct cmd_dispatch *c;

    LOG_DISP("dispatch_cmd(%s):", cmd);

    for (c = dispatch_table; c->cmd != NULL; c++) {
        if (!strncmp(c->cmd, cmd, strlen(c->cmd))) {
            c->dispatch(cmd);
            return;
        }
    }

    LOGE("No cmd handlers defined for '%s'", cmd);
}
*/
static int do_send_disp_status(int argc, char **argv)
{
    return disp_send_status();
}

static int do_set_disp_enable(int argc, char **argv)
{
    // get the fbid;
    int fbid = (char)argv[1][0]-48;
    if (!strcmp(argv[0], DISPD_CMD_ENABLE_DISP))
        return dispmgr_enable_disp(fbid,true);

    return dispmgr_enable_disp(fbid, false);
}

static char* do_get_disp_modelist(int argc, char **argv)
{
    // get the fbid;
    int fbid = (char)argv[1][0]-48;
	return disp_get_disp_modelist(fbid);
}

static int do_set_disp_mode(int argc, char **argv)
{
    // get the fbid;
    int fbid = (char)argv[1][0]-48;
	return disp_set_disp_mode(fbid, argv[2]);
}


static void dispatch_cmd(char *data) {
    int argc = 0;
    char *argv[CMD_ARGS_MAX];
    char tmp[255];
    char *p = data;
    char *q = tmp;
    char *qlimit = tmp + sizeof(tmp) - 1;
    boolean esc = false;
    boolean quote = false;
    int k;
    int j;
    struct cmd_dispatch *c;
    
    memset(argv, 0, sizeof(argv));
    memset(tmp, 0, sizeof(tmp));
    while(*p) {
        if (*p == '\\') {
            if (esc) {
                if (q >= qlimit)
                    goto overflow;
                *q++ = '\\';
                esc = false;
            } else
                esc = true;
            p++;
            continue;
        } else if (esc) {
            if (*p == '"') {
                if (q >= qlimit)
                    goto overflow;
                *q++ = '"';
            } else if (*p == '\\') {
                if (q >= qlimit)
                    goto overflow;
                *q++ = '\\';
            } else {
                send_msg_with_code(500, "Unsupported escape sequence", -1);
                goto out;
            }
            p++;
            esc = false;
            continue;
        }

        if (*p == '"') {
            if (quote)
                quote = false;
            else
                quote = true;
            p++;
            continue;
        }

        if (q >= qlimit)
            goto overflow;
        *q = *p++;
        if (!quote && *q == ' ') {
            *q = '\0';
            if (argc >= CMD_ARGS_MAX)
                goto overflow;
            argv[argc++] = strdup(tmp);
            memset(tmp, 0, sizeof(tmp));
            q = tmp;
            continue;
        }
        q++;
    }

    *q = '\0';
    if (argc >= CMD_ARGS_MAX)
        goto overflow;
    argv[argc++] = strdup(tmp);
#if 0
    for (k = 0; k < argc; k++) {
        SLOGD("arg[%d] = '%s'", k, argv[k]);
    }
#endif

    if (quote) {
        send_msg_with_code(500, "Unclosed quotes error", -1);
        goto out;
    }

    LOGW("dispatch cmd %s",argv[0]);

    for (c = dispatch_table; c->cmd != NULL; c++) {
        if (!strncmp(c->cmd, argv[0], strlen(c->cmd))) {
            c->dispatch(argc,argv);
            goto out;
        }
    }
    
    send_msg_with_code(500, "Command not recognized", -1);
out:

    for (j = 0; j < argc; j++)
        free(argv[j]);
    return;

overflow:
    send_msg_with_code(500, "Command too long", -1);
    goto out;
}

