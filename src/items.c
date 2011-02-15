/*
 * items.h
 * Copyright (C) 2009, 2010, 2011 Joachim de Groot <jdegroot@web.de>
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

#include <assert.h>
#include <string.h>

#include "amulets.h"
#include "armour.h"
#include "container.h"
#include "display.h"
#include "game.h"
#include "gems.h"
#include "items.h"
#include "map.h"
#include "nlarn.h"
#include "player.h"
#include "potions.h"
#include "rings.h"
#include "scrolls.h"
#include "spells.h"
#include "utils.h"
#include "weapons.h"

static const char *item_desc_get(item *it, int known);

const item_type_data item_data[IT_MAX] =
{
    /* item_t       name_sg       name_pl        IMG   max_id           op bl co eq us st id */
    { IT_NONE,      "",           "",            ' ',  0,               0, 0, 0, 0, 0, 0, 0, },
    { IT_AMULET,    "amulet",     "amulets",     '"',  AM_LARN,         0, 1, 0, 1, 0, 0, 1, },
    { IT_AMMO,      "ammunition", "ammunition",  '\'', AMT_MAX,         1, 1, 1, 1, 0, 1, 1, },
    { IT_ARMOUR,    "armour",     "armour",      '[',  AT_MAX,          1, 1, 1, 1, 0, 0, 1, },
    { IT_BOOK,      "book",       "books",       '+',  SP_MAX_BOOK,     0, 1, 1, 0, 1, 1, 1, },
    { IT_CONTAINER, "container",  "containers",  ']',  CT_MAX,          0, 0, 1, 0, 0, 0, 0, },
    { IT_GEM,       "gem",        "gems",        '*',  GT_MAX,          1, 0, 0, 0, 0, 1, 0, },
    { IT_GOLD,      "coin",       "coins",       '$',  0,               0, 0, 0, 0, 0, 1, 0, },
    { IT_POTION,    "potion",     "potions",     '!',  PO_CURE_DIANTHR, 0, 1, 1, 0, 1, 1, 1, },
    { IT_RING,      "ring",       "rings",       '=',  RT_MAX,          1, 1, 1, 1, 0, 0, 1, },
    { IT_SCROLL,    "scroll",     "scrolls",     '?',  ST_MAX,          0, 1, 1, 0, 1, 1, 1, },
    { IT_WEAPON,    "weapon",     "weapons",     '(',  WT_MAX,          1, 1, 1, 1, 0, 0, 1, },
};

const item_material_data item_materials[IM_MAX] =
{
    /* type           name           adjective   colour */
    { IM_NONE,        "",            "",         DC_NONE      },
    { IM_PAPER,       "paper",       "papier",   DC_WHITE     },
    { IM_CLOTH,       "cloth",       "cloth",    DC_LIGHTGRAY },
    { IM_LEATHER,     "leather",     "leathern", DC_BROWN     },
    { IM_WOOD,        "wood",        "wooden",   DC_BROWN     },
    { IM_BONE,        "bone",        "osseous",  DC_DARKGRAY  },
    { IM_DRAGON_HIDE, "dragon hide", "scabby",   DC_GREEN     },
    { IM_LEAD,        "lead",        "leady",    DC_DARKGRAY  },
    { IM_IRON,        "iron",        "irony",    DC_LIGHTGRAY },
    { IM_STEEL,       "steel",       "steely",   DC_WHITE     },
    { IM_COPPER,      "copper",      "cupreous", DC_BROWN     },
    { IM_SILVER,      "silver",      "silvery",  DC_LIGHTGRAY },
    { IM_GOLD,        "gold",        "golden",   DC_YELLOW    },
    { IM_PLATINUM,    "platinum",    "platinum", DC_WHITE     },
    { IM_MITHRIL,     "mitril",      "mithrial", DC_LIGHTGRAY },
    { IM_GLASS,       "glass",       "vitreous", DC_LIGHTCYAN },
    { IM_STONE,       "stone",       "stony",    DC_WHITE     },
    { IM_GEMSTONE,    "gemstone",    "gemstone", DC_RED       },
};

/* functions */

item *item_new(item_t item_type, int item_id)
{
    item *nitem;
    effect *eff = NULL;
    int loops = 0;

    assert(item_type > IT_NONE && item_type < IT_MAX);

    /* has to be zeroed or memcmp will fail */
    nitem = g_malloc0(sizeof(item));

    nitem->type = item_type;
    nitem->id = item_id;
    nitem->count = 1;

    /* set bonus_known on unoptimizable items to avoid stacking problems */
    if (!item_is_optimizable(item_type))
    {
        nitem->bonus_known = TRUE;
    }

    /* register item with game */
    nitem->oid = game_item_register(nlarn, nitem);

    /* add special item type specific attributes */
    switch (item_type)
    {
    case IT_AMULET:
        /* if the amulet has already been created try to create another one */
        while (nlarn->amulet_created[nitem->id] && (loops < item_max_id(IT_AMULET)))
        {
            /* loop through all amulets to find an available one */
            nitem->id++;

            /* do not create the amulet of larn accidentally */
            if (nitem->id == item_max_id(item_type))
                nitem->id = 1;

            loops++;
        }

        /* every amulet has been created */
        if (loops == item_max_id(IT_AMULET))
        {
            /* remove the failed attempt */
            item_destroy(nitem);

            /* create something that is not a container */
            do
            {
                item_type = rand_1n(IT_MAX);
            }
            while (item_type == IT_CONTAINER);

            /* no need to do a finetouch here, will be done in the calling func. */
            return item_new_random(item_type, FALSE);
        }

        nlarn->amulet_created[nitem->id] = TRUE;

        if (amulet_effect_t(nitem))
        {
            eff = effect_new(amulet_effect_t(nitem));
            item_effect_add(nitem, eff);
        }

        break;

    case IT_GEM:
        /* ensure minimal size */
        if (nitem->bonus == 0)
            nitem->bonus = rand_1n(20);
        break;

    case IT_GOLD:
        nitem->count = item_id;
        break;

    case IT_POTION:
        /* prevent that the unique potion can be created twice */
        if ((item_id == PO_CURE_DIANTHR) && nlarn->cure_dianthr_created)
        {
            nitem->id = rand_1n(item_max_id(IT_POTION));
        }
        else if (item_id == PO_CURE_DIANTHR)
        {
            nlarn->cure_dianthr_created = TRUE;
        }
        break;

    case IT_RING:
        if (ring_effect_t(nitem))
        {
            eff = effect_new(ring_effect_t(nitem));

            /* ring of extra regeneration is better than the average */
            if (item_id == RT_EXTRA_REGEN)
            {
                eff->amount *= 5;
            }

            item_effect_add(nitem, eff);
        }
        break;

    case IT_WEAPON:
        while (weapon_is_unique(nitem) && nlarn->weapon_created[nitem->id])
        {
            /* create another random weapon instead */
            nitem->id = rand_1n(WT_MAX);
        }

        if (weapon_is_unique(nitem))
        {
            /* mark unique weapon as created */
            nlarn->weapon_created[nitem->id] = TRUE;
        }

        /* special effects for Bessman's Hammer */
        if (nitem->id == WT_BESSMAN)
        {
            eff = effect_new(ET_INC_STR);
            eff->amount = 10;
            item_effect_add(nitem, eff);

            eff = effect_new(ET_INC_DEX);
            eff->amount = 10;
            item_effect_add(nitem, eff);

            eff = effect_new(ET_INC_INT);
            eff->amount = -10;
            item_effect_add(nitem, eff);
        }
        break;

    default:
        /* nop */
        break;
    }

    return nitem;
}

item *item_new_random(item_t item_type, gboolean finetouch)
{
    item *it;

    int item_id = 0;
    int min_id = 1, max_id = 0;

    assert(item_type > IT_NONE && item_type < IT_MAX);

    max_id = item_max_id(item_type);

    /* special settings for some item types */
    switch (item_type)
    {
    case IT_CONTAINER:
        max_id = CT_CHEST; /* only bags and caskets */
        break;

    case IT_GOLD:
        /* ID is amount in case of gold */
        min_id = 50;
        max_id = 250;
        break;

    default:
        /* nop */
        break;
    }
    item_id = rand_m_n(min_id, max_id);
    it = item_new(item_type, item_id);

    if (item_type == IT_AMMO)
        it->count = rand_m_n(10, 50);

    if (finetouch)
        item_new_finetouch(it);

    return it;
}

item *item_new_by_level(item_t item_type, int num_level)
{
    item *nitem;
    int id_min, id_max;
    float variance, id_base, divisor;

    assert (item_type > IT_NONE && item_type < IT_MAX && num_level < MAP_MAX);

    /* no amulets above dungeon level 6 */
    if ((item_type == IT_AMULET) && (num_level < 6))
    {
        do
        {
            item_type = rand_1n(IT_MAX);
        } while (item_type == IT_CONTAINER);
    }

    divisor = 1 / (float)(MAP_MAX - 1);

    switch (item_type)
    {
    case IT_BOOK:
        variance = 0.2;
        break;

    case IT_RING:
        variance = 0.5;
        break;

    default:
        /* no per-map randomnization */
        return item_new_random(item_type, TRUE);
    }

    id_base = item_max_id(item_type) * (num_level * divisor);
    id_min = id_base - (item_max_id(item_type) * variance);
    id_max = id_base + (item_max_id(item_type) * variance);

    /* clean results */
    if (id_min < 1) id_min = 1;
    if (id_max < 1) id_max = 1;
    if (id_max > item_max_id(item_type)) id_max = item_max_id(item_type);

    /* create the item */
    nitem = item_new(item_type, rand_m_n(id_min, id_max));

    return item_new_finetouch(nitem);
}

item *item_new_finetouch(item *it)
{
    int i;

    assert(it != NULL);

    /* maybe the item is blessed or cursed */
    if (it->type == IT_POTION && it->id == PO_WATER)
    {
        // water is always blessed
        item_bless(it);
        it->blessed_known = TRUE;
    }
    else if (item_is_blessable(it->type) && chance(25))
    {
        /* only blessed or cursed items have a bonus / malus */
        if (item_is_optimizable(it->type))
        {
            it->bonus = rand_1n(3);
        }

        if (chance(50))
        {
            /* blessed */
            item_bless(it);
        }
        else
        {
            /* cursed */
            it->bonus = 0 - it->bonus;
            item_curse(it);
        }
    }

    if ((it->bonus != 0) && it->effects)
    {
        /* modify effects linked to the item */
        for (i = 0; i < it->effects->len; i++)
        {
            gpointer eid = (effect *)g_ptr_array_index(it->effects, i);
            effect *e = game_effect_get(nlarn, eid);
            e->amount += it->bonus;
        }
    }

    /* maybe the item is corroded */
    if (item_is_corrodible(it->type) && chance(25))
    {
        item_erode(NULL, it, rand_1n(IET_MAX), FALSE);
    }

    return it;
}

item *item_copy(item *original)
{
    item *nitem;
    guint idx = 0;

    assert(original != NULL);

    /* clone item */
    nitem = g_malloc0(sizeof(item));
    memcpy(nitem, original, sizeof(item));

    /* copy effects */
    nitem->effects = NULL;

    if (original->effects != NULL)
    {
        for (idx = 0; idx < original->effects->len; idx++)
        {
            effect *e = g_ptr_array_index(original->effects, idx);
            effect *ne = effect_copy(e);

            ne->item = nitem;
            item_effect_add(nitem, ne);
        }
    }

    /* reset inventory */
    nitem->content = NULL;

    /* regíster copy with game */
    nitem->oid = game_item_register(nlarn, nitem);

    return nitem;
}

item *item_split(item *original, guint32 count)
{
    item *nitem;

    assert(original != NULL && count < original->count);

    nitem = item_copy(original);

    nitem->count = count;
    original->count -= count;

    return nitem;
}

void item_destroy(item *it)
{
    assert(it != NULL && it->type > IT_NONE && it->type < IT_MAX);

    if (it->effects)
    {
        effect *eff;

        while (it->effects->len)
        {
            gpointer effect_id = g_ptr_array_remove_index_fast(it->effects,
                                 it->effects->len - 1);

            if ((eff = game_effect_get(nlarn, effect_id)))
                effect_destroy(eff);
        }

        g_ptr_array_free(it->effects, TRUE);
    }

    if (it->content != NULL)
    {
        inv_destroy(it->content, FALSE);
    }

    if (it->notes != NULL)
    {
        g_free(it->notes);
    }

    /* unregister item */
    game_item_unregister(nlarn, it->oid);

    g_free(it);
}

void item_serialize(gpointer oid, gpointer it, gpointer root)
{
    cJSON *ival;

    item *i = (item *)it;

    cJSON_AddItemToArray((cJSON *)root, ival = cJSON_CreateObject());

    cJSON_AddNumberToObject(ival, "oid", GPOINTER_TO_UINT(oid));
    cJSON_AddNumberToObject(ival, "type", i->type);
    cJSON_AddNumberToObject(ival, "id", i->id);
    cJSON_AddNumberToObject(ival, "bonus", i->bonus);
    cJSON_AddNumberToObject(ival, "count", i->count);

    if (i->blessed == 1) cJSON_AddTrueToObject(ival, "blessed");
    if (i->cursed == 1) cJSON_AddTrueToObject(ival, "cursed");
    if (i->blessed_known == 1) cJSON_AddTrueToObject(ival, "blessed_known");
    if (i->bonus_known == 1) cJSON_AddTrueToObject(ival, "bonus_known");

    if (i->corroded > 0) cJSON_AddNumberToObject(ival, "corroded", i->corroded);
    if (i->burnt > 0) cJSON_AddNumberToObject(ival, "burnt", i->burnt);
    if (i->rusty > 0) cJSON_AddNumberToObject(ival, "rusty", i->rusty);

    /* container content */
    if (inv_length(i->content) > 0)
    {
        cJSON_AddItemToObject(ival, "content", inv_serialize(i->content));
    }

    /* player's notes */
    if (i->notes != NULL)
    {
        cJSON_AddStringToObject(ival, "notes", i->notes);
    }

    /* effects */
    if (i->effects)
    {
        cJSON_AddItemToObject(ival, "effects", effects_serialize(i->effects));
    }
}

item *item_deserialize(cJSON *iser, struct game *g)
{
    guint oid;
    item *it;
    cJSON *obj;

    it = g_malloc0(sizeof(item));

    /* must-have attributes */
    oid = cJSON_GetObjectItem(iser, "oid")->valueint;
    it->oid = GUINT_TO_POINTER(oid);

    it->type = cJSON_GetObjectItem(iser, "type")->valueint;
    it->id = cJSON_GetObjectItem(iser, "id")->valueint;
    it->bonus = cJSON_GetObjectItem(iser, "bonus")->valueint;
    it->count = cJSON_GetObjectItem(iser, "count")->valueint;

    /* optional attributes */
    obj = cJSON_GetObjectItem(iser, "blessed");
    if (obj != NULL) it->blessed = obj->valueint;

    obj = cJSON_GetObjectItem(iser, "cursed");
    if (obj != NULL) it->cursed = obj->valueint;

    obj = cJSON_GetObjectItem(iser, "blessed_known");
    if (obj != NULL) it->blessed_known = obj->valueint;

    obj = cJSON_GetObjectItem(iser, "bonus_known");
    if (obj != NULL) it->bonus_known = obj->valueint;

    obj = cJSON_GetObjectItem(iser, "corroded");
    if (obj != NULL) it->corroded = obj->valueint;

    obj = cJSON_GetObjectItem(iser, "burnt");
    if (obj != NULL) it->burnt = obj->valueint;

    obj = cJSON_GetObjectItem(iser, "rusty");
    if (obj != NULL) it->rusty = obj->valueint;

    /* container content */
    obj = cJSON_GetObjectItem(iser, "content");
    if (obj != NULL) it->content = inv_deserialize(obj);

    /* player's notes */
    obj = cJSON_GetObjectItem(iser, "notes");
    if (obj != NULL) it->notes = g_strdup(obj->valuestring);

    /* effects */
    obj = cJSON_GetObjectItem(iser, "effects");
    if (obj != NULL) it->effects = effects_deserialize(obj);

    /* add item to game */
    g_hash_table_insert(g->items, it->oid, it);

    /* increase max_id to match used ids */
    if (g->item_max_id < oid)
        g->item_max_id = oid;

    return it;
}

int item_compare(item *a, item *b)
{
    int tmp_count, result;
    gpointer oid_a, oid_b;

    if (a->type != b->type)
    {
        return FALSE;
    }
    else if (a->type == IT_GOLD)
    {
        return TRUE;
    }

    /* as count can be different for equal items, save count of b */
    tmp_count = b->count;
    b->count = a->count;

    /* store oids (never identical!) */
    oid_a = a->oid;
    oid_b = b->oid;

    a->oid = NULL;
    b->oid = NULL;

    result = (memcmp(a, b, sizeof(item)) == 0);

    b->count = tmp_count;

    /* restore oids */
    a->oid = oid_a;
    b->oid = oid_b;

    return result;
}

int item_sort(gconstpointer a, gconstpointer b, gpointer data, gboolean force_id)
{
    gint order;
    item *item_a = game_item_get(nlarn, *((gpointer**)a));
    item *item_b = game_item_get(nlarn, *((gpointer**)b));
    player *p = (player *)data;

    if (item_a->type == item_b->type)
    {
        /* both items are of identical type. compare their names. */
        order = g_ascii_strcasecmp(item_desc_get(item_a, force_id || player_item_known(p, item_a)),
                                   item_desc_get(item_b, force_id || player_item_known(p, item_b)));
    }
    else if (item_a->type < item_b->type)
        order = -1;
    else
        order = 1;

    return order;
}

char *item_describe(item *it, int known, int singular, int definite, char *str, int str_len)
{
    GString *desc = g_string_new(NULL);
    char *add_info = NULL;
    char **add_infos = strv_new();

    assert((it != NULL) && (it->type > IT_NONE) && (it->type < IT_MAX));

    /* collect additional information */
    if (it->blessed_known)
    {
        if (it->blessed) strv_append(&add_infos, "blessed");
        else if (it->cursed) strv_append(&add_infos, "cursed");
        else strv_append(&add_infos, "uncursed");
    }

    if (it->burnt == 1) strv_append(&add_infos, "burnt");
    if (it->burnt == 2) strv_append(&add_infos, "very burnt");
    if (it->corroded == 1) strv_append(&add_infos, "corroded");
    if (it->corroded == 2) strv_append(&add_infos, "very corroded");
    if (it->rusty == 1) strv_append(&add_infos, "rusty");
    if (it->rusty == 2) strv_append(&add_infos, "very rusty");

    if (g_strv_length(add_infos))
        add_info = g_strjoinv(", ", add_infos);

    g_strfreev(add_infos);

    switch (it->type)
    {
    case IT_AMULET:
        if (known)
            g_string_append_printf(desc, "amulet of %s", item_desc_get(it, known));
        else
            g_string_append_printf(desc, "%s amulet", item_desc_get(it, known));
        break;

    case IT_AMMO:
        g_string_append_printf(desc, "%s%s", item_desc_get(it, known),
                               (!singular && it->count > 1) ? "s" : "");

        if (it->bonus_known)
        {
            g_string_append_printf(desc, " %+d", it->bonus);
        }
        break;

    case IT_ARMOUR:
        g_string_append(desc, item_desc_get(it, known));

        if (it->bonus_known)
        {
            g_string_append_printf(desc, " %+d", it->bonus);
        }

        break;

    case IT_BOOK:
        if (known)
        {
            g_string_append_printf(desc, "book%s of %s",
                                   (!singular && it->count > 1) ? "s" : "",
                                   item_desc_get(it, known));
        }
        else
            g_string_append_printf(desc, "%s book%s", item_desc_get(it, known),
                                   (!singular && it->count > 1) ? "s" : "");
        break;

    case IT_CONTAINER:
        g_string_append(desc, item_desc_get(it, known));
        break;

    case IT_GEM:
        g_string_append_printf(desc, "%d carats %s", gem_size(it), item_desc_get(it, known));
        break;

    case IT_GOLD:
        g_string_append_printf(desc, "gold piece%s", it->count > 1 ? "s" : "");
        break;

    case IT_POTION:
        if (known)
        {
            g_string_append_printf(desc, "potion%s of %s",
                                   (!singular && it->count > 1) ? "s" : "",
                                   item_desc_get(it, known));
        }
        else
            g_string_append_printf(desc, "%s potion%s", item_desc_get(it, known),
                                   (!singular && it->count > 1) ? "s" : "");
        break;

    case IT_RING:
        if (known)
            g_string_append_printf(desc, "ring of %s", item_desc_get(it, known));
        else
            g_string_append_printf(desc, "%s ring", item_desc_get(it, known));

        /* display bonus if it is known */
        if (it->bonus_known)
            g_string_append_printf(desc, " %+d", it->bonus);
        break;

    case IT_SCROLL:
        if (known)
        {
            g_string_append_printf(desc, "scroll%s of %s",
                                   (!singular && it->count > 1) ? "s" : "",
                                   item_desc_get(it, known));
        }
        else
        {
            g_string_append_printf(desc, "scroll%s labeled %s",
                                   (!singular && it->count > 1) ? "s" : "",
                                   item_desc_get(it, known));
        }
        break;

    case IT_WEAPON:
        if (weapon_is_unique(it))
        {
            g_string_append_printf(desc, "%s%s",
                                   weapon_needs_article(it) ? "the " : "",
                                   item_desc_get(it, known));

            if (it->bonus_known || add_info != NULL)
            {
                if (it->bonus_known)
                {
                    if (add_info != NULL)
                    {
                        /* bonus and additional information */
                        g_string_append_printf(desc, " (%+d, %s)", it->bonus, add_info);
                    }
                    else
                    {
                        /* bonus only */
                        g_string_append_printf(desc, " (%+d)", it->bonus);
                    }
                }
                else
                {
                    /* additional information only */
                    g_string_append_printf(desc, " (%s)", add_info);
                }
            }
        }
        else
        {
            g_string_append(desc, item_desc_get(it, known));

            if (it->bonus_known)
                g_string_append_printf(desc, " %+d", it->bonus);
        }
        break;

    default:
        break;
    }

    if (it->notes)
    {
        g_string_append_printf(desc, " (%s)",
                               (strlen(it->notes) > 5 ? "noted" : it->notes));
    }

    /* prepend additional information unless the item is an unique weapon */
    if (!(it->type == IT_WEAPON && weapon_is_unique(it)))
    {
        if (add_info != NULL)
        {
            g_string_prepend_c(desc, ' ');
            g_string_prepend(desc, add_info);
        }

        /* prepend count or article */
        gchar first_char = desc->str[0];
        g_string_prepend_c(desc, ' ');

        if ((it->count == 1) || singular)
        {
            g_string_prepend(desc, (const char *)(definite ? "the" : a_an(&first_char)));
        }
        else
        {
            g_string_prepend(desc, int2str(it->count));
        }
    }

    /* copy the assembled string to the destination buffer */
    g_strlcpy(str, desc->str, str_len);

    /* free the additional information */
    g_free(add_info);

    /* free the temporary string */
    g_string_free(desc, TRUE);

    return(str);
}

item_material_t item_material(item *it)
{
    item_material_t material;

    assert (it != NULL);

    switch (it->type)
    {
    case IT_AMULET:
        material = amulet_material(it->id);
        break;

    case IT_AMMO:
        material = ammo_material(it);
        break;

    case IT_ARMOUR:
        material = armour_material(it);
        break;

    case IT_BOOK:
        material = IM_PAPER;
        break;

    case IT_CONTAINER:
        material = container_material(it);
        break;

    case IT_GEM:
        material = IM_GEMSTONE;
        break;

    case IT_GOLD:
        material = IM_GOLD;
        break;

    case IT_POTION:
        material = IM_GLASS;
        break;

    case IT_RING:
        material = ring_material(it->id);
        break;

    case IT_SCROLL:
        material = IM_PAPER;
        break;

    case IT_WEAPON:
        material =  weapon_material(it);
        break;

    default:
        material = IM_NONE;
    }

    return material;
}

guint item_base_price(item *it)
{
    assert (it != NULL && it->type > IT_NONE && it->type < IT_MAX);

    guint price;

    switch (it->type)
    {
    case IT_AMULET:
        price = amulet_price(it);
        break;

    case IT_AMMO:
        price = ammo_price(it);
        break;

    case IT_ARMOUR:
        price = armour_price(it);
        break;

    case IT_BOOK:
        price = book_price(it);
        break;

    case IT_CONTAINER:
        price = container_price(it);
        break;

    case IT_GEM:
        price = gem_price(it);
        break;

    case IT_GOLD:
        price = 0;
        break;

    case IT_POTION:
        price = potion_price(it);
        break;

    case IT_RING:
        price = ring_price(it);
        break;

    case IT_SCROLL:
        price = scroll_price(it);
        break;

    case IT_WEAPON:
        price = weapon_price(it);
        break;

    default:
        price = 0;
    }

    return price;
}

guint item_price(item *it)
{
    assert (it != NULL && it->type > IT_NONE && it->type < IT_MAX);

    guint price = item_base_price(it);

    /* modify base prices by item's attributes */

    /* 20% price increase / decrease for every +/-1 bonus */
    if (it->bonus != 0) price = price * (1 + (0.2 * it->bonus));

    /* gem prices are not affected by being blessed or cursed */
    if (it->type == IT_GEM)
        return price;

    /* double price if blessed */
    if (it->blessed) price <<=1;

    /* half price if cursed */
    if (it->cursed) price >>=1;

    /* half price if corroded */
    if (it->corroded == 1) price >>=1;

    /* quarter price if very corroded */
    if (it->corroded == 2) price /= 4;

    /* half price if burnt */
    if (it->burnt == 1) price >>=1;

    /* quarter price if very burnt */
    if (it->burnt == 2) price /= 4;

    /* half price if rusty */
    if (it->rusty == 1) price >>=1;

    /* quarter price if very rusty */
    if (it->rusty == 2) price /= 4;

    return price;
}

int item_weight(item *it)
{
    int weight = 0;

    assert(it != NULL && it->type > IT_NONE && it->type < IT_MAX);

    switch (it->type)
    {
    case IT_AMULET:
        weight = 150;
        break;

    case IT_AMMO:
        weight =  ammo_weight(it);
        break;

    case IT_ARMOUR:
        weight =  armour_weight(it);
        break;

    case IT_BOOK:
        weight =  book_weight(it);
        break;

    case IT_POTION:
        weight =  250;
        break;

    case IT_RING:
        weight =  10;
        break;

    case IT_SCROLL:
        weight =  100;
        break;

    case IT_CONTAINER:
        weight = container_weight(it) + inv_weight(it->content);
        break;

    case IT_GOLD:
        /* Is this too heavy? is this too light?
           It should give the player a reason to use the bank. */
        weight =  4;
        break;

    case IT_GEM:
        weight =  gem_weight(it);
        break;

    case IT_WEAPON:
        weight =  weapon_weight(it);
        break;

    default:
        weight =  0;
        break;
    }

    if (item_is_stackable(it->type))
        weight = weight * it->count;

    return weight;
}

int item_colour(item *it)
{
    assert(it != NULL && it->type > IT_NONE && it->type < IT_MAX);

    switch (it->type)
    {
    case IT_AMULET:
    case IT_AMMO:
    case IT_ARMOUR:
    case IT_RING:
    case IT_WEAPON:
        return item_materials[item_material(it)].colour;
        break;

    case IT_BOOK:
        return book_colour(it);
        break;

    case IT_POTION:
        return potion_colour(it->id);
        break;

    case IT_SCROLL:
        return DC_WHITE;
        break;

    case IT_CONTAINER:
        return DC_BROWN;
        break;

    case IT_GOLD:
        return DC_YELLOW;
        break;

    case IT_GEM:
        return gem_colour(it);
        break;

    default:
        return DC_BLACK;
        break;
    }
}

void item_effect_add(item *it, effect *e)
{
    assert (it != NULL && it->oid != NULL && e != NULL);

    /* create list if not existant */
    if (!it->effects)
    {
        it->effects = g_ptr_array_new();
    }

    /* this effect is permanent */
    e->turns = 0;

    /* link item to effect */
    e->item = it->oid;

    /* add effect to list */
    effect_add(it->effects, e);
}

void item_effect_del(item *it, effect *e)
{
    assert (it != NULL && e != NULL);

    /* del effect from list */
    effect_del(it->effects, e);

    /*destroy effect */
    effect_destroy(e);

    /* delete array if no effect is left */
    if (!it->effects->len)
    {
        g_ptr_array_free(it->effects, TRUE);
        it->effects = NULL;
    }
}

int item_bless(item *it)
{
    assert (it != NULL && !it->cursed);

    if (it->blessed)
        return FALSE;

    it->blessed = TRUE;

    return TRUE;
}

int item_curse(item *it)
{
    assert (it != NULL && !it->blessed);

    if (it->cursed)
        return FALSE;

    it->cursed = TRUE;

    return TRUE;
}

int item_remove_blessing(item *it)
{
    assert (it != NULL && it->blessed);

    if (!it->blessed)
        return FALSE;

    it->blessed = FALSE;
    it->blessed_known = FALSE;

    return TRUE;
}

int item_remove_curse(item *it)
{
    assert (it != NULL && it->cursed);

    if (!it->cursed || it->blessed)
        return FALSE;

    it->cursed = FALSE;
    it->blessed_known = TRUE;

    return TRUE;
}

item *item_enchant(item *it)
{
    guint pos;
    gpointer oid;
    effect *e;
    char desc[81] = { 0 };

    assert(it != NULL);

    it->bonus++;

    /* warn against overenchantment */
    if (it->bonus == 3)
    {
        /* hide bonus from description */
        gboolean bonus_known = it->bonus_known;
        it->bonus_known = FALSE;

        item_describe(it, player_item_known(nlarn->p, it),
                      (it->count == 1), TRUE, desc, 80);

        desc[0] = g_ascii_toupper(desc[0]);
        log_add_entry(nlarn->log, "%s vibrate%s strangely.",
                      desc, (it->count == 1) ? "s" : "");

        it->bonus_known = bonus_known;
    }

    /* item has been overenchanted */
    if (it->bonus > 3)
    {
        player_item_destroy(nlarn->p, it);
        return NULL;
    }

    if ((it->type == IT_RING) && it->effects)
    {
        for (pos = 0; pos < it->effects->len; pos++)
        {
            oid = g_ptr_array_index(it->effects, pos);
            e = game_effect_get(nlarn, oid);

            e->amount++;
        }
    }

    return it;
}

item *item_disenchant(item *it)
{
    guint pos;
    gpointer oid;
    effect *e;

    assert(it != NULL);

    if (it->bonus <= -3)
    {
        player_item_destroy(nlarn->p, it);
        return NULL;
    }

    it->bonus--;

    if (it->bonus == -3)
    {
        char desc[81] = { 0 };
        item_describe(it, player_item_known(nlarn->p, it),
                      (it->count == 1), TRUE, desc, 80);

        desc[0] = g_ascii_toupper(desc[0]);
        log_add_entry(nlarn->log, "%s vibrate%s warningly.",
                      desc, (it->count == 1) ? "s" : "");

        it->bonus_known = TRUE;
    }

    if ((it->type == IT_RING) && it->effects)
    {
        for (pos = 0; pos < it->effects->len; pos++)
        {
            oid = g_ptr_array_index(it->effects, pos);
            e = game_effect_get(nlarn, oid);

            e->amount--;
        }
    }

    return it;
}

static int material_affected(item_material_t mat, item_erosion_type iet)
{
    switch (iet)
    {
    case IET_BURN:
        return (mat <= IM_BONE);
    case IET_CORRODE:
        return (mat == IM_IRON);
    case IET_RUST:
        return (mat == IM_IRON);
    default:
        return (FALSE);
    }
}

item *item_erode(inventory **inv, item *it, item_erosion_type iet, gboolean visible)
{
    gboolean destroyed = FALSE;
    char *erosion_desc = NULL;
    char item_desc[61] = { 0 };

    assert(it != NULL);

    /* Don't ever destroy the potion of cure dianthroritis.
       This is not currently possible, but add a check in case that changes. */
    if (it->type == IT_POTION && it->id == PO_CURE_DIANTHR)
        return (it);

    if (!material_affected(item_material(it), iet))
        return (it);

    if (visible)
    {
        /* prepare item description before it has been affected */
        item_describe(it, player_item_known(nlarn->p, it),
                      (it->count == 1), TRUE, item_desc, 60);
        item_desc[0] = g_ascii_toupper(item_desc[0]);
    }

    // Blessed items have a 50% chance of resisting the erosion.
    if (it->blessed && chance(50))
    {
        if (visible)
        {
            log_add_entry(nlarn->log, "%s resist%s.", item_desc,
                          (it->count == 1) ? "s" : "");
            it->blessed_known = TRUE;
        }
        return (it);
    }

    switch (iet)
    {
    case IET_BURN:
        it->burnt++;
        erosion_desc = "burn";

        if (it->burnt == 3) destroyed = TRUE;
        break;

    case IET_CORRODE:
        it->corroded++;
        erosion_desc = "corrode";

        if (it->corroded == 3) destroyed = TRUE;
        break;

    case IET_RUST:
        it->rusty++;
        erosion_desc = "rust";

        /* it's been very rusty already -> destroy */
        if (it->rusty == 3) destroyed = TRUE;
        break;

    default:
        /* well, just do nothing. */
        break;
    }

    /* if the item has an inventory, erode it as well */
    if (it->content != NULL)
        inv_erode(&it->content, iet, FALSE);

    if (erosion_desc != NULL && visible)
    {
        /* items has been eroded, describe the event if it is visible */
        log_add_entry(nlarn->log, "%s %s%s.", item_desc,
                      erosion_desc, (it->count == 1) ? "s" : "");
    }

    if (destroyed)
    {
        /* item has been destroyed */
        if (visible)
        {
            /* describe the event if the item was visible */
            log_add_entry(nlarn->log, "%s %s destroyed.", item_desc,
                          (it->count == 1) ? "is" : "are");
        }

        if (inv != NULL)
        {
            if (*inv == nlarn->p->inventory)
            {
                /* if the inventory we are working on is the player's
                 * inventory, try to unequip the item first as we do
                 * not know if it is eqipped (this would lead to nasty
                 * segementation faults otherwise) */
                log_disable(nlarn->log);
                player_item_unequip(nlarn->p, &nlarn->p->inventory, it, TRUE);
                log_enable(nlarn->log);
            }

            inv_del_element(inv, it);
        }

        if (it->content != NULL)
        {
            /* if the item is a container an still has undestroyed content,
             * this content has to be put into the items inventory, e.g. a
             * casket burns -> all undestroyed items inside the casket
             * continue to exist on the floor tile the casket was standing on
             */
            while (inv_length(it->content) > 0)
            {
                item *cit = inv_get(it->content, inv_length(it->content) - 1);
                inv_del_element(&it->content, cit);
                inv_add(inv, cit);
            }
        }

        /* remove the item from the game */
        item_destroy(it);
        it = NULL;
    }

    return it;
}

int item_obtainable(item_t type, int id)
{
    int obtainable;

    switch (type)
    {
    case IT_AMMO:
        obtainable = ammo_t_obtainable(id);
        break;

    case IT_ARMOUR:
        obtainable = TRUE;
        break;

    case IT_BOOK:
        obtainable = book_type_obtainable(id);
        break;

    case IT_CONTAINER:
        obtainable = (id == CT_BAG);
        break;

    case IT_POTION:
        obtainable = potion_type_obtainable(id);
        break;

    case IT_RING:
        obtainable = ring_type_obtainable(id);
        break;

    case IT_SCROLL:
        obtainable = scroll_type_obtainable(id);
        break;

    case IT_WEAPON:
        obtainable = weapon_type_obtainable(id);
        break;

    default:
        obtainable = FALSE;
    }

    return obtainable;
}

char *item_detailed_description(item *it, gboolean known, gboolean shop)
{
    assert (it != NULL);

    GString *desc = g_string_new("");

    switch (it->type)
    {
    case IT_AMMO:
        g_string_append_printf(desc, "Ammunition for %ss\n",
                               ammo_class_name[ammo_class(it)]);

        if (it->bonus_known)
        {
            g_string_append_printf(desc, "Damage: +%d\n"
                                   "Accuracy: +%d\n",
                                   ammo_damage(it), ammo_accuracy(it));
        }
        else
        {
            g_string_append_printf(desc, "Base damage: +%d\n"
                                   "Base accuracy: +%d\n",
                                   ammo_base_damage(it), ammo_base_accuracy(it));
        }

        break;

    case IT_BOOK:
        if (known)
        {
            g_string_append_printf(desc, "%s\nSpell level: %d\n",
                                   spell_desc_by_id(it->id),
                                   spell_level_by_id(it->id));
        }
        break;

    case IT_WEAPON:
        if (weapon_is_twohanded(it))
            g_string_append_printf(desc, "Two-handed weapon\n");

        if (weapon_is_ranged(it))
            g_string_append_printf(desc, "Ranged weapon\n");

        if (it->bonus_known)
        {
            g_string_append_printf(desc, "Damage: +%d\n"
                                   "Accuracy: +%d\n",
                                   weapon_damage(it), weapon_acc(it));
        }
        else
        {
            g_string_append_printf(desc, "Base damage: +%d\n"
                                   "Base accuracy: +%d\n",
                                   weapon_base_damage(it), weapon_base_acc(it));
        }
        break;

    case IT_ARMOUR:
        if (it->bonus_known)
        {
            g_string_append_printf(desc, "Armour class: %d\n", armour_ac(it));
        }
        else
        {
            g_string_append_printf(desc, "Base AC: %d\n", armour_base_ac(it));
        }
        break;

    default:
        break;
    }

    if (it->notes)
        g_string_append_printf(desc, "Notes: %s\n", it->notes);

    g_string_append_printf(desc, "Weight:   %.2f kg\n",
                           (float)item_weight(it) / 1000);

    g_string_append_printf(desc, "Material: %s",
                           item_material_name(item_material(it)));

    if (shop)
    {
        /* inside shop - show the item's price */
        g_string_append_printf(desc, "\nPrice:    %d gp", item_price(it));
    }

    return g_string_free(desc, FALSE);
}

inventory *inv_new(gconstpointer owner)
{
    inventory *ninv;

    ninv = g_malloc0(sizeof(inventory));
    ninv->content = g_ptr_array_new();

    ninv->owner = owner;

    return ninv;
}

void inv_destroy(inventory *inv, gboolean special)
{
    assert(inv != NULL);

    while (inv_length(inv) > 0)
    {
        item *it = inv_get(inv, inv_length(inv) - 1);

        if (special)
        {
            /* Mark potion of cure dianthroritis and eye of larn
               as never having been created. */
            if (it->type == IT_POTION && it->id == PO_CURE_DIANTHR)
                nlarn->cure_dianthr_created = FALSE;
            else if (it->type == IT_AMULET && it->id == AM_LARN)
                nlarn->amulet_created[AM_LARN] = FALSE;
        }
        g_ptr_array_remove(inv->content, it->oid);
        item_destroy(it);
    }

    g_ptr_array_free(inv->content, TRUE);

    g_free(inv);
}

cJSON *inv_serialize(inventory *inv)
{
    int idx;
    cJSON *sinv = cJSON_CreateArray();

    for (idx = 0; idx < inv_length(inv); idx++)
    {
        item *it = inv_get(inv, idx);
        cJSON_AddItemToArray(sinv, cJSON_CreateNumber(GPOINTER_TO_UINT(it->oid)));
    }

    return sinv;
}

inventory *inv_deserialize(cJSON *iser)
{
    int idx;
    inventory *inv;

    inv = g_malloc0(sizeof(inventory));

    inv->content = g_ptr_array_new();

    for (idx = 0; idx < cJSON_GetArraySize(iser); idx++)
    {
        guint oid = cJSON_GetArrayItem(iser, idx)->valueint;
        g_ptr_array_add(inv->content, GUINT_TO_POINTER(oid));
    }

    return inv;
}

void inv_callbacks_set(inventory *inv, inv_callback_bool pre_add,
                       inv_callback_void post_add, inv_callback_bool pre_del,
                       inv_callback_void post_del)
{
    assert (inv != NULL);

    inv-> pre_add  = pre_add;
    inv-> post_add = post_add;
    inv-> pre_del  = pre_del;
    inv-> post_del = post_del;
}

int inv_add(inventory **inv, item *it)
{
    guint idx;

    assert(inv != NULL && it != NULL && it->oid != NULL);

    /* create inventory if necessary */
    if (!(*inv))
    {
        *inv = inv_new(NULL);
    }

    /* check if pre_add callback is set */
    if ((*inv)->pre_add)
    {
        /* call pre_add callback */
        if (!(*inv)->pre_add(*inv, it))
        {
            return FALSE;
        }
    }

    /* stack stackable items */
    if (item_is_stackable(it->type))
    {
        /* loop through items in the target inventory to find a similar item */
        for (idx = 0; idx < inv_length(*inv); idx++)
        {
            item *i = inv_get(*inv, idx);
            /* compare the current item with the one whis is to be added */
            if (item_compare(i, it))
            {
                /* just increase item count and release the original */
                i->count += it->count;
                item_destroy(it);

                it = NULL;

                break;
            }
        }
    }

    if (it != NULL)
    {
        /* add the item to the inventory if it has not already been added */
        g_ptr_array_add((*inv)->content, it->oid);
    }

    /* call post_add callback */
    if ((*inv)->post_add)
    {
        (*inv)->post_add(*inv, it);
    }

    return inv_length(*inv);
}

item *inv_get(inventory *inv, guint idx)
{
    gpointer oid = NULL;

    assert (inv != NULL && idx < inv->content->len);
    oid = g_ptr_array_index(inv->content, idx);

    return game_item_get(nlarn, oid);
}

item *inv_del(inventory **inv, guint idx)
{
    item *item;

    assert(*inv != NULL && (*inv)->content != NULL && idx < inv_length(*inv));

    item = inv_get(*inv, idx);

    if ((*inv)->pre_del)
    {
        if (!(*inv)->pre_del(*inv, item))
        {
            return NULL;
        }
    }

    g_ptr_array_remove_index((*inv)->content, idx);

    if ((*inv)->post_del)
    {
        (*inv)->post_del(*inv, item);
    }

    /* destroy inventory if empty and not owned by anybody */
    if (!inv_length(*inv) && !(*inv)->owner)
    {
        inv_destroy(*inv, FALSE);
        *inv = NULL;
    }

    return item;
}

int inv_del_element(inventory **inv, item *it)
{
    assert(*inv != NULL && (*inv)->content != NULL && it != NULL);

    if ((*inv)->pre_del)
    {
        if (!(*inv)->pre_del(*inv, it))
        {
            return FALSE;
        }
    }

    g_ptr_array_remove((*inv)->content, it->oid);

    if ((*inv)->post_del)
    {
        (*inv)->post_del(*inv, it);
    }

    /* destroy inventory if empty and not owned by anybody */
    if (!inv_length(*inv) && !(*inv)->owner)
    {
        inv_destroy(*inv, FALSE);
        *inv = NULL;
    }

    return TRUE;
}

int inv_del_oid(inventory **inv, gpointer oid)
{
    assert(*inv != NULL && (*inv)->content != NULL && oid != NULL);

    if (!g_ptr_array_remove((*inv)->content, oid))
    {
        return FALSE;
    }

    /* destroy inventory if empty and not owned by anybody */
    if (!inv_length(*inv) && !(*inv)->owner)
    {
        inv_destroy(*inv, FALSE);
        *inv = NULL;
    }

    return TRUE;
}

void inv_erode(inventory **inv, item_erosion_type iet, gboolean visible)
{
    item *it;
    guint idx;

    assert(inv != NULL);

    for (idx = 0; idx < inv_length(*inv); idx++)
    {
        it = inv_get(*inv, idx);
        item_erode(inv, it, iet, visible);
    }
}

guint inv_length(inventory *inv)
{
    return (inv == NULL) ? 0 : inv->content->len;
}

void inv_sort(inventory *inv, GCompareDataFunc compare_func, gpointer user_data)
{
    assert(inv != NULL && inv->content != NULL);

    g_ptr_array_sort_with_data(inv->content, compare_func, user_data);
}

int inv_weight(inventory *inv)
{
    int sum = 0;
    guint idx;

    if (inv == NULL)
    {
        return 0;
    }

    /* add contents weight */
    for (idx = 0; idx < inv_length(inv); idx++)
    {
        sum += item_weight(inv_get(inv, idx));
    }

    return sum;
}

int inv_length_filtered(inventory *inv, int (*filter)(item *))
{
    int count = 0;
    guint pos;

    if (inv == NULL)
    {
        /* check for non-existant inventories */
        return 0;
    }

    /* return the inventory length if no filter has been set */
    if (!filter)
    {
        return inv_length(inv);
    }

    for (pos = 0; pos < inv_length(inv); pos++)
    {
        item *i = inv_get(inv, pos);

        if (filter(i))
        {
            count++;
        }
    }

    return count;
}

item *inv_get_filtered(inventory *inv, guint idx, int (*filter)(item *))
{
    guint num;
    guint curr = 0;

    /* return the inventory length if no filter has been set */
    if (!filter)
    {
        return inv_get(inv, idx);
    }

    for (num = 0; num < inv_length(inv); num++)
    {
        item *i = inv_get(inv, num);

        if (filter(i))
        {
            /* filter matches */
            if (curr == idx)
            {
                /* this is the requested item */
                return i;
            }
            else
            {
                curr++;
            }
        }
    }

    /* not found */
    return NULL;
}

int item_filter_container(item *it)
{
    assert (it != NULL);
    return (it->type == IT_CONTAINER);
}

int item_filter_not_container(item *it)
{
    assert (it != NULL);
    return (it->type != IT_CONTAINER);
}

int item_filter_gems(item *it)
{
    assert (it != NULL);
    return (it->type == IT_GEM);
}

int item_filter_gold(item *it)
{
    assert (it != NULL);
    return (it->type == IT_GOLD);
}

int item_filter_not_gold(item *it)
{
    assert (it != NULL);
    return (it->type != IT_GOLD);
}

int item_filter_potions(item *it)
{
    assert (it != NULL);
    return (it->type == IT_POTION);
}

int item_filter_legible(item *it)
{
    assert (it != NULL);
    return (it->type == IT_SCROLL) || (it->type == IT_BOOK);
}

int item_filter_unid(item *it)
{
    assert (it != NULL);
    return (!player_item_identified(nlarn->p, it));
}

int item_filter_cursed(item *it)
{
    assert (it != NULL);
    return (it->cursed == TRUE);
}

int item_filter_cursed_or_unknown(item *it)
{
    assert (it != NULL);
    return (it->cursed == TRUE
            || (it->blessed_known == FALSE && item_is_blessable(it->type)));
}

int item_filter_nonblessed(item *it)
{
    assert (it != NULL);
    return (it->blessed == FALSE
            || (it->blessed_known == FALSE && item_is_blessable(it->type)));
}

int item_filter_pcd(item *it)
{
    assert (it != NULL);
    return (it->type == IT_POTION && it->id == PO_CURE_DIANTHR);
}

int item_filter_blank_scroll(item *it)
{
    assert (it != NULL);
    return (it->type == IT_SCROLL && it->id == ST_BLANK);
}

static const char *item_desc_get(item *it, int known)
{
    assert(it != NULL && it->type > IT_NONE && it->type < IT_MAX);

    switch (it->type)
    {
    case IT_AMULET:
        if (known)
            return amulet_name(it);
        else
            return item_material_adjective(item_material(it));
        break;

    case IT_AMMO:
        return ammo_name(it);
        break;

    case IT_ARMOUR:
        return armour_name(it);
        break;

    case IT_BOOK:
        if (known)
            return book_name(it);
        else
            return book_desc(it->id);
        break;

    case IT_CONTAINER:
        return container_name(it);
        break;

    case IT_POTION:
        if (known)
            return potion_name(it);
        else
            return potion_desc(it->id);
        break;

    case IT_RING:
        if (known)
            return ring_name(it);
        else
            return item_material_adjective(item_material(it));
        break;

    case IT_SCROLL:
        if (known)
            return scroll_name(it);
        else
            return scroll_desc(it->id);
        break;

    case IT_GEM:
        return (char *)gem_name(it);
        break;

    case IT_WEAPON:
        return weapon_name(it);
        break;

    default:
        return "";
    }
}

