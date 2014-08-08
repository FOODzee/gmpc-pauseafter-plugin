/* gmpc-pauseafter (GMPC plugin)
 * Copyright (C) 2006-2009 Qball Cow <qball@sarine.nl>
 * Project homepage: http://gmpcwiki.sarine.nl/
 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <gtk/gtk.h>
#include <gmpc/plugin.h>
#include <glade/glade.h>
#include <libmpd/libmpd-internal.h>
#include <gmpc/misc.h>

void pauseafter_set_enabled(int enabled);
int pauseafter_get_enabled(void);

int pauseafter_tool_menu(GtkWidget *menu);

extern GtkTreeModel *playlist;
/* Allow gmpc to check the version the plugin is compiled against */
int plugin_api_version = PLUGIN_API_VERSION;

/* mpd event */
static void pauseafter_mpd_status_changed(MpdObj *mi, ChangedStatusType what, void *data);

/** 
 * Define the plugin structure
 */
gmpcPlugin plugin = {
	/* name */
	.name = "Pause after...",
	/* version */
	.version = {0,15,0},
	/* type */
	.plugin_type = GMPC_PLUGIN_NO_GUI,
	/** enable/disable */
	.get_enabled = pauseafter_get_enabled,
	.set_enabled = pauseafter_set_enabled,

    .mpd_status_changed = pauseafter_mpd_status_changed, 
    .tool_menu_integration = pauseafter_tool_menu
};




int pauseafter_get_enabled(void)
{
	return cfg_get_single_value_as_int_with_default(config, "pauseafter", "enable", TRUE);
}
void pauseafter_set_enabled(int enabled)
{
	cfg_set_single_value_as_int(config, "pauseafter", "enable", enabled);
	pl3_tool_menu_update(); 
}




enum {PA_NONE, PA_TRACK, PA_ALBUM, PA_N} pauseafter_what = PA_NONE;

void pauseafter_track()
{
    pauseafter_what = PA_TRACK;
}

gulong N = 0;
void pauseafter_ntracks()
{
    pauseafter_what = PA_N;
}


static void pauseafter_mpd_status_changed(MpdObj *mi, ChangedStatusType what, void *data){
    if (what&MPD_CST_SONGID){
        switch(pauseafter_what){
            case PA_TRACK:
                mpd_player_pause(mi);
                break;
        }
    }
}

int pauseafter_tool_menu(GtkWidget *menu)
{
    GtkWidget *item, *submenu;
    if(!pauseafter_get_enabled()) return;


    item = gtk_image_menu_item_new_with_label("Pause after...");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item),
            gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU));
 
    submenu = gtk_menu_new();
    GtkWidget * track, *album, *ntracks;
    track = gtk_menu_item_new_with_label("track");
    //album = gtk_menu_item_new_with_label("album");
    ntracks = gtk_menu_item_new_with_label("N tracks");
    gtk_menu_append(GTK_MENU(submenu), track);
    //gtk_menu_append(GTK_MENU(submenu), album);
    gtk_menu_append(GTK_MENU(submenu), ntracks); 
    g_signal_connect(G_OBJECT(track), "activate", G_CALLBACK(pauseafter_track), NULL);    
    //g_signal_connect(G_OBJECT(album), "activate", G_CALLBACK(shufpl_byalbum), NULL);    
    g_signal_connect(G_OBJECT(artist), "activate", G_CALLBACK(pauseafter_ntracks), NULL);   

    gtk_menu_item_set_submenu(item, submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    return 1;
}


