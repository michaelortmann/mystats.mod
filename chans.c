/*
 * chans.c -- part of mystats.mod
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

static int mystats_chan_byname(char *chan) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    int cid;

    /* Get channel id by name */
    sql_query("SELECT cid FROM %s_chans WHERE name = '%s'", SQL_PREFIX, chan);

    result = mysql_store_result(&mysql);
    if (!result)
        return 0;

    if(!result->row_count) {
        mysql_free_result(result);
        return 0;
    }

    row = mysql_fetch_row(result);
    cid = atoi(row[0]);

    mysql_free_result(result);
    return cid;
}

static void mystats_chan_join(char *name) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    int status, cid;

    sql_query("SELECT status FROM %s_chans WHERE name = '%s'", SQL_PREFIX, name);

    result = mysql_store_result(&mysql);
    if (!result)
        return;

    if(!result->row_count) {
        mysql_free_result(result);
        sql_query("INSERT INTO %s_chans (name) VALUES ('%s')", SQL_PREFIX, name);
        return;
    }

    row = mysql_fetch_row(result);
    mysql_free_result(result);
    status = atoi(row[0]);
    if (status == -1)
        return;

    /* Clean up and prepare for joining already joined users */
    cid = mystats_chan_byname(name);
    sql_query("DELETE FROM %s_online WHERE cid = '%d'", SQL_PREFIX, cid);
    sql_query("UPDATE %s_chans SET status = 1 WHERE cid = '%d'", SQL_PREFIX, cid);

    return;
}

static int mystats_chan_status(int cid) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    int status;

    /* Get channel status */
    sql_query("SELECT status FROM %s_chans WHERE cid = '%d'", SQL_PREFIX, cid);

    result = mysql_store_result(&mysql);
    if (!result)
        return -1;

    if(!result->row_count) {
        mysql_free_result(result);
        return -1;
    }

    row = mysql_fetch_row(result);
    status = atoi(row[0]);

    mysql_free_result(result);
    return status;
}

static void mystats_chan_part(char *chan) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    int cid;

    /* Clean up and set correct channel status */
    cid = mystats_chan_byname(chan);
    sql_query("SELECT nick FROM %s_online WHERE cid = '%d'", SQL_PREFIX, cid);

    result = mysql_store_result(&mysql);
    if (!result)
        return;

    if(!result->row_count) {
        mysql_free_result(result);
        return;
    }

    while ((row = mysql_fetch_row(result))) {
        if (match_my_nick(row[0]))
            sql_query("DELETE FROM %s_online WHERE nick = '%s' AND cid = '%d'",
                      SQL_PREFIX, row[0], cid);
        else
            mystats_part(row[0], "", "", chan);
    }

    sql_query("UPDATE %s_chans SET status = '0' WHERE cid = '%d'", SQL_PREFIX, cid);

    mysql_free_result(result);
    return;
}

