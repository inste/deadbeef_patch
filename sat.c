/*
    DeaDBeeF - ultimate music player for GNU/Linux systems with X11
    Copyright (C) 2009-2010 Alexey Yakovenko <waker@users.sourceforge.net>

	"Stop after this track" plugin
	Copyright (C) 2010 Ilya Ponetayev <instenet@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "../../deadbeef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../gettext.h"
 
DB_functions_t *deadbeef;
DB_misc_t plugin;
DB_playItem_t * __it = NULL;
static uintptr_t sat_mutex;

static int
sat_songfinished (DB_event_track_t *ev, uintptr_t data) {
	deadbeef->mutex_lock(sat_mutex);
	
	if (NULL != __it && ev->track == __it) {
		deadbeef->playback_stop();
		deadbeef->pl_lock();
		/* FIXME - now we don't care about other plugins */
		deadbeef->pl_replace_meta(ev->track, "_playing", "sat.flag=\"\" ");
		deadbeef->pl_unlock();
		deadbeef->plug_trigger_event_trackinfochanged(ev->track);	
		__it = NULL;
	}
	
	deadbeef->mutex_unlock(sat_mutex);
	return 0;
}

int stop_after_this(DB_plugin_action_t *act, DB_playItem_t *it) {
	
	deadbeef->mutex_lock(sat_mutex);
	deadbeef->pl_lock();
	
	if (NULL == __it || it != __it) {
		__it = it;
		/* FIXME - now we don't care about other plugins */
		deadbeef->pl_replace_meta(it, "_playing", "sat.flag=\"[S] \" ");
	} else {
		__it = NULL;
		/* FIXME - now we don't care about other plugins */
		deadbeef->pl_replace_meta(it, "_playing", "sat.flag=\"\" ");
	}
	deadbeef->plug_trigger_event_trackinfochanged(it);
	deadbeef->pl_unlock();
	deadbeef->mutex_unlock(sat_mutex);
	return 0;
}


int stop_after_this_start(void) {
	sat_mutex = deadbeef->mutex_create_nonrecursive();
	__it = NULL;
	deadbeef->ev_subscribe (DB_PLUGIN (&plugin), DB_EV_SONGFINISHED, DB_CALLBACK (sat_songfinished), 0);
	return 0;
}

int stop_after_this_stop(void) {
	deadbeef->ev_unsubscribe (DB_PLUGIN (&plugin), DB_EV_SONGFINISHED, DB_CALLBACK (sat_songfinished), 0);
	deadbeef->mutex_free(sat_mutex);
	return 0;
}

static DB_plugin_action_t lookup_action = {
    .title = "Stop after this track",
    .name = "stop_after_this",
    .flags = DB_ACTION_SINGLE_TRACK,
    .callback = stop_after_this,
    .next = NULL
};

static DB_plugin_action_t *
stop_after_this_get_actions (DB_playItem_t *it)
{
    lookup_action.flags &= ~DB_ACTION_DISABLED;
    return &lookup_action;
}

DB_misc_t plugin = {
    DB_PLUGIN_SET_API_VERSION
    .plugin.type = DB_PLUGIN_MISC,
    .plugin.version_major = 1,
    .plugin.version_minor = 0,
    .plugin.id = "stop_after_this",
    .plugin.name = "Stop after this track",
    .plugin.descr = "Stop after this track function",
    .plugin.author = "Ilya Ponetayev",
    .plugin.email = "instenet@gmail.com",
    .plugin.website = "http://deadbeef.sourceforge.net",
    .plugin.start = stop_after_this_start,
    .plugin.stop = stop_after_this_stop,
    .plugin.get_actions = stop_after_this_get_actions
};

DB_plugin_t *
sat_load (DB_functions_t *ddb) {
    deadbeef = ddb;
    return &plugin.plugin;
}
