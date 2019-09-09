/*
 * armour.c
 * Copyright (C) 2009-2011, 2012 Joachim de Groot <jdegroot@web.de>
 *
 * NLarn is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NLarn is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "armour.h"
#include "items.h"

const armour_data armours[AT_MAX] =
{
    /* id            name                      ac  category   material    effect      we     pr disguise  ob un*/
    //{ AT_CLOTHHOOD,  "hood",                    0, AC_HELMET, IM_CLOTH,   ET_NONE,   200,     5, AT_MAX, 1,  0 },
    { AT_ROBE,       "plain robe",              0, AC_SUIT,   IM_CLOTH,   ET_NONE,   600,    15, AT_MAX, 1,  0 },
    { AT_TUNIC,      "leather tunic",           1, AC_SUIT,   IM_LEATHER, ET_NONE,  3500,    30, AT_MAX, 1,  0 },
    { AT_CLOAK,      "light cloak",             1, AC_CLOAK,  IM_CLOTH,   ET_NONE,   400,    15, AT_MAX, 1,  0 },
    { AT_LGLOVES,    "pair of leather gloves",  1, AC_GLOVES, IM_LEATHER, ET_NONE,   800,    25, AT_MAX, 1,  0 },
    { AT_LBOOTS,     "pair of low boots",       1, AC_BOOTS,  IM_LEATHER, ET_NONE,  1800,    25, AT_MAX, 1,  0 },
    { AT_LHELMET,    "leather skullcap",        1, AC_HELMET, IM_LEATHER, ET_NONE,  2200,    25, AT_MAX, 1,  0 },
    { AT_LEATHER,    "leather armour",          2, AC_SUIT,   IM_LEATHER, ET_NONE,  4000,    40, AT_MAX, 1,  0 },
    { AT_BSHIELD,    "buckler",                 1, AC_SHIELD, IM_IRON,    ET_NONE,  3500,    25, AT_MAX, 1,  0 },
    { AT_SLEATHER,   "studded leather armour",  3, AC_SUIT,   IM_LEATHER, ET_NONE,  7500,    80, AT_MAX, 1,  0 },
    { AT_RINGMAIL,   "orcish ring armour",      4, AC_SUIT,   IM_IRON,    ET_NONE, 10000,   320, AT_MAX, 1,  0 },
    { AT_RSHIELD,    "round shield",            2, AC_SHIELD, IM_WOOD,    ET_NONE,  4000,   125, AT_MAX, 1,  0 },
    { AT_MGLOVES,    "pair of mail gauntlets",  2, AC_GLOVES, IM_IRON,    ET_NONE,  2800,   125, AT_MAX, 1,  0 },
    { AT_HBOOTS,     "pair of high boots",      2, AC_BOOTS,  IM_LEATHER, ET_NONE,  2500,    95, AT_MAX, 1,  0 },
    { AT_CHAINHOOD,  "mail coif",               2, AC_HELMET, IM_IRON,    ET_NONE,  3400,   180, AT_MAX, 1,  0 },
    { AT_CHAINMAIL,  "chain mail",              5, AC_SUIT,   IM_IRON,    ET_NONE, 11500,   600, AT_MAX, 1,  0 },
    { AT_KSHIELD,    "kite shield",             3, AC_SHIELD, IM_WOOD,    ET_NONE,  7800,   350, AT_MAX, 1,  0 },
    { AT_SCALEMAIL,  "scale mail",              6, AC_SUIT,   IM_IRON,    ET_NONE, 13000,  1000, AT_MAX, 1,  0 },
    { AT_HPLATEMAIL, "half plate mail",         8, AC_SUIT,   IM_IRON,    ET_NONE, 17500,  2200, AT_MAX, 1,  0 },
    { AT_IGLOVES,    "pair of iron gauntlets",  3, AC_GLOVES, IM_IRON,    ET_NONE,  3400,   300, AT_MAX, 1,  0 },
    { AT_IBOOTS,     "pair of iron greaves",    3, AC_BOOTS,  IM_IRON,    ET_NONE,  3400,   450, AT_MAX, 1,  0 },
    { AT_FHELMET,    "full helmet",             3, AC_HELMET, IM_IRON,    ET_NONE,  3800,   450, AT_MAX, 1,  0 },
    { AT_FPLATEMAIL, "full plate mail",         9, AC_SUIT,   IM_IRON,    ET_NONE, 19000,  3200, AT_MAX, 1,  0 },
    { AT_PROTCLOAK,  "cloak of protection",     3, AC_CLOAK,  IM_CLOTH,   ET_NONE,   400,  1250, AT_CLOAK,1, 0 },
    { AT_GCROWN,     "golden crown",            0, AC_HELMET, IM_GOLD,    ET_NONE,  1300,  2000, AT_MAX, 1,  0 },
    { AT_TSHIELD,    "tower shield",            5, AC_SHIELD, IM_IRON,    ET_NONE, 10000,   750, AT_MAX, 1,  0 },
    { AT_SPEEDBOOTS, "pair of speed boots",     1, AC_BOOTS,  IM_LEATHER, ET_SPEED,  800,  2800, AT_LBOOTS,0,1 },
    { AT_SSHIELD,    "stainless steel shield",  4, AC_SHIELD, IM_STEEL,   ET_NONE,  8000,  1500, AT_MAX, 1,  0 },
    { AT_SPLATEMAIL, "stainless steel plate mail",10,AC_SUIT, IM_STEEL,   ET_NONE, 18500,  6800, AT_MAX, 1,  0 },
    { AT_CPLATEMAIL, "crystal shard mail",     13, AC_SUIT,   IM_GLASS,   ET_NONE, 22500,  8000, AT_MAX, 0,  0 },
    { AT_INVISCLOAK, "cloak of invisibility",   1, AC_CLOAK,  IM_CLOTH,   ET_INVISIBILITY,
                                                                                     400,  2800, AT_CLOAK,0, 1 },
    { AT_ELVENCHAIN, "elven mithril coat",     10, AC_SUIT,   IM_MITHRIL, ET_NONE,  8500, 16400, AT_CHAINMAIL,0,1 },
    { AT_DRAGONMAIL, "dragon scale mail",       7, AC_SUIT,   IM_DRAGON_HIDE,
                                                                   ET_RESIST_FIRE, 15000, 13000, AT_SCALEMAIL,0,1 },
    { AT_HEROCROWN,  "crown of heroism",        0, AC_HELMET, IM_GOLD,    ET_HEROISM,1300,11000, AT_GCROWN,0,1 },
};
