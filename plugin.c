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
#include <libmpd/libmpdclient.h>
#include <gmpc/misc.h>

void pauseafter_set_enabled(int enabled);
int pauseafter_get_enabled(void);

int pauseafter_tool_menu(GtkWidget *menu);

extern GtkTreeModel *playlist;
extern GtkBuilder *pl3_xml;
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



GtkWidget * track, *album, *ntracks;

enum {PA_NONE, PA_TRACK, PA_ALBUM, PA_N} pauseafter_what = PA_NONE;

void pauseafter_track()
{
    if (pauseafter_what == PA_NONE){
        pauseafter_what = PA_TRACK;
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(track), gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU));
    } else {
        pauseafter_what = PA_NONE;
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(track), NULL);
    }
}



void pauseafter_album()
{
    if (pauseafter_what == PA_NONE){
        pauseafter_what = PA_ALBUM;
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(album), gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU));
    } else {
        pauseafter_what = PA_NONE;
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(album), NULL);
    }
}

gulong N = 0;
gulong pauseafter_getN();

void pauseafter_ntracks()
{
    N = pauseafter_getN();  
    if (N){
        pauseafter_what = PA_N;
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ntracks), gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU));
    } else {
        pauseafter_what = PA_NONE;
        gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ntracks), NULL);
    }
}

gulong pauseafter_getN(){
    GtkWidget *dialog = NULL;
    GtkWidget *label, *spin, *box;
    GtkCellRenderer * renderer;

    dialog = gtk_dialog_new_with_buttons("How much tracks to wait?", 
            GTK_WINDOW(gtk_builder_get_object(pl3_xml, "pl3_win")), 
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, 
            GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, 
            GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
            NULL);

    spin = gtk_spin_button_new( gtk_adjustment_new(0.,0.,100000.,1.,5.,5.) , 1., 0);
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin), N );

    box = gtk_dialog_get_content_area( GTK_DIALOG(dialog) );
    gtk_box_pack_start(GTK_BOX(box), spin, TRUE, TRUE, 0);

    gulong res = 0;
    gtk_widget_show_all(dialog);
    switch(gtk_dialog_run(GTK_DIALOG(dialog))){
        case GTK_RESPONSE_ACCEPT:
            res = floor( gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin)) );
        default:
            break;
    }

    gtk_widget_destroy(dialog);
    return res;
}

static void pauseafter_mpd_status_changed(MpdObj *mi, ChangedStatusType what, void *data){
    mpd_Song *cur_song, *prev_song;
    int cur_song_pos;
    if (what&MPD_CST_SONGID){
        switch(pauseafter_what){
            case PA_TRACK:
                mpd_player_pause(mi);
                gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(track), NULL);
                pauseafter_what = PA_NONE;
                break;
            case PA_ALBUM:
                cur_song_pos = mpd_player_get_current_song_pos(mi);
                cur_song = mpd_playlist_get_song_from_pos( mi, cur_song_pos );
                prev_song = mpd_playlist_get_song_from_pos( mi, cur_song_pos - 1 );
                if (strcmp(cur_song->album, prev_song->album)){
                    mpd_player_pause(mi);
                    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(album), NULL);
                    pauseafter_what = PA_NONE;
                }
                break;
            case PA_N:
                if (!--N){
                    mpd_player_pause(mi);
                    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ntracks), NULL);
                    pauseafter_what = PA_NONE;
                } 
                break;   
        }
    }
}

int pauseafter_tool_menu(GtkWidget *menu)
{
    GtkWidget *item, *submenu;
    if(!pauseafter_get_enabled()) return;


    item = gtk_image_menu_item_new_with_label("Pause after...");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU));
 
    submenu = gtk_menu_new();
    track = gtk_image_menu_item_new_with_label("Track");
    album = gtk_image_menu_item_new_with_label("Album");
    ntracks = gtk_image_menu_item_new_with_label("N tracks");
    gtk_menu_append(GTK_MENU(submenu), track);
    gtk_menu_append(GTK_MENU(submenu), album);
    gtk_menu_append(GTK_MENU(submenu), ntracks); 
    g_signal_connect(G_OBJECT(track), "activate", G_CALLBACK(pauseafter_track), NULL);    
    g_signal_connect(G_OBJECT(album), "activate", G_CALLBACK(pauseafter_album), NULL);    
    g_signal_connect(G_OBJECT(ntracks), "activate", G_CALLBACK(pauseafter_ntracks), NULL);   

    gtk_menu_item_set_submenu(item, submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    return 1;
}


