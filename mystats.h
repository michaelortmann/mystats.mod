/*
 * mystats.c -- part of mystats.mod
 * Copyright (C) 2003  Douglas Cau <douglas@cau.se>
 *
 *  $Id: mystats.h 71 2004-03-25 16:39:35Z cau $
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

#ifdef MAKING_MYSTATS

/* Functions provided by chans.c */
static int mystats_chan_byname(char *);
static void mystats_chan_join(char *);
static int mystats_chan_status(int);
static void mystats_chan_part(char *);

/* Functions provided by common.c */
static int is_alpha(int);
static char *sql_escape(char *);
static int sql_query(char *, ...);
static int mystats_countchar(char *, char);
static char *mystats_tmcalc(int);
static int mystats_dupe(int, int, int *);
static void mystats_activity(int, int);
static void mystats_countword(char *, int, int);
static void mystats_breakout(char *, int, int);

/* Functions provided by mystats.c */
static int mystats_hook_rehash();
static int mystats_hook_daily();
static int mystats_hook_minutely();
static int mystats_setup(char *);
static int mystats_expmem();
static void mystats_report(int, int);
static char *mystats_close();
char *mystats_start(Function *);

/* Functions provided by public.c */
static char *mystats_get_category(int);
static int mystats_top10(char *, char *, char *, char *, char *);
static int mystats_stats(char *, char *, char *, char *, char *);
static int mystats_place(char *, char *, char *, char *, char *);
static int mystats_ranking(char *, char *, char *, char *, char *);
static int mystats_words(char *, char *, char *, char *, char *);
static int mystats_uttered(char *, char *, char *, char *, char *);

/* Functions provided by sensors.c */
static int mystats_join(char *, char *, char *, char *);
static int mystats_part(char *, char *, char *, char *);
static int mystats_pubm(char *, char *, char *, char *, char *);
static int mystats_mode(char *, char *, char *, char *, char *, char *);
static int mystats_kick(char *, char *, char *, char *, char *, char *);
static int mystats_nick(char *, char *, char *, char *, char *);
static int mystats_topc(char *, char *, char *, char *, char *);
static int mystats_ctcp(char *, char *, char *, char *, char *, char *);

/* Functions provided by settings.c */

/* Functions provided by triggers.c */
static int mystats_dcc_adduser(struct userrec *, int, char *);
static int mystats_dcc_deluser(struct userrec *, int, char *);
static int mystats_dcc_addhost(struct userrec *, int, char *);
static int mystats_dcc_delhost(struct userrec *, int, char *);
static int mystats_dcc_match(struct userrec *, int, char *);
static int mystats_dcc_addchan(struct userrec *, int, char *);
static int mystats_dcc_delchan(struct userrec *, int, char *);
static int mystats_dcc_purge(struct userrec *, int, char *);
static int mystats_dcc_addbot(struct userrec *, int, char *);
static int mystats_dcc_delbot(struct userrec *, int, char *);
static int mystats_dcc_channel(struct userrec *, int, char *);

/* Functions provided by users.c */
static int mystats_user_bymask(char *, char *);
static int mystats_user_bychan(char *, int);
static int mystats_user_isbot(int);
static void mystats_user_stats(int, int);
static void mystats_user_synchost(char *, int, int);
static int mystats_user_mode(char *, int);
static memberlist *mystats_user_getm(struct chanset_t *, char *);
static char *mystats_user_cname(int);
static int mystats_user_byhand(char *);

#endif /* MAKING MYSTATS */

