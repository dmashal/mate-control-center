/* -*- mode: C; c-basic-offset: 4 -*-
 * themus - utilities for MATE themes
 * Copyright (C) 2002 Jonathan Blandford <aes@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <gio/gio.h>
#include <mate-wm-manager.h>
#include "mate-theme-apply.h"
#include "gtkrc-utils.h"

#define INTERFACE_SCHEMA        "org.mate.interface"
#define GTK_THEME_KEY           "gtk-theme"
#define COLOR_SCHEME_KEY        "gtk-color-scheme"
#define ICON_THEME_KEY          "icon-theme"
#define FONT_KEY                "font-name"

#define MOUSE_SCHEMA            "org.mate.peripherals-mouse"
#define CURSOR_FONT_KEY         "cursor-font"
#define CURSOR_THEME_KEY        "cursor-theme"
#define CURSOR_SIZE_KEY         "cursor-size"

#define NOTIFICATION_SCHEMA     "org.mate.NotificationDaemon"
#define NOTIFICATION_THEME_KEY  "theme"

#define compare(x,y) (!x && y) || (x && !y) || (x && y && strcmp (x, y))

void
mate_meta_theme_set (MateThemeMetaInfo *meta_theme_info)
{
  GSettings *interface_settings;
  GSettings *mouse_settings;
  GSettings *notification_settings = NULL;
  const char * const *schemas;
  gboolean schema_exists;
  gint i;
  gchar *old_key;
  gint old_key_int;
  MateWindowManager *window_manager;
  MateWMSettings wm_settings;

  mate_wm_manager_init ();

  window_manager = mate_wm_manager_get_current (gdk_display_get_default_screen (gdk_display_get_default ()));

  interface_settings = g_settings_new (INTERFACE_SCHEMA);
  mouse_settings = g_settings_new (MOUSE_SCHEMA);
  
  /* We  need this because mate-control-center does not depend on mate-notification-daemon,
   * and if we try to get notification theme without schema installed, gsettings crashes
   * see https://bugzilla.gnome.org/show_bug.cgi?id=651225 */
  schemas = g_settings_list_schemas ();
  schema_exists = FALSE;
  for (i = 0; schemas[i] != NULL; i++) {
    if (g_strcmp0 (schemas[i], NOTIFICATION_SCHEMA) == 0) {
      schema_exists = TRUE;
      break;
      }
  }
  if (schema_exists == TRUE) {
      notification_settings = g_settings_new (NOTIFICATION_SCHEMA);
  }

  /* Set the gtk+ key */
  old_key = g_settings_get_string (interface_settings, GTK_THEME_KEY);
  if (compare (old_key, meta_theme_info->gtk_theme_name))
    {
      g_settings_set_string (interface_settings, GTK_THEME_KEY, meta_theme_info->gtk_theme_name);
    }
  g_free (old_key);

  /* Set the color scheme key */
  old_key = g_settings_get_string (interface_settings, COLOR_SCHEME_KEY);
  if (compare (old_key, meta_theme_info->gtk_color_scheme))
    {
      /* only save the color scheme if it differs from the default
         scheme for the selected gtk theme */
      gchar *newval, *gtkcols;

      newval = meta_theme_info->gtk_color_scheme;
      gtkcols = gtkrc_get_color_scheme_for_theme (meta_theme_info->gtk_theme_name);

      if (newval == NULL || !strcmp (newval, "") ||
          mate_theme_color_scheme_equal (newval, gtkcols))
        {
          g_settings_reset (interface_settings, COLOR_SCHEME_KEY);
        }
      else
        {
          g_settings_set_string (interface_settings, COLOR_SCHEME_KEY, newval);
        }
      g_free (gtkcols);
    }
  g_free (old_key);

  /* Set the wm key */
  wm_settings.flags = MATE_WM_SETTING_THEME;
  wm_settings.theme = meta_theme_info->marco_theme_name;
  if (window_manager)
    mate_window_manager_change_settings (window_manager, &wm_settings);

  /* set the icon theme */
  old_key = g_settings_get_string (interface_settings, ICON_THEME_KEY);
  if (compare (old_key, meta_theme_info->icon_theme_name))
    {
      g_settings_set_string (interface_settings, ICON_THEME_KEY, meta_theme_info->icon_theme_name);
    }
  g_free (old_key);

  /* set the notification theme */
  if (notification_settings != NULL)
    {
    if (meta_theme_info->notification_theme_name != NULL)
      {
        old_key = g_settings_get_string (notification_settings, NOTIFICATION_THEME_KEY);
        if (compare (old_key, meta_theme_info->notification_theme_name))
          {
            g_settings_set_string (notification_settings, NOTIFICATION_THEME_KEY, meta_theme_info->notification_theme_name);
          }
        g_free (old_key);
      }
    }

  /* Set the cursor theme key */
#ifdef HAVE_XCURSOR
  old_key = g_settings_get_string (mouse_settings, CURSOR_THEME_KEY);
  if (compare (old_key, meta_theme_info->cursor_theme_name))
    {
      g_settings_set_string (mouse_settings, CURSOR_THEME_KEY, meta_theme_info->cursor_theme_name);
    }

  old_key_int = g_settings_get_int (mouse_settings, CURSOR_SIZE_KEY);
  if (old_key_int != meta_theme_info->cursor_size)
    {
      g_settings_set_int (mouse_settings, CURSOR_SIZE_KEY, meta_theme_info->cursor_size);
    }
#else
  old_key = g_settings_get_string (mouse_settings, CURSOR_FONT_KEY);
  if (compare (old_key, meta_theme_info->cursor_theme_name))
    {
      g_settings_set_string (mouse_settings, CURSOR_FONT_KEY, meta_theme_info->cursor_theme_name);
    }
#endif

  g_free (old_key);
  g_object_unref (interface_settings);
  g_object_unref (mouse_settings);
  if (notification_settings != NULL)
    g_object_unref (notification_settings);
}
