/*
 * armour.h
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

#ifndef __ARMOUR_H_
#define __ARMOUR_H_

#include "effects.h"
#include "items.h"
#include "utils.h"

typedef enum _armour_class
{
    AC_BOOTS,
    AC_CLOAK,
    AC_GLOVES,
    AC_HELMET,
    AC_SHIELD,
    AC_SUIT,
    AC_MAX
} armour_class;

typedef enum _armour_t
{
    //AT_CLOTHHOOD,
    AT_ROBE,
    AT_TUNIC,
    AT_CLOAK,
    AT_LGLOVES,
    AT_LBOOTS,
    AT_LHELMET,
    AT_LEATHER,
    AT_BSHIELD,
    AT_SLEATHER,
    AT_RINGMAIL,
    AT_RSHIELD,
    AT_MGLOVES,
    AT_HBOOTS,
    AT_CHAINHOOD,
    AT_CHAINMAIL,
    AT_KSHIELD,
    AT_SCALEMAIL,
    AT_HPLATEMAIL,
    AT_IGLOVES,
    AT_IBOOTS,
    AT_FHELMET,
    AT_FPLATEMAIL,
    AT_PROTCLOAK,
    AT_GCROWN,
    AT_TSHIELD,
    AT_SPEEDBOOTS,
    AT_SSHIELD,
    AT_SPLATEMAIL,
    AT_CPLATEMAIL,
    AT_INVISCLOAK,
    AT_ELVENCHAIN,
    AT_DRAGONMAIL,
    AT_HEROCROWN,
    AT_MAX
} armour_t;

typedef struct _armour_data
{
    armour_t id;
    const char *name;
    int ac;
    armour_class category;
    item_material_t material;
    effect_t et;        /* for uniques: effect on the item */
    int weight;         /* used to determine inventory weight */
    int price;          /* base price in the shops */
    armour_t disguise;  /* item used for description until armour type is identified */
    unsigned
        obtainable: 1,  /* available in the shop? */
        unique: 1;      /* generated only once */
} armour_data;

/* external vars */

extern const armour_data armours[AT_MAX];

/* inline functions */
static inline effect_t armour_effect(item *armour)
{
    g_assert(armour->id < AT_MAX);
    return armours[armour->id].et;
}

static inline armour_t armour_disguise(item *armour)
{
    g_assert(armour->id < AT_MAX);
    return armours[armour->id].disguise;
}

static inline gboolean armour_unique(item *armour)
{
    g_assert(armour->id < AT_MAX);
    return armours[armour->id].unique;
}

static inline guint armour_base_ac(item *armour)
{
    g_assert(armour->id < AT_MAX);
    return armours[armour->id].ac;
}

static inline guint armour_ac(item *armour)
{
    int ac = armour_base_ac(armour)
             + item_condition_bonus(armour);

    return (guint)max(ac, 0);
}

/* macros */

#define armour_name(armour)     (armours[(armour)->id].name)
#define armour_class(armour)    (armours[(armour)->id].category)
#define armour_material(armour) (armours[(armour)->id].material)
#define armour_weight(armour)   (armours[(armour)->id].weight)
#define armour_price(armour)    (armours[(armour)->id].price)

#endif
