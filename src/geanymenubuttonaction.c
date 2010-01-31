/*
 *      geanymenubuttonaction.c - this file is part of Geany, a fast and lightweight IDE
 *
 *      Copyright 2009 Enrico Tröger <enrico(dot)troeger(at)uvena(dot)de>
 *      Copyright 2009 Nick Treleaven <nick(dot)treleaven(at)btinternet(dot)com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either vergeany 2 of the License, or
 *      (at your option) any later vergeany.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* GtkAction subclass to provide a GtkMenuToolButton in a toolbar.
 * This class is missing the action_create_menu_item() function and so can't be
 * used for creating menu items. */


#include "geany.h"
#include "support.h"
#include "utils.h"
#include "geanymenubuttonaction.h"


typedef struct _GeanyMenubuttonActionPrivate		GeanyMenubuttonActionPrivate;

#define GEANY_MENU_BUTTON_ACTION_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE((obj), \
			GEANY_MENU_BUTTON_ACTION_TYPE, GeanyMenubuttonActionPrivate))


struct _GeanyMenubuttonActionPrivate
{
	GtkWidget	*menu;
};

enum
{
	BUTTON_CLICKED,
	LAST_SIGNAL
};
static guint signals[LAST_SIGNAL];


G_DEFINE_TYPE(GeanyMenubuttonAction, geany_menu_button_action, GTK_TYPE_ACTION);


static void geany_menu_button_action_finalize(GObject *object)
{
	GeanyMenubuttonActionPrivate *priv = GEANY_MENU_BUTTON_ACTION_GET_PRIVATE(object);

	g_object_unref(priv->menu);

	(* G_OBJECT_CLASS(geany_menu_button_action_parent_class)->finalize)(object);
}


static void delegate_button_activated(GtkAction *action)
{
	g_signal_emit(action, signals[BUTTON_CLICKED], 0);
}


static void geany_menu_button_action_class_init(GeanyMenubuttonActionClass *klass)
{
	GtkActionClass *action_class = GTK_ACTION_CLASS(klass);
	GObjectClass *g_object_class = G_OBJECT_CLASS(klass);

	g_object_class->finalize = geany_menu_button_action_finalize;

	action_class->activate = delegate_button_activated;
	action_class->toolbar_item_type = GTK_TYPE_MENU_TOOL_BUTTON;

	g_type_class_add_private(klass, sizeof(GeanyMenubuttonActionPrivate));

	signals[BUTTON_CLICKED] = g_signal_new("button-clicked",
										G_TYPE_FROM_CLASS(klass),
										(GSignalFlags) 0,
										0,
										0,
										NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 0);
}


static void geany_menu_button_action_init(GeanyMenubuttonAction *action)
{
	/* nothing to do */
}


GtkAction *geany_menu_button_action_new(const gchar *name,
										const gchar *label,
									    const gchar *tooltip,
										const gchar *stock_id)
{
	GtkAction *action = g_object_new(GEANY_MENU_BUTTON_ACTION_TYPE,
		"name", name,
		"label", label,
		"tooltip", tooltip,
		"stock-id", stock_id,
		NULL);

	return action;
}


GtkWidget *geany_menu_button_action_get_menu(GeanyMenubuttonAction *action)
{
	GeanyMenubuttonActionPrivate *priv;

	g_return_val_if_fail(action != NULL, NULL);

	priv = GEANY_MENU_BUTTON_ACTION_GET_PRIVATE(action);

	return priv->menu;
}


static void menu_items_changed_cb(GtkContainer *container, GtkWidget *widget, GeanyMenubuttonAction *action)
{
	GeanyMenubuttonActionPrivate *priv;
	gboolean enable;
	GSList *l;

	g_return_if_fail(action != NULL);

	priv = GEANY_MENU_BUTTON_ACTION_GET_PRIVATE(action);
	if (priv->menu != NULL)
		enable = (g_list_length(gtk_container_get_children(GTK_CONTAINER(priv->menu))) > 0);
	else
		enable = FALSE;

	if (enable)
	{
		foreach_slist(l, gtk_action_get_proxies(GTK_ACTION(action)))
		{
			if (gtk_menu_tool_button_get_menu(GTK_MENU_TOOL_BUTTON(l->data)) == NULL)
				gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(l->data), priv->menu);
		}
	}
	else
	{
		foreach_slist(l, gtk_action_get_proxies(GTK_ACTION(action)))
		{
			gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(l->data), NULL);
		}
	}
}


void geany_menu_button_action_set_menu(GeanyMenubuttonAction *action, GtkWidget *menu)
{
	GeanyMenubuttonActionPrivate *priv;

	g_return_if_fail(action != NULL);

	priv = GEANY_MENU_BUTTON_ACTION_GET_PRIVATE(action);

	if (priv->menu != NULL)
		g_signal_handlers_disconnect_by_func(priv->menu, menu_items_changed_cb, action);
	if (menu != NULL)
	{
		g_signal_connect(menu, "add", G_CALLBACK(menu_items_changed_cb), action);
		g_signal_connect(menu, "remove", G_CALLBACK(menu_items_changed_cb), action);
	}

	priv->menu = menu;

	menu_items_changed_cb(GTK_CONTAINER(menu), NULL, action);
}
