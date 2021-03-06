/*
 *  This file is part of Rabbits
 *  Copyright (C) 2015  Clement Deschamps and Luc Michel
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "ui.h"

#ifdef CONFIG_SDL_UI
# include "sdl/ui.h"
#else
# include "dummy/ui.h"
#endif

#include "rabbits-common.h"

ui * ui::singleton = NULL;

ui * ui::get_ui()
{
    if (ui::singleton == NULL) {
#ifdef CONFIG_SDL_UI
        ui::singleton = new sdl_ui;
#else
        ui::singleton = new dummy_ui;
#endif
    }

    return ui::singleton;
}

void ui::start_ui()
{
    get_ui();
    /* TODO: Idle screen */
}
