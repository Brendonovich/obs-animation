/*
obs-animation-filter
Copyright (C) 2022 Brendan Allan <brendonovich@outlook.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include "animation-filter.h"

#include "plugin-macros.generated.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

obs_websocket_vendor vendor;

extern struct obs_source_info animation_filter;

bool obs_module_load(void)
{
    state = bzalloc(sizeof(global_state_t));
    da_init(state->infos);
    
    blog(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
    obs_register_source(&animation_filter);

    return true;
}

void obs_module_post_load(void)
{
    vendor = obs_websocket_register_vendor("obs-animation-filter");
    if (!vendor)
    {
        blog(LOG_ERROR, "Vendor registration failed!");
        return;
    }

     if (!obs_websocket_vendor_register_request(vendor, "SetAnimation", set_animation_cb, NULL))
         blog(LOG_ERROR, "Failed to register `set_animation` request with obs-websocket.");
}

void obs_module_unload()
{
    da_free(state->infos);
    bfree(state);

    blog(LOG_INFO, "plugin unloaded");
}