/*
 * users.c -- part of mystats.mod
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

static int mystats_user_bymask(char *host, char *nick) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    int uid = 0;
    char *longmask;

    /* Get user id by hostmask and nick */
    sql_query("SELECT mask, uid FROM %s_hosts", SQL_PREFIX);

    result = mysql_store_result(&mysql);
    if (!result)
        return 0;

    if(!result->row_count) {
        mysql_free_result(result);
        return 0;
    }

    longmask = nmalloc(strlen(host) + strlen(nick) + 2);
    sprintf(longmask, "%s!%s", nick, host);
    while ((row = mysql_fetch_row(result))) {
        if (wild_match(row[0], longmask)) {
            uid = atoi(row[1]);
            break;
        }
    }

    mysql_free_result(result);
    nfree(longmask);
    return uid;
}

static int mystats_user_bychan(char *nick, int cid) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    int uid;

    /* Get user id by channel and nick */
    sql_query("SELECT uid FROM %s_online WHERE nick = '%s' AND cid = '%d'",
              SQL_PREFIX, nick, cid);

    result = mysql_store_result(&mysql);
    if (!result)
        return 0;

    if(!result->row_count) {
        mysql_free_result(result);
        return 0;
    }

    row = mysql_fetch_row(result);
    uid = atoi(row[0]);

    mysql_free_result(result);
    return uid;
}

static int mystats_user_isbot(int uid) {
    MYSQL_RES *result;
    MYSQL_ROW row;

    /* Get user type */
    sql_query("SELECT type FROM %s_users WHERE uid = '%d'", SQL_PREFIX, uid);

    result = mysql_store_result(&mysql);
    if (!result)
        return 0;

    if(!result->row_count) {
        mysql_free_result(result);
        return 0;
    }

    row = mysql_fetch_row(result);
    mysql_free_result(result);
    return (atoi(row[0]) == 1);
}

static void mystats_user_stats(int uid, int cid) {
    MYSQL_RES *result;

    if (mystats_user_isbot(uid))
        return;

    /* Create a stats record if there is none */
    sql_query("SELECT * FROM %s_stats WHERE uid = '%d' AND cid = '%d'",
              SQL_PREFIX, uid, cid);

    result = mysql_store_result(&mysql);
    if (!result)
        return;

    if(!result->row_count)
        sql_query("INSERT INTO %s_stats (uid, cid) VALUES ('%d', '%d')",
                  SQL_PREFIX, uid, cid);

    mysql_free_result(result);
    return;
}

static void mystats_user_synchost(char *host, int uid, int idx) {
    struct chanset_t *c;
    memberlist *m;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char *longmask;

    /* Get all channels to sync host in */
    sql_query("SELECT name FROM %s_chans WHERE status = '2'", SQL_PREFIX);

    result = mysql_store_result(&mysql);
    if (!result)
    return;

    if(!result->row_count) {
        mysql_free_result(result);
        return;
    }

    while ((row = mysql_fetch_row(result))) {
        c = findchan_by_dname(row[0]);

        /* Go through everyone to se if their host matches */
        for (m = c->channel.member; m && m->nick[0]; m = m->next) {
            longmask = nmalloc(strlen(m->userhost) + strlen(m->nick) + 2);
            sprintf(longmask, "%s!%s", m->nick, m->userhost);

            if (!match_my_nick(m->nick) && wild_match(host, longmask)) {
                sql_query("UPDATE %s_online SET uid = '%d' WHERE nick = '%s'",
                          SQL_PREFIX, uid, m->nick);
                dprintf(idx, MYSTATS_USR_CHHOST, m->nick, row[0],
                        uid?"":" not", uid?mystats_user_cname(uid):"an user anymore");
            }

            nfree(longmask);
        }
    }

    mysql_free_result(result);
    return;
}

static int mystats_user_mode(char *nick, int cid) {
    MYSQL_RES *result;
    MYSQL_ROW row;

    sql_query("SELECT mode FROM %s_online WHERE nick = '%s' AND cid = '%d'",
              SQL_PREFIX, nick, cid);
    result = mysql_store_result(&mysql);
    if (!result)
        return 0;

    if(!result->row_count) {
        mysql_free_result(result);
        return 0;
    }

    row = mysql_fetch_row(result);
    mysql_free_result(result);
    return atoi(row[0]);
}

static memberlist *mystats_user_getm(struct chanset_t *c, char *nick) {
    memberlist *m;

    for (m = c->channel.member; m && m->nick[0]; m = m->next)
        if (!strcmp(nick, m->nick))
            return m;

    return NULL;
}

static char *mystats_user_cname(int uid) {
    MYSQL_RES *result;
    MYSQL_ROW row;

    sql_query("SELECT handle FROM %s_users WHERE uid = '%d'", SQL_PREFIX, uid);
    result = mysql_store_result(&mysql);
    if (!result)
        return "*";

    if(!result->row_count) {
        mysql_free_result(result);
        return "*";
    }

    row = mysql_fetch_row(result);
    mysql_free_result(result);
    return row[0];
}

static int mystats_user_byhand(char *hand) {
    MYSQL_RES *result;
    MYSQL_ROW row;

    sql_query("SELECT uid FROM %s_users WHERE handle = '%s'", SQL_PREFIX, hand);
    result = mysql_store_result(&mysql);
    if (!result)
        return 0;

    if(!result->row_count) {
        mysql_free_result(result);
        return 0;
    }

    row = mysql_fetch_row(result);
    mysql_free_result(result);
    return atoi(row[0]);
}

