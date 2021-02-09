/*
 * settings.c -- part of mystats.mod
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

MYSQL       mysql;
static char SQL_HOST[128];
static char SQL_USER[16];
static char SQL_PASS[16];
static char SQL_DBASE[32];
static char SQL_PREFIX[32];
static char SQL_CASHE[256];
static char CAT_SHOW[256];
static int  COUNT_WORDS = 0;
static int  COUNT_MINWORDS = 32;

static int mystats_cat_show[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0
};

static char *mystats_cat_cshow[] = {
    "lines", "words", "joins", "kicks", "kicked", "questions", "topics",
    "actions", "online", "wpm"
};

static tcl_ints mystats_tcl_ints[] = {
    {"swords",      &COUNT_WORDS,       0},
    {"sminwords",   &COUNT_MINWORDS,    0},
    {NULL,          NULL,               0}
};

static tcl_strings mystats_tcl_strings[] = {
    {"shost",       SQL_HOST,           sizeof(SQL_HOST),       0},
    {"suser",       SQL_USER,           sizeof(SQL_USER),       0},
    {"spass",       SQL_PASS,           sizeof(SQL_PASS),       0},
    {"sdbase",      SQL_DBASE,          sizeof(SQL_DBASE),      0},
    {"sprefix",     SQL_PREFIX,         sizeof(SQL_PREFIX),     0},
    {"scats",       CAT_SHOW,           sizeof(CAT_SHOW),       0},
    {NULL,          NULL,               0,                      0}
};

