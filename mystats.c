/*
 * mystats.c -- part of mystats.mod
 * Copyright (C) 2003  Douglas Cau <douglas@cau.se>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Sep 2010: updated by pseudo to run on eggdrop 1.8.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <mysql.h>

#include "../module.h"
#include "../irc.mod/irc.h"
#include "../channels.mod/channels.h"
#include "../server.mod/server.h"

#define MODULE_NAME "mystats"
#define MAKING_MYSTATS

#include "mystats.h"
#include "language.h"

#undef global
static Function *global = NULL, *server_funcs = NULL,
                *irc_funcs = NULL, *channels_funcs = NULL;

#include "settings.c"
#include "common.c"
#include "users.c"
#include "chans.c"
#include "sensors.c"
#include "triggers.c"
#include "public.c"


static int mystats_hook_rehash() {
    char SQL_NEW[256];
    int reconnect = 1, i;
    char *s = CAT_SHOW, *c;
    Context;

    /* Reset scats */
    for (i = 0; i < 10; i++)
        mystats_cat_show[i] = 0;

    /* Load scats to be shown by !stats */
    while (s && strlen(s)) {
        c = newsplit(&s);

        for (i = 0; i < 10; i++) {
            if (strcmp(mystats_cat_cshow[i], c)) {
                mystats_cat_show[i] = 1;
                break;
            }
        }
    }

    /* Load default values*/
    if (!strlen(CAT_SHOW)) {
        mystats_cat_show[1] = 1;
        mystats_cat_show[3] = 1;
        mystats_cat_show[4] = 1;
        mystats_cat_show[7] = 1;
        mystats_cat_show[8] = 1;
    }

    /* Check if we need to reconnect */
    if (strlen(SQL_CASHE)) {
        sprintf(SQL_NEW, "%s:%s:%s:%s", SQL_HOST, SQL_USER, SQL_PASS, SQL_DBASE);

        if (!strcmp(SQL_NEW, SQL_CASHE))
            reconnect = 0;
    }

    /* Close if we have an open connection */
    if (reconnect && strlen(SQL_CASHE)) {
        putlog(LOG_MISC, "*", "MyStats: %s", MYSTATS_SQL_CLOSE);
        mysql_close(&mysql);
	}

    /* Open fresh connection if we need to */
    if (reconnect) {
        mysql_init(&mysql);

        if (!mysql_real_connect(&mysql, SQL_HOST, SQL_USER, SQL_PASS, SQL_DBASE, 0, NULL, 0)) {
            putlog(LOG_MISC, "*", "MyStats: %s", MYSTATS_ERR_NOSQL);
            return 0;
        }

        putlog(LOG_MISC, "*", "MyStats: %s", MYSTATS_SQL_OPEN);
        sprintf(SQL_CASHE, "%s:%s:%s:%s", SQL_HOST, SQL_USER, SQL_PASS, SQL_DBASE);
    }

    return 0;
}

static int mystats_hook_daily() {
    Context;

    /* Cleanup words only used once, and not used for three days */
    sql_query("DELETE FROM %s_words WHERE count = 1 "
              "AND TO_DAYS(NOW()) - TO_DAYS(last) < 4", SQL_PREFIX);

    sql_query("OPTIMIZE TABLE %s_chans, %s_hosts, %s_online, %s_stats, "
              "%s_users, %s_words", SQL_PREFIX, SQL_PREFIX, SQL_PREFIX,
              SQL_PREFIX, SQL_PREFIX, SQL_PREFIX);

    return 0;
}

static int mystats_hook_minutely() {
    struct chanset_t *c;
    memberlist *m;
    MYSQL_RES *result;
    MYSQL_ROW row;
    int *users;
    Context;

    /* Add a minute to all online users */
    sql_query("SELECT cid, uid FROM %s_online", SQL_PREFIX);
    result = mysql_store_result(&mysql);
    if (!result)
        return 0;

    users = nmalloc((result->row_count + 1) * 2);
    users[0] = -1;
    while ((row = mysql_fetch_row(result))) {
        if (mystats_dupe(atoi(row[1]), atoi(row[0]), users))
            continue;

        sql_query("UPDATE %s_stats SET online = online+1 WHERE cid = '%s' "
                  "AND uid = '%s' ", SQL_PREFIX, row[0], row[1]);
    }

    mysql_free_result(result);
    nfree(users);

    /* Join users already online */
    sql_query("SELECT cid, name FROM %s_chans WHERE status = '1'", SQL_PREFIX);
    result = mysql_store_result(&mysql);
    if (!result)
        return 0;

    if(!result->row_count) {
        mysql_free_result(result);
        return 0;
    }

    while ((row = mysql_fetch_row(result))) {
        c = findchan_by_dname(row[1]);
        sql_query("UPDATE %s_chans SET status = '2', topic = '%s', "
                  "setby = 'bot on join', seton = '%d' WHERE cid = '%s'",
                  SQL_PREFIX, c->channel.topic?c->channel.topic:"",
                  (int)time(NULL), row[0]);

        for (m = c->channel.member; m && m->nick[0]; m = m->next) {
            if (match_my_nick(m->nick))
                sql_query("INSERT INTO %s_online (nick, cid, signon) "
                          "VALUES ('%s', '%d', '%d')", SQL_PREFIX,
                          m->nick, mystats_chan_byname(row[1]), (int)time(NULL));
            else
                mystats_join(m->nick, m->userhost, m->user?m->user->handle:"*", row[1]);

            if (chan_hasop(m) || chan_fakeop(m))
                mystats_mode("", "", "*", row[1], "+o", m->nick);
            else if (chan_hashalfop(m) || chan_fakehalfop(m))
                mystats_mode("", "", "*", row[1], "+h", m->nick);
            else if (chan_hasvoice(m))
                mystats_mode("", "", "*", row[1], "+v", m->nick);
        }
    }

    mysql_free_result(result);

    return 0;
}

static int mystats_setup(char *mod) {
    p_tcl_bind_list H_temp;
    Context;

    if((H_temp = find_bind_table("pub")))
        add_builtins(H_temp, mystats_cmd_tbl);

    return 0;
}

static cmd_t mystats_load_tbl[] = {
    {"irc",	"",		mystats_setup,	0},
    {NULL,	NULL,	0,					0}
};

static int mystats_expmem() {
    Context;
    /* No memory is beeing allocated */

    return 0;
}

static void mystats_report(int idx, int details) {
    Context;
    /* FIXME: Maybe display some nice info on what we'r doing? */

    return;
}

static char *mystats_close() {
    MYSQL_RES *result;
    MYSQL_ROW row;
    p_tcl_bind_list	H_temp;
    Context;

    /* Unload language and helpfile */
    del_lang_section(MODULE_NAME);
    rem_help_reference(MODULE_NAME ".help");

    /* Unload triggers */
    rem_builtins(H_dcc, mystats_dcc_tbl);
    rem_builtins(H_load, mystats_load_tbl);
    if((H_temp = find_bind_table("pub")))
        rem_builtins(H_temp, mystats_cmd_tbl);

    /* Unload sensors */
    rem_builtins(H_join, mystats_join_tbl);
    rem_builtins(H_part, mystats_part_tbl);
    rem_builtins(H_sign, mystats_part_tbl);
    rem_builtins(H_pubm, mystats_pubm_tbl);
    rem_builtins(H_mode, mystats_mode_tbl);
    rem_builtins(H_kick, mystats_kick_tbl);
    rem_builtins(H_nick, mystats_nick_tbl);
    rem_builtins(H_topc, mystats_topc_tbl);
    rem_builtins(H_ctcp, mystats_ctcp_tbl);

    /* Unload hooks */
    del_hook(HOOK_REHASH, mystats_hook_rehash);
    del_hook(HOOK_MINUTELY, mystats_hook_minutely);
    del_hook(HOOK_DAILY, mystats_hook_daily);

    /* Unload eggdrop.conf options */
    rem_tcl_strings(mystats_tcl_strings);
    rem_tcl_ints(mystats_tcl_ints);

    /* Disconnect from MySQL */
    sql_query("SELECT name FROM %s_chans WHERE status > 0");
    result = mysql_store_result(&mysql);

    if (result) {
        while ((row = mysql_fetch_row(result)))
            mystats_chan_part(row[0]);

        mysql_free_result(result);
    }
    mysql_close(&mysql);

    module_undepend(MODULE_NAME);
    return NULL;
}


EXPORT_SCOPE char *mystats_start();

static Function mystats_table[] = {
    (Function) mystats_start,
    (Function) mystats_close,
    (Function) mystats_expmem,
    (Function) mystats_report,
};

char *mystats_start(Function *global_funcs) {
    int no_settings = 0;
    struct chanset_t *chan;
    global = global_funcs;
    Context;

    /* Check dependencies */
    module_register(MODULE_NAME, mystats_table, 1, 77);
    if (!module_depend(MODULE_NAME, "eggdrop", 108, 4)) {
      module_undepend(MODULE_NAME);
      return "This module requires Eggdrop 1.8.4 or later.";
    }

    if(!(irc_funcs = module_depend(MODULE_NAME, "irc", 1, 5))) {
        module_undepend(MODULE_NAME);
        return "This module requires the irc module";
    }

    if(!(server_funcs=module_depend(MODULE_NAME, "server", 1, 4))) {
        module_undepend(MODULE_NAME);
        return "This module requires the server module.";
    }

    if(!(channels_funcs=module_depend(MODULE_NAME, "channels", 1, 2))) {
        module_undepend(MODULE_NAME);
        return "This module requires the channels module.";
    }

    /* Load eggdrop.conf options */
    add_tcl_strings(mystats_tcl_strings);
    if (!strlen(SQL_USER))
        no_settings = 1;
    else if (!strlen(SQL_PASS))
        no_settings = 1;
    else if (!strlen(SQL_HOST))
        no_settings = 1;
    else if (!strlen(SQL_DBASE))
        no_settings = 1;
	else if (!strlen(SQL_PREFIX))
        no_settings = 1;
    if (no_settings) {
        rem_tcl_strings(mystats_tcl_strings);
        module_undepend(MODULE_NAME);
        return "Mystats: Missing settings in your config file, please read the README.";
    }
    add_tcl_ints(mystats_tcl_ints);

    /* Add hooks */
    add_hook(HOOK_REHASH, mystats_hook_rehash);
    add_hook(HOOK_MINUTELY, mystats_hook_minutely);
    add_hook(HOOK_DAILY, mystats_hook_daily);

    /* Add sensors */
    add_builtins(H_join, mystats_join_tbl);
    add_builtins(H_part, mystats_part_tbl);
    add_builtins(H_sign, mystats_part_tbl);
    add_builtins(H_pubm, mystats_pubm_tbl);
    add_builtins(H_mode, mystats_mode_tbl);
    add_builtins(H_kick, mystats_kick_tbl);
    add_builtins(H_nick, mystats_nick_tbl);
    add_builtins(H_topc, mystats_topc_tbl);
    add_builtins(H_ctcp, mystats_ctcp_tbl);

    /* Add triggers */
    add_builtins(H_dcc, mystats_dcc_tbl);
    add_builtins(H_load, mystats_load_tbl);

    /* Load language file and helpfile */
    add_lang_section(MODULE_NAME);
    add_help_reference(MODULE_NAME ".help");

    /* Setup a MySQL connection */
    strcpy(SQL_CASHE, "");
    mystats_hook_rehash();
    mystats_hook_daily();
    sql_query("UPDATE %s_chans SET status = '0' WHERE status = '2'", SQL_PREFIX);
    sql_query("DELETE FROM %s_online", SQL_PREFIX);
    mystats_setup(0);

    /* Join channels (when loading the module manually) */
    for (chan = chanset; chan; chan = chan->next)
         mystats_chan_join(chan->dname);
	
    return NULL;
}

