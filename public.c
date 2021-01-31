/*
 * public.c -- part of mystats.mod
 * Copyright (C) 2003  Douglas Cau <douglas@cau.se>
 *
 *  $Id: public.c 57 2004-03-23 20:10:09Z cau $
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

static char *mystats_category[] = {
    "line", "words", "joins", "kicks", "kicked", "questions", "topic",
    "actions", "online", "words/%s_stats.online"
};

static char *mystats_get_category(int cat) {
    char *str;
    Context;

    str = nmalloc(33);
    sprintf(str, mystats_category[cat], SQL_PREFIX);
    return str;
}

static int mystats_top10(char *nick, char *host, char *handle, char *channel, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    int cid, cat = -1, i = 0;
    char output[512], *str;
    Context;

    /* Are we monitoring this channel? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) == -1)
        return 0;

    /* Get category to order by */
    if (!strlen(par)) {
        cat = 1;
    } else {
        for (i = 0; i < 10; i++) {
            if (!strcmp(par, get_language(0xd043 + i))) {
                cat = i;
                break;
            }
        }
    }

    if (cat == -1) {
        dprintf(DP_SERVER, "NOTICE %s :%s\n", nick, MYSTATS_PUB_NOCAT);
        return 0;
    }

    /* Get the top10 users */
    str = mystats_get_category(cat);
    sql_query("SELECT %s_users.handle, %s_stats.%s AS tot FROM %s_users "
              "INNER JOIN %s_stats ON %s_users.uid = %s_stats.uid "
              "WHERE %s_stats.cid = '%d' ORDER BY tot DESC LIMIT 10",
              SQL_PREFIX, SQL_PREFIX, str, SQL_PREFIX, SQL_PREFIX,
              SQL_PREFIX, SQL_PREFIX, SQL_PREFIX, cid);
    nfree(str);

    result = mysql_store_result(&mysql);
    if (!result) {
        dprintf(DP_SERVER, "NOTICE %s :%s\n", nick, MYSTATS_SQL_ERRRES);
        return 0;
    }

    if(!result->row_count) {
        mysql_free_result(result);
        dprintf(DP_SERVER, "NOTICE %s :%s\n", nick, MYSTATS_PUB_NOUSERS);
        return 0;
    }

    /* Prepeare output */
    i = 1;
    sprintf(output, "Top10 (%s): ", get_language(0xd043 + cat));
    while ((row = mysql_fetch_row(result))) {
        str = (cat == 8)?mystats_tmcalc(atoi(row[1])):row[1];
        sprintf(output, "%s%d.%s(%s) ", output, i++, row[0], str);
        if (cat == 8)
            nfree(str);
    }

    dprintf(DP_SERVER, "PRIVMSG %s :%s\n", channel, output);

    mysql_free_result(result);
    putlog(LOG_CMDS, channel, "<<%s>> !%s! top10", nick, handle);
    return 0;
}

static int mystats_stats(char *nick, char *host, char *handle, char *channel, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    int cid, uid, i;
    char output[512], *str;
    Context;

    /* Are we monitoring this channel? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) == -1)
        return 0;

    /* Get user */
    if (strlen(par) && par[0] == '-')
        uid = mystats_user_byhand(++par);
    else
        uid = mystats_user_bychan(strlen(par)?par:nick, cid);

    if (!uid) {
        dprintf(DP_SERVER, "NOTICE %s :%s\n", nick, MYSTATS_PUB_STATSUSAGE);
        return 0;
    }

    sql_query("SELECT line, words, joins, kicks, kicked, questions, topic, "
              "actions, online, words/online FROM %s_stats WHERE uid = '%d' "
              "AND cid = '%d'", SQL_PREFIX, uid, cid);

    result = mysql_store_result(&mysql);
    if (!result)
        return 0;

    if(!result->row_count) {
        mysql_free_result(result);
        return 0;
    }

    row = mysql_fetch_row(result);
    strcpy(output, "");
    for (i = 0; i < 10; i++) {
        if (mystats_cat_show[i]) {
            str = (i == 8)?mystats_tmcalc(atoi(row[i])):row[i];
            sprintf(output, "%s %s: %s,", output, get_language(0xd043 + i), str);
            if (i == 8)
                nfree(str);
        }
    }

    if (strlen(output))
        output[strlen(output) - 1] = 0;

    dprintf(DP_SERVER, "PRIVMSG %s :Stats (%s):%s\n",
            channel, strlen(par)?par:nick, output);

    mysql_free_result(result);
    putlog(LOG_CMDS, channel, "<<%s>> !%s! stats", nick, handle);
    return 0;
}

static int mystats_place(char *nick, char *host, char *handle, char *channel, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    int cid, uid, cat = -1, i, place;
    char *user, *order, *str;
    Context;

    /* Are we monitoring this channel? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) == -1)
        return 0;

    user = newsplit(&par);
    order = par;

    /* Get user */
    if (strlen(user) && user[0] == '-')
        uid = mystats_user_byhand(++user);
    else
        uid = mystats_user_bychan(strlen(user)?user:nick, cid);

    if (!uid) {
        dprintf(DP_SERVER, "NOTICE %s :%s\n", nick, MYSTATS_PUB_PLACEUSAGE);
        return 0;
    }

    /* Get category to order by */
    if (!strlen(par)) {
        cat = 1;
    } else {
        for (i = 0; i < 10; i++) {
            if (!strcmp(par, get_language(0xd043 + i))) {
                cat = i;
                break;
            }
        }
    }

	if (cat == -1) {
		dprintf(DP_SERVER, "NOTICE %s :%s\n", nick, MYSTATS_PUB_NOCAT);
		return 0;
	}

    /* Get the list to go through */
    str = mystats_get_category(cat);
    sql_query("SELECT %s_users.uid, %s_users.handle, %s_stats.%s AS tot FROM %s_users "
              "INNER JOIN %s_stats ON %s_users.uid = %s_stats.uid "
              "WHERE %s_stats.cid = '%d' ORDER BY tot DESC",
              SQL_PREFIX, SQL_PREFIX, SQL_PREFIX, str, SQL_PREFIX, SQL_PREFIX,
              SQL_PREFIX, SQL_PREFIX, SQL_PREFIX, cid);
    nfree(str);

    result = mysql_store_result(&mysql);
    if (!result)
        return 0;

    place = 0;
    for (i = 1; (row = mysql_fetch_row(result)); i++) {
        if (atoi(row[0]) == uid) {
            place = i;
            break;
        }
    }

    if (!place) {
        dprintf(DP_SERVER, "NOTICE %s :%s\n", nick, MYSTATS_PUB_NORECORD);
        mysql_free_result(result);
        return 0;
    }

    str = (cat == 8)?mystats_tmcalc(atoi(row[2])):row[2];
    dprintf(DP_SERVER, "PRIVMSG %s :Place (%s): %d.%s(%s)\n", channel,
            get_language(0xd043 + cat), place, row[1], str);
    if (cat == 8)
        nfree(str);


    mysql_free_result(result);
    putlog(LOG_CMDS, channel, "<<%s>> !%s! place", nick, handle);
    return 0;
}

static int mystats_ranking(char *nick, char *host, char *handle, char *channel, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    char output[512], *str;
    int cid, uid, i, l, place;
    Context;

    /* Are we monitoring this channel? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) == -1)
        return 0;

    /* Get user */
    if (strlen(par) && par[0] == '-')
        uid = mystats_user_byhand(++par);
    else
        uid = mystats_user_bychan(strlen(par)?par:nick, cid);

    if (!uid) {
        dprintf(DP_SERVER, "NOTICE %s :%s\n", nick, MYSTATS_PUB_RANKUSAGE);
        return 0;
    }

    /* Get place for each category */
    sprintf(output, "PRIVMSG %s :Ranking (%s):", channel, strlen(par)?par:nick);
    for (i = 0; i < 10; i++) {
        if (!mystats_cat_show[i])
            continue;

        str = mystats_get_category(i);
        sql_query("SELECT %s_users.uid, %s_stats.%s AS tot FROM %s_users "
                  "INNER JOIN %s_stats ON %s_users.uid = %s_stats.uid "
                  "WHERE %s_stats.cid = '%d' ORDER BY tot DESC",
                  SQL_PREFIX, SQL_PREFIX, str, SQL_PREFIX, SQL_PREFIX,
                  SQL_PREFIX, SQL_PREFIX, SQL_PREFIX, cid);
        nfree(str);

        result = mysql_store_result(&mysql);
        if (!result)
            return 0;

        place = 0;
        for (l = 1; (row = mysql_fetch_row(result)); l++) {
            if (atoi(row[0]) == uid) {
                place = l;
                break;
            }
        }

        if (!place) {
            dprintf(DP_SERVER, "NOTICE %s :%s\n", nick, MYSTATS_PUB_NORECORD);
            mysql_free_result(result);
            return 0;
        }

        str = (i == 8)?mystats_tmcalc(atoi(row[1])):row[1];
        sprintf(output, "%s #%d@%s(%s)", output, place,
                get_language(0xd043 + i), str);
        if (i == 8)
            nfree(str);

        mysql_free_result(result);
    }

    dprintf(DP_SERVER, "%s\n", output);
    putlog(LOG_CMDS, channel, "<<%s>> !%s! ranking", nick, handle);
    return 0;
}

static int mystats_words(char *nick, char *host, char *handle, char *channel, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    int cid, uid, total, i = 1;
    char output[512];
    Context;

    /* Are we monitoring this channel? Are we counting words? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) == -1 || COUNT_WORDS != 1)
        return 0;

    /* Get user */
    if (strlen(par) && par[0] == '-')
        uid = mystats_user_byhand(++par);
    else
        uid = mystats_user_bychan(strlen(par)?par:nick, cid);

    if (!uid) {
        dprintf(DP_SERVER, "NOTICE %s :%s\n", nick, MYSTATS_PUB_WORDSUSAGE);
        return 0;
    }

    /* Get total wordcount */
    sql_query("SELECT SUM(count) FROM %s_words WHERE uid = '%d' AND cid = '%d'",
              SQL_PREFIX, uid, cid);

    result = mysql_store_result(&mysql);
    if (!result)
        return 0;
	
    if (!(row = mysql_fetch_row(result))) {
        mysql_free_result(result);
        return 0;
    }

    total = row[0]?atoi(row[0]):0;
    if (total < COUNT_MINWORDS) {
        mysql_free_result(result);
        dprintf(DP_SERVER, "PRIVMSG %s :Words (%s): %s\n", channel, strlen(par)?par:nick,
                MYSTATS_PUB_MINWORDS);
        putlog(LOG_CMDS, channel, "<<%s>> !%s! words", nick, handle);
        return 0;
    }

    mysql_free_result(result);
    sql_query("SELECT word, count FROM %s_words WHERE uid = '%d' AND cid = '%d'"
              "ORDER BY count DESC LIMIT 10", SQL_PREFIX, uid, cid);

    result = mysql_store_result(&mysql);
    if (!result)
        return 0;
	
    sprintf(output, "Words (%s):", strlen(par)?par:nick);
    while ((row = mysql_fetch_row(result)))
        sprintf(output, "%s %d.%s(%s)", output, i++, row[0], row[1]);

    dprintf(DP_SERVER, "PRIVMSG %s :%s\n", channel, output);
    putlog(LOG_CMDS, channel, "<<%s>> !%s! words", nick, handle);
    mysql_free_result(result);
    return 0;
}

static int mystats_uttered(char *nick, char *host, char *handle, char *channel, char *par) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    int cid, i = 1;
    char output[512];
    Context;

    /* Are we monitoring this channel? Are we counting words? */
    cid = mystats_chan_byname(channel);
    if (mystats_chan_status(cid) == -1 || COUNT_WORDS != 1)
        return 0;

    if (strlen(par) < 2) {
        dprintf(DP_SERVER, "NOTICE %s :%s\n", nick, MYSTATS_PUB_UTTEREDUSAGE);
        return 0;
    }

    sql_query("SELECT %s_users.handle AS handle, SUM(%s_words.count) AS total "
              "FROM %s_words INNER JOIN %s_users "
              "ON %s_words.uid = %s_users.uid WHERE cid = '%d' "
              "AND %s_words.word LIKE '%s' GROUP BY handle "
              "ORDER BY total DESC LIMIT 10",
              SQL_PREFIX, SQL_PREFIX, SQL_PREFIX, SQL_PREFIX, SQL_PREFIX,
              SQL_PREFIX, cid, SQL_PREFIX, par);

    result = mysql_store_result(&mysql);
    if (!result)
        return 0;

    if (!result->row_count)
        sprintf(output, MYSTATS_PUB_NOUTTERED, par);
    else
        sprintf(output, "Uttered (%s):", par);

    while ((row = mysql_fetch_row(result))) {
        sprintf(output, "%s %d.%s(%s)", output, i++, row[0], row[1]);
    }

    dprintf(DP_SERVER, "PRIVMSG %s :%s\n", channel, output);
    putlog(LOG_CMDS, channel, "<<%s>> !%s! uttered", nick, handle);
    mysql_free_result(result);
    return 0;
}

static cmd_t mystats_cmd_tbl[] = {
    {"!top10",			"",		mystats_top10,		0},
    {"!stats",			"",		mystats_stats,		0},
    {"!place",			"",		mystats_place,		0},
    {"!ranking",		"",		mystats_ranking,	0},
    {"!words",			"",		mystats_words,		0},
    {"!uttered",        "",     mystats_uttered,    0},
    {NULL,              NULL,   0,                  0}
};

