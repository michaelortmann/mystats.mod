/*
 * common.c -- part of mystats.mod
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

static int is_alpha(int c) {
    char *str = MYSTATS_CHR_LIST;
    int i, len = strlen(str);

    for (i = 0; i < len; i++)
        if (c == *(str + i))
            return 1;

    return 0;
}

static char *sql_escape(char *str) {
    static char *rstr;

    rstr = nmalloc(strlen(str)*2+1);
    if(rstr)
        mysql_real_escape_string(&mysql, rstr, str, strlen(str));

    return rstr;
}

static int sql_query(char *str, ...) {
    va_list	va;
    char	*query,*squery;
    char	*estr;
    int		query_size,query_pos=0;
    int		l,myres;

    va_start(va, str);
    query_size = strlen(str)*2;
    squery = query = nmalloc(query_size);

    while(*str) {
        if(*str == '%') {
            switch(*++str) {
            case 's':
                estr = sql_escape(va_arg(va, char *));
                l = strlen(estr);

                squery = query = nrealloc(squery, query_size+l+32);
                query_size += l+1;
                query += query_pos;

                memcpy(query, estr, l);
                nfree(estr);
                query += l;
                query_pos += l;
                break;
            case 'i':
            case 'd':
                squery = query = nrealloc(squery, query_size+32);
                query_size += 32;
                query += query_pos;

                l = snprintf(query, 32, "%d", va_arg(va, int));
                if(l == -1) l = 32;
                query += l;
                query_pos += l;

                break;
            default:
                *query++ = *str;
                query_pos++;
            }

            str++;
        } else {
            *query++ = *str++;
            query_pos++;
        }
    }

    va_end(va);
    *query = 0;

    myres = mysql_query(&mysql, squery);

    nfree(squery);
    return myres;
}

static int mystats_countchar(char *text, char c) {
    int words = 0;

    if (c == ' ')
        words++;

    while (*text) {
        if (*text == c && *(text + 1) != c) words++;
        text++;
    }

    return words;
}

static char *mystats_tmcalc(int min) {
    char *str;
    int week, day, hour;

    str = nmalloc(33);
    if (min >= 10080) {
        week = min/10080;
        min -= week * 10080;
        day = min/1440;
        sprintf(str, "%dw, %dd", week, day);
    } else if (min >= 1440) {
        day = min/1440;
        min -= day * 1440;
        hour = min/60;
        sprintf(str, "%dd, %dh", day, hour);
    } else {
        hour = min/60;
        min -= hour * 60;
        sprintf(str, "%dh, %dm", hour, min);
    }

    return str;
}

static int mystats_dupe(int uid, int cid, int *users) {
    int i = 0;

    while(users[i++] != -1) {
        if (users[i] == uid && users[(i)*2 + 1] == cid)
            return 1;
    }

    users[i] = uid;
    users[i+1] = -1;
    users[(i)*2 + 1] = cid;

    return 0;
}

static void mystats_activity(int cid, int uid) {
    time_t rawtime;
    struct tm *local;
    int hour;

    rawtime = time(NULL);
    local = localtime(&rawtime);
    hour = local->tm_hour / 4;

    sql_query("UPDATE %s_chans SET hour_%d = hour_%d +1 WHERE cid = '%d'",
              SQL_PREFIX, hour, hour, cid);

    if (uid && !mystats_user_isbot(uid)) {
        sql_query("UPDATE %s_stats SET hour_%d = hour_%d +1 WHERE cid = '%d' "
                  "AND uid = '%d'", SQL_PREFIX, hour, hour, cid, uid);
    }

    return;
}

static void mystats_countword(char *str, int uid, int cid) {

    sql_query("UPDATE %s_words SET count = count+1 WHERE uid = '%d' AND word = '%s' "
              "AND cid = '%d'", SQL_PREFIX, uid, str, cid);
	
    if (!mysql_affected_rows(&mysql))
        sql_query("INSERT INTO %s_words (uid, cid, word) VALUES ('%d', '%d', '%s')",
                  SQL_PREFIX, uid, cid, str);
	
    return;
}

static void mystats_breakout(char *str, int uid, int cid) {
    char *next;
    int go = 1;

    if (!str || strlen(str) < 2)
        return;

    if (!is_alpha(*str))
        return mystats_breakout(str + 1, uid, cid);

    if (!is_alpha(*(str + 1)))
        return mystats_breakout(str + 2, uid, cid);

    for (next = str + 2; is_alpha(*next); next++)
        ;

    if (!*next)
        go = 0;

    *next = 0;
    if (strlen(str) > 1)
        mystats_countword(str, uid, cid);

    if (go)
        return mystats_breakout(next + 1, uid, cid);
}

