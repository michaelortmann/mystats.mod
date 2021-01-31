/*
 * triggers.c -- part of mystats.mod
 * Copyright (C) 2003  Douglas Cau <douglas@cau.se>
 *
 *  $Id: triggers.c 57 2004-03-23 20:10:09Z cau $
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

static int mystats_dcc_adduser(struct userrec *u, int idx, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    char *handle, *host;
    Context;

    handle = newsplit(&par);
    host = newsplit(&par);

    /* Check syntax */
    if (!handle[0]) {
        dprintf(idx, "%s\n", MYSTATS_USR_ADDUSAGE);
        return 0;
    }

    /* Check that user doesnt already exist */
    sql_query("SELECT * FROM %s_users WHERE handle = '%s'", SQL_PREFIX, handle);

    result = mysql_store_result(&mysql);
    if (!result) {
        dprintf(idx, "%s\n", MYSTATS_SQL_ERRRES);
        return 0;
    }

    if(result->row_count) {
        mysql_free_result(result);
        dprintf(idx, "%s\n", MYSTATS_USR_EXISTS);
        return 0;
    }

    /* Now add user */
    mysql_free_result(result);
    sql_query("INSERT INTO %s_users (handle) VALUES ('%s')", SQL_PREFIX, handle);
    dprintf(idx, MYSTATS_USR_ADDED, handle);

    /* Add host if specified */
    if (host[0]) {
        sql_query("SELECT uid FROM %s_users WHERE handle = '%s'", SQL_PREFIX, handle);

        result = mysql_store_result(&mysql);
        if (!result) {
            dprintf(idx, "%s\n", MYSTATS_SQL_ERRRES);
            return 0;
        }

        if(!result->row_count) {
            mysql_free_result(result);
            return 0;
        }

        row = mysql_fetch_row(result);

        sql_query("INSERT INTO %s_hosts (mask, uid) VALUES ('%s', '%s')",
                  SQL_PREFIX, host, row[0]);
        mystats_user_synchost(host, atoi(row[0]), idx);
        dprintf(idx, MYSTATS_HST_ADDED, host, handle);
        mysql_free_result(result);
    }

    putlog(LOG_CMDS, "*", "#%s# +suser", dcc[idx].nick);
    return 0;
}

static int mystats_dcc_deluser(struct userrec *u, int idx, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    char *handle;
    Context;

    handle = newsplit(&par);

    /* Check syntax */
    if (!handle[0]) {
        dprintf(idx, "%s\n", MYSTATS_USR_DELUSAGE);
        return 0;
    }

    /* Check if there is such a user */
    sql_query("SELECT uid FROM %s_users WHERE handle = '%s'", SQL_PREFIX, handle);

    result = mysql_store_result(&mysql);
    if (!result) {
        dprintf(idx, "%s\n", MYSTATS_SQL_ERRRES);
        return 0;
    }

    if(!result->row_count) {
        mysql_free_result(result);
        dprintf(idx, "%s\n", MYSTATS_USR_NOUSER);
        return 0;
    }

    row = mysql_fetch_row(result);

    /* Check if user is a bot */
    if (mystats_user_isbot(atoi(row[0]))) {
        mysql_free_result(result);
        dprintf(idx, "%s\n", MYSTATS_USR_ISBOT);
        return 0;
    }

    /* Clean up all records */
    sql_query("DELETE FROM %s_users WHERE uid = '%s'", SQL_PREFIX, row[0]);
    sql_query("DELETE FROM %s_hosts WHERE uid = '%s'", SQL_PREFIX, row[0]);
    sql_query("DELETE FROM %s_stats WHERE uid = '%s'", SQL_PREFIX, row[0]);
    sql_query("DELETE FROM %s_words WHERE uid = '%s'", SQL_PREFIX, row[0]);
    sql_query("UPDATE %s_online SET uid = 0 WHERE uid = '%s'", SQL_PREFIX, row[0]);

    mysql_free_result(result);
    dprintf(idx, MYSTATS_USR_DELETED, handle);
    putlog(LOG_CMDS, "*", "#%s# -suser", dcc[idx].nick);
    return 0;
}

static int mystats_dcc_addhost(struct userrec *u, int idx, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    char *handle, *host;
    Context;

    handle = newsplit(&par);
    host = newsplit(&par);

    /* Check syntax */
    if (!handle[0] || !host[0]) {
        dprintf(idx, "%s\n", MYSTATS_HST_ADDUSAGE);
        return 0;
    }

    /* Check if there is such a user */
    sql_query("SELECT uid FROM %s_users WHERE handle = '%s'", SQL_PREFIX, handle);

    result = mysql_store_result(&mysql);
    if (!result) {
        dprintf(idx, "%s\n", MYSTATS_SQL_ERRRES);
        return 0;
    }

    if(!result->row_count) {
        mysql_free_result(result);
        dprintf(idx, "%s\n", MYSTATS_USR_NOUSER);
        return 0;
    }

    row = mysql_fetch_row(result);
    sql_query("INSERT INTO %s_hosts (mask, uid) VALUES ('%s', '%s')",
              SQL_PREFIX, host, row[0]);
    mystats_user_synchost(host, atoi(row[0]), idx);
    dprintf(idx, MYSTATS_HST_ADDED, host, handle);

    mysql_free_result(result);
    putlog(LOG_CMDS, "*", "#%s# +shost", dcc[idx].nick);
    return 0;
}

static int mystats_dcc_delhost(struct userrec *u, int idx, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    char *host;
    Context;

    host = newsplit(&par);

    /* Check syntax */
    if (!host[0]) {
        dprintf(idx, "%s\n", MYSTATS_HST_DELUSAGE);
        return 0;
    }

    /* Chek if there is such a host */
    sql_query("SELECT uid FROM %s_hosts WHERE mask = '%s'", SQL_PREFIX, host);

    result = mysql_store_result(&mysql);
    if (!result) {
        dprintf(idx, "%s\n", MYSTATS_SQL_ERRRES);
        return 0;
    }

    if(!result->row_count) {
        mysql_free_result(result);
        dprintf(idx, "%s\n", MYSTATS_HST_NOHOST);
        return 0;
    }

    row = mysql_fetch_row(result);
    mysql_free_result(result);

    /* Delete host and sync channels */
    sql_query("DELETE FROM %s_hosts WHERE mask = '%s'", SQL_PREFIX, host);
    mystats_user_synchost(host, 0, idx);
    dprintf(idx, MYSTATS_HST_DELETED, host);

    putlog(LOG_CMDS, "*", "#%s# -shost", dcc[idx].nick);
    return 0;
}

static int mystats_dcc_match(struct userrec *u, int idx, char *par) {
    MYSQL_RES *result;
    MYSQL_RES *result_main;
    MYSQL_ROW row;
    MYSQL_ROW host;
    Context;

    if (!strlen(par)) {
        dprintf(idx, "%s\n", MYSTATS_USR_MATCHUSAGE);
        return 0;
    }

    dprintf(idx, MYSTATS_USR_MATCHING, par);
    sql_query("SELECT uid, handle FROM %s_users WHERE handle LIKE '%s'", SQL_PREFIX, par);

    result_main = mysql_store_result(&mysql);
    if (!result_main) {
        dprintf(idx, "%s\n", MYSTATS_SQL_ERRRES);
        return 0;
    }

    while ((row = mysql_fetch_row(result_main))) {
        sql_query("SELECT mask FROM %s_hosts WHERE uid = '%s'", SQL_PREFIX, row[0]);

        result = mysql_store_result(&mysql);
        if (!result)
            break;

        dprintf(idx, "%s\n", row[1]);
        while ((host = mysql_fetch_row(result))) {
            dprintf(idx, "    %s\n", host[0]);
        }

        mysql_free_result(result);
    }

    dprintf(idx, MYSTATS_USR_MATCHES, result_main->row_count);
    mysql_free_result(result_main);
    putlog(LOG_CMDS, "*", "#%s# smatch %s", dcc[idx].nick, par);
    return 0;
}

static int mystats_dcc_addchan(struct userrec *u, int idx, char *par) {
    struct chanset_t *c;
    char *chan;
    int cid;
    Context;

    chan = newsplit(&par);
    cid = mystats_chan_byname(chan);

    /* Check syntax */
    if (!chan[0]) {
        dprintf(idx, "%s\n", MYSTATS_CHN_ADDUSAGE);
        return 0;
    }

    /* Check if I'm on the channel */
    if (!(c = findchan_by_dname(chan)) || !cid) {
        dprintf(idx, "%s\n", MYSTATS_CHN_NOTON);
        return 0;
    }

    sql_query("UPDATE %s_chans SET status = '1' WHERE cid = '%d'", SQL_PREFIX, cid);
    sql_query("DELETE FROM %s_online WHERE cid = '%d'", SQL_PREFIX, cid);

    dprintf(idx, MYSTATS_CHN_START, chan);
    putlog(LOG_CMDS, "*", "#%s# +schan %s", dcc[idx].nick, chan);
    return 0;
}

static int mystats_dcc_delchan(struct userrec *u, int idx, char *par) {
    struct chanset_t *c;
    char *chan;
    int cid;
    Context;

    chan = newsplit(&par);
    cid = mystats_chan_byname(chan);

    /* Check syntax */
    if (!chan[0]) {
        dprintf(idx, "%s\n", MYSTATS_CHN_DELUSAGE);
        return 0;
    }

    /* Check if I'm on the channel */
    if (!(c = findchan_by_dname(chan)) || !cid) {
        dprintf(idx, "%s\n", MYSTATS_CHN_NOTON);
        return 0;
    }

    /* Change channel status and part everyone in the channel */
    mystats_chan_part(chan);
    sql_query("UPDATE %s_chans SET status = -1 WHERE cid = '%d'", SQL_PREFIX, cid);

    dprintf(idx, MYSTATS_CHN_STOP, chan);
    putlog(LOG_CMDS, "*", "#%s# -schan %s", dcc[idx].nick, chan);
    return 0;
}

static int mystats_dcc_purge(struct userrec *u, int idx, char *par) {
    int cid, status;
    char *chan;
    Context;

    chan = newsplit(&par);
    cid = mystats_chan_byname(chan);

    /* Check syntax */
    if (!chan[0]) {
        dprintf(idx, "%s\n", MYSTATS_CHN_PURGEUSAGE);
        return 0;
    }

    status = mystats_chan_status(cid);

    if (status != -1) {
        dprintf(idx, "%s\n", MYSTATS_CHN_PURGEERR);
        return 0;
    }

    /* Delete all stats */
    sql_query("DELETE FROM %s_stats WHERE cid = '%d'", SQL_PREFIX, cid);

    dprintf(idx, MYSTATS_CHN_PURGED, chan);
    putlog(LOG_CMDS, "*", "#%s# -spurge %s", dcc[idx].nick, chan);
    return 0;
}

static int mystats_dcc_addbot(struct userrec *u, int idx, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    char *handle, *host;
    Context;

    handle = newsplit(&par);
    host = newsplit(&par);

    /* Check syntax */
    if (!handle[0]) {
        dprintf(idx, "%s\n", MYSTATS_BOT_ADDUSAGE);
        return 0;
    }

    /* Check that user doesnt already exist */
    sql_query("SELECT * FROM %s_users WHERE handle = '%s'", SQL_PREFIX, handle);

    result = mysql_store_result(&mysql);
    if (!result) {
        dprintf(idx, "%s\n", MYSTATS_SQL_ERRRES);
        return 0;
    }

    if(result->row_count) {
        mysql_free_result(result);
        dprintf(idx, "%s\n", MYSTATS_USR_EXISTS);
        return 0;
    }

    /* Now add user */
    mysql_free_result(result);
    sql_query("INSERT INTO %s_users (handle, type) VALUES ('%s', 1)", SQL_PREFIX, handle);
    dprintf(idx, MYSTATS_BOT_ADDED, handle);

    /* Add host if specified */
    if (host[0]) {
        sql_query("SELECT uid FROM %s_users WHERE handle = '%s'", SQL_PREFIX, handle);

        result = mysql_store_result(&mysql);
        if (!result) {
            dprintf(idx, "%s\n", MYSTATS_SQL_ERRRES);
            return 0;
        }

        if(!result->row_count) {
            mysql_free_result(result);
            return 0;
        }

        row = mysql_fetch_row(result);

        sql_query("INSERT INTO %s_hosts (mask, uid) VALUES ('%s', '%s')",
                  SQL_PREFIX, host, row[0]);
        mystats_user_synchost(host, atoi(row[0]), idx);
        dprintf(idx, MYSTATS_HST_ADDED, host, handle);
        mysql_free_result(result);
    }

    putlog(LOG_CMDS, "*", "#%s# +sbot", dcc[idx].nick);
    return 0;
}

static int mystats_dcc_delbot(struct userrec *u, int idx, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    char *handle;
    Context;

    handle = newsplit(&par);

    /* Check syntax */
    if (!handle[0]) {
        dprintf(idx, "%s\n", MYSTATS_BOT_DELUSAGE);
        return 0;
    }

    /* Check if there is such a user */
    sql_query("SELECT uid FROM %s_users WHERE handle = '%s'", SQL_PREFIX, handle);

    result = mysql_store_result(&mysql);
    if (!result) {
        dprintf(idx, "%s\n", MYSTATS_SQL_ERRRES);
        return 0;
    }

    if(!result->row_count) {
        mysql_free_result(result);
		dprintf(idx, "%s\n", MYSTATS_USR_NOUSER);
        return 0;
    }

    row = mysql_fetch_row(result);

    /* Check if user is a bot */
    if (!mystats_user_isbot(atoi(row[0]))) {
        mysql_free_result(result);
        dprintf(idx, "%s\n", MYSTATS_BOT_ISUSER);
        return 0;
    }

    /* Clean up all records */
    sql_query("DELETE FROM %s_users WHERE uid = '%s'", SQL_PREFIX, row[0]);
    sql_query("DELETE FROM %s_hosts WHERE uid = '%s'", SQL_PREFIX, row[0]);
    sql_query("DELETE FROM %s_stats WHERE uid = '%s'", SQL_PREFIX, row[0]);
    sql_query("UPDATE %s_online SET uid = 0 WHERE uid = '%s'", SQL_PREFIX, row[0]);

    mysql_free_result(result);
    dprintf(idx, MYSTATS_BOT_DELETED, handle);
    putlog(LOG_CMDS, "*", "#%s# -suser", dcc[idx].nick);
    return 0;
}

static int mystats_dcc_channel(struct userrec *u, int idx, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int wnick, whand, cid;
    char *format, *handle;
    Context;

    wnick = strlen(MYSTATS_LST_NICK);
    whand = strlen(MYSTATS_LST_HAND);

    cid = mystats_chan_byname(par);
    if (mystats_chan_status(cid) < 2) {
        dprintf(idx, "%s\n", MYSTATS_CHN_NOTON);
        return 0;
    }

    format = nmalloc(20);

    sql_query("SELECT mode, nick, uid FROM %s_online WHERE cid = '%d' ORDER BY nick ASC",
              SQL_PREFIX, cid);
    result = mysql_store_result(&mysql);
    if (!result) {
        dprintf(idx, "%s\n", MYSTATS_SQL_ERRRES);
        return 0;
    }

    if(!result->row_count) {
        mysql_free_result(result);
        dprintf(idx, "%s\n", MYSTATS_SQL_ERRRES);
        return 0;
    }

    /* Get longest nickname */
    while ((row = mysql_fetch_row(result))) {
        if (strlen(row[1]) > wnick)
            wnick = strlen(row[1]);
    }

    /* Get longest handle */
    mysql_data_seek(result, 0);
    while ((row = mysql_fetch_row(result))) {
        handle = mystats_user_cname(atoi(row[2]));
        if (strlen(handle) > whand)
            whand = strlen(handle);
    }

    /* Generate format line */
    strcpy(format, "%s%-");
    sprintf(format, "%s%d", format, wnick);
    strcat(format, "s %-");
    sprintf(format, "%s%d", format, whand);
    strcat(format, "s %s\n");

    /* Start going through the list and print out */
    dprintf(idx, format, " ", MYSTATS_LST_NICK, MYSTATS_LST_HAND, MYSTATS_LST_FLAGS);
    mysql_data_seek(result, 0);
    while ((row = mysql_fetch_row(result)))
        dprintf(idx, format, (atoi(row[0]) > 2)?"@":((atoi(row[0]) > 1)?"%":
                (atoi(row[0])?"+":" ")), row[1], mystats_user_cname(atoi(row[2])),
                mystats_user_isbot(atoi(row[2]))?"Bot":"");

    nfree(format);
    mysql_free_result(result);
    dprintf(idx, "%s\n", MYSTATS_LST_END);
    putlog(LOG_CMDS, "*", "#%s# schan", dcc[idx].nick);
    return 0;
}

static cmd_t mystats_dcc_tbl[] = {
    {"+suser",      "m",    mystats_dcc_adduser,    NULL},
    {"-suser",      "m",    mystats_dcc_deluser,    NULL},
    {"+shost",      "m",    mystats_dcc_addhost,    NULL},
    {"-shost",      "m",    mystats_dcc_delhost,    NULL},
    {"smatch",      "m",    mystats_dcc_match,      NULL},
    {"+schan",      "m",    mystats_dcc_addchan,    NULL},
    {"-schan",      "m",    mystats_dcc_delchan,    NULL},
    {"spurge",      "m",    mystats_dcc_purge,      NULL},
    {"+sbot",       "m",    mystats_dcc_addbot,     NULL},
    {"-sbot",       "m",    mystats_dcc_delbot,     NULL},
    {"schannel",    "m",    mystats_dcc_channel,    NULL},
    {NULL,          NULL,   NULL,                   NULL}
};

