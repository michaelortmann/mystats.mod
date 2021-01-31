/*
 * sensors.c -- part of mystats.mod
 * Copyright (C) 2003  Douglas Cau <douglas@cau.se>
 *
 *  $Id: sensors.c 57 2004-03-23 20:10:09Z cau $
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

static int mystats_join(char *nick, char *host, char *handle, char *channel) {
    int cid, uid;
    Context;

    if (match_my_nick(nick)) {
        mystats_chan_join(channel);
        return 0;
    }

    /* Are we monitoring this channel? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) == -1)
        return 0;

    uid = mystats_user_bymask(host, nick);

    sql_query("INSERT INTO %s_online (nick, cid, uid, signon) "
              "VALUES ('%s', '%d', '%d', '%d')",
              SQL_PREFIX, nick, cid, uid, (int)time(NULL));

    if (uid && !mystats_user_isbot(uid)) {
        mystats_user_stats(uid, cid);
        sql_query("UPDATE %s_stats SET joins = joins + 1 "
                  "WHERE uid = '%d' AND cid = '%d'",
                  SQL_PREFIX, uid, cid);
    }

    return 0;
}

static cmd_t mystats_join_tbl[] = {
    {"*",		"",		mystats_join,		NULL},
    {NULL,	NULL,	NULL,						NULL}
};

static int mystats_part(char *nick, char *host, char *handle, char *channel) {
    int uid, cid;
    Context;

    /* Are we monitoring this channel? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) == -1)
        return 0;

    if (match_my_nick(nick)) {
        mystats_chan_part(channel);
        return 0;
    }

    uid = mystats_user_bychan(nick, cid);
    sql_query("DELETE FROM %s_online WHERE nick = '%s' AND cid = '%d'",
              SQL_PREFIX, nick, cid);

    if (uid && !mystats_user_isbot(uid)) {
        mystats_user_stats(uid, cid);
        sql_query("UPDATE %s_stats SET seen = '%d' WHERE uid = '%d' AND cid = '%d'",
                  SQL_PREFIX, (int)time(NULL), uid, cid);
    }

    return 0;
}

static cmd_t mystats_part_tbl[] = {
    {"*",	"",		mystats_part,	NULL},
    {NULL,	NULL,	NULL,				NULL}
};

static int mystats_pubm(char *nick, char *host, char *handle, char *channel, char *text) {
    int cid, uid, words, questions;
    char str[512];
    Context;

    /* Are we monitoring this channel? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) < 2)
        return 0;

    uid = mystats_user_bychan(nick, cid);
    mystats_activity(cid, uid);
    if (uid && !mystats_user_isbot(uid)) {
        mystats_user_stats(uid, cid);
        words = mystats_countchar(text, ' ');
        questions = mystats_countchar(text, '?');

        if (COUNT_WORDS) {
            strcpy(str, text);
            mystats_breakout(str, uid, cid);
        }

        sql_query("UPDATE %s_stats SET line = line+1, words = words+%d, "
                  "questions = questions+%d WHERE uid = '%d' AND cid = '%d'",
                  SQL_PREFIX, words, questions, uid, cid);
    }

    sql_query("UPDATE %s_online SET idle = NULL WHERE nick = '%s' AND cid = '%d'",
              SQL_PREFIX, nick, cid);

    return 0;
}

static cmd_t mystats_pubm_tbl[] = {
    {"*",	"",		mystats_pubm,	NULL},
    {NULL,	NULL,	NULL,				NULL}
};

static int mystats_mode(char *nick, char *host, char *handle, char *channel,
                        char *mode, char *victim) {
    int cid, pmid, mid = 0;
    memberlist *m;
    Context;

    /* Are we monitoring this channel? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) < 2)
        return 0;

    pmid = mystats_user_mode(victim, cid);
    m = mystats_user_getm(findchan_by_dname(channel), victim);
    if (mode[0] == '+') {
        switch (mode[1]) {
            case 'o': mid += 2;
            case 'h': mid += 1;
            case 'v': mid += 1;
        }

        if (pmid >= mid)
            return 0;
    } else {
        switch (mode[1]) {
            case 'o': mid = (chan_hashalfop(m) || chan_fakehalfop(m))?2:
                      (chan_hasvoice(m)?1:0); break;
            case 'h': if (pmid > 2) return 0;
                      mid = chan_hasvoice(m)?1:0; break;
            case 'v': if (pmid > 1) return 0;
                      mid = 0;
        }
    }

    /* Update user mode */
    sql_query("UPDATE %s_online SET mode = %d WHERE nick = '%s' AND cid = '%d'",
              SQL_PREFIX, mid, victim, cid);

    return 0;
}

static cmd_t mystats_mode_tbl[] = {
    {"*",		"",		mystats_mode,	NULL},
    {NULL,	NULL,	NULL,					NULL}
};

static int mystats_kick(char *nick, char *host, char *handle, char *channel,
                        char *target, char *reason) {
    int uid, cid, tid;
    Context;

    /* Are we monitoring this channel? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) < 2)
        return 0;

    uid = mystats_user_bychan(nick, cid);
    tid = mystats_user_bychan(target, cid);

    /* Update kicks and kicked statistics */
    if (uid && !mystats_user_isbot(uid)) {
        mystats_user_stats(uid, cid);
        sql_query("UPDATE %s_stats SET kicks = kicks+1 WHERE uid = '%d' AND cid = '%d'",
                  SQL_PREFIX, uid, cid);
    }

    if (tid && !mystats_user_isbot(uid)) {
        mystats_user_stats(tid, cid);
        sql_query("UPDATE %s_stats SET kicked = kicked+1 WHERE uid = '%d' AND cid = '%d'",
                  SQL_PREFIX, tid, cid);
    }

    /* Part kicked user */
    mystats_part(target, "", "*", channel);

    return 0;
}

static cmd_t mystats_kick_tbl[] = {
    {"*",	"",		mystats_kick,	NULL},
    {NULL,	NULL,	NULL,				NULL}
};

static int mystats_nick(char *nick, char *host, char *handle, char *channel,
                        char *newnick) {
    int cid;
    Context;

    /* Are we monitoring this channel? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) < 2)
        return 0;

    /* Update nick in online database */
    sql_query("UPDATE %s_online SET nick = '%s' WHERE nick = '%s' AND cid = '%d'",
              SQL_PREFIX, newnick, nick, cid);

    return 0;
}

static cmd_t mystats_nick_tbl[] = {
    {"*",	"",		mystats_nick,	NULL},
    {NULL,	NULL,	NULL,				NULL}
};

static int mystats_topc(char *nick, char *host, char *handle, char *channel, char *topic) {
    int cid, uid;
    Context;

    /* Are we monitoring this channel? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) < 2)
        return 0;

    /* Update topic in chans table */
    sql_query("UPDATE %s_chans SET topic = '%s', setby = '%s', seton = '%d' "
              "WHERE cid = '%d'", SQL_PREFIX, topic, nick, (int)time(NULL), cid);

    /* Update topic stats for user */
    uid = mystats_user_bychan(nick, cid);
    if (uid && !mystats_user_isbot(uid)) {
        mystats_user_stats(uid, cid);
        sql_query("UPDATE %s_stats SET topic = topic+1 WHERE uid = '%d' AND cid = '%d'",
                  SQL_PREFIX, uid, cid);
    }

    return 0;
}

static cmd_t mystats_topc_tbl[] = {
    {"*",	"",		mystats_topc,	NULL},
    {NULL,	NULL,	NULL,				NULL}
};

static int mystats_ctcp(char *nick, char *host, char *handle, char *channel,
                        char *key, char *rest) {
    int cid, uid;
    Context;

    /* Are we monitoring this channel? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) < 2)
        return 0;

    uid = mystats_user_bychan(nick, cid);
    mystats_activity(cid, uid);
    if (uid && !mystats_user_isbot(uid)) {
        mystats_user_stats(uid, cid);
        sql_query("UPDATE %s_stats SET actions = actions+1 WHERE uid = '%d' "
                  "AND cid = '%d'", SQL_PREFIX, uid, cid);
    }

    return 0;
}

static cmd_t mystats_ctcp_tbl[] = {
    {"ACTION",	"",		mystats_ctcp,	NULL},
    {NULL,			NULL,	NULL,						NULL}
};

