#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n-lib.h>
#include <sde-utils-jansson.h>

#define PLUGIN_PRIV_TYPE MultiloadWaterlinePlugin

#include <waterline/plugin.h>
#include <waterline/misc.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "multiload/multiload.h"
#include "multiload/properties.h"

/** BEGIN H **/
typedef struct _MultiloadWaterlinePlugin MultiloadWaterlinePlugin;

/* an instance of this struct is what will be assigned to 'priv' */
struct _MultiloadWaterlinePlugin
{
    MultiloadPlugin ma;
    GtkWidget *dlg;
};
/** END H **/

#if 0
static void
multiload_read(char **fp, MultiloadPlugin *ma)
{
  guint i, found = 0;

  /* Initial settings */
  ma->speed = 0;
  ma->size = 0;
  for ( i = 0; i < NGRAPHS; i++ )
    {
      /* Default visibility and colors */
      ma->graph_config[i].visible = FALSE;
      multiload_colorconfig_default(ma, i);
    }

  if ( fp != NULL )
    {
      line s;
      while ( lxpanel_get_line(fp, &s) != LINE_BLOCK_END )
        {
          if ( s.type == LINE_VAR )
            {
              if ( g_ascii_strcasecmp(s.t[0], "speed") == 0 )
                ma->speed = atoi(s.t[1]);
              else if ( g_ascii_strcasecmp(s.t[0], "size") == 0 )
                ma->size = atoi(s.t[1]);
              else
                {
                  const char *suffix; /* Set by multiload_find_graph_by_name */
                  int i = multiload_find_graph_by_name(s.t[0], &suffix);

                  if ( suffix == NULL || i < 0 || i >= NGRAPHS )
                    continue;
                  else if ( g_ascii_strcasecmp(suffix, "Visible") == 0 )
                    ma->graph_config[i].visible = atoi(s.t[1]) ? TRUE : FALSE;
                  else if ( g_ascii_strcasecmp(suffix, "Colors") == 0 )
                    multiload_colorconfig_unstringify(ma, i, s.t[1]);
                }
            }
          else
            {
              ERR ("Failed to parse config token %s\n", s.str);
              break;
            }
        }
    }

    /* Handle errors from atoi */
    if ( ma->speed == 0 )
      ma->speed = DEFAULT_SPEED;
    if ( ma->size == 0 )
      ma->size = DEFAULT_SIZE;
    /* Ensure at lease one graph is visible */
    for ( i = 0; i < NGRAPHS; i++ )
      if ( ma->graph_config[i].visible == TRUE )
        found++;
    if ( found == 0 )
      ma->graph_config[0].visible = TRUE;
}

static void multiload_save_configuration(Plugin * p, FILE * fp)
{
  PLUGIN_PRIV_TYPE * multiload = PRIV(p);
  MultiloadPlugin *ma = &multiload->ma;
  guint i;

  /* Write size and speed */
  lxpanel_put_int (fp, "speed", ma->speed);
  lxpanel_put_int (fp, "size", ma->size);

  for ( i = 0; i < NGRAPHS; i++ )
    {
      char *key, list[8*MAX_COLORS];

      /* Visibility */
      key = g_strdup_printf("%sVisible", graph_types[i].name);
      lxpanel_put_int (fp, key, ma->graph_config[i].visible);
      g_free (key);

      /* Save colors */
      multiload_colorconfig_stringify (ma, i, list);
      key = g_strdup_printf("%sColors", graph_types[i].name);
      lxpanel_put_str(fp, key, list);
      g_free (key);
    }
}
#endif

static void multiload_apply_defaults(Plugin * p)
{
    PLUGIN_PRIV_TYPE * multiload = PRIV(p);
    MultiloadPlugin * ma = &multiload->ma;

    guint i;

    /* Initial settings */
    ma->speed = DEFAULT_SPEED;
    ma->size = DEFAULT_SIZE;
    for ( i = 0; i < NGRAPHS; i++ )
    {
        ma->graph_config[i].visible = FALSE;
        multiload_colorconfig_default(ma, i);
    }
    ma->graph_config[0].visible = TRUE;
}

static void multiload_read_configuration(Plugin * p)
{
    PLUGIN_PRIV_TYPE * multiload = PRIV(p);
    MultiloadPlugin * ma = &multiload->ma;
    json_t * json = plugin_inner_json(p);

    ma->speed = su_json_dot_get_int(json, "speed", ma->speed);
    ma->size = su_json_dot_get_int(json, "size", ma->size);

    int visible_count = 0;
    guint i;
    for (i = 0; i < NGRAPHS; i++ )
    {
        /* Visibility */
        {
            gchar * key = g_strdup_printf("%s_visible", graph_types[i].name);
            ma->graph_config[i].visible = su_json_dot_get_bool(json, key, ma->graph_config[i].visible);
            if (ma->graph_config[i].visible)
                visible_count++;
            g_free(key);
        }

        /* Colors */
        {
              gchar * key = g_strdup_printf("%s_colors", graph_types[i].name);
              gchar * value = NULL; su_json_dot_get_string(json, key, NULL, &value);
              if (value)
              {
                  multiload_colorconfig_unstringify(ma, i, value);
                  g_free(value);
              }
              g_free(key);
        }
    }

}

static void multiload_save_configuration(Plugin * p)
{
    PLUGIN_PRIV_TYPE * multiload = PRIV(p);
    MultiloadPlugin * ma = &multiload->ma;
    json_t * json = plugin_inner_json(p);

    su_json_dot_set_int(json, "speed", ma->speed);
    su_json_dot_set_int(json, "size", ma->size);

    guint i;
    for (i = 0; i < NGRAPHS; i++ )
    {
        /* Visibility */
        {
            gchar * key = g_strdup_printf("%s_visible", graph_types[i].name);
            su_json_dot_set_bool(json, key, ma->graph_config[i].visible);
            g_free(key);
        }

        /* Colors */
        {
              gchar * key = g_strdup_printf("%s_colors", graph_types[i].name);
              char value[8 * MAX_COLORS];
              multiload_colorconfig_stringify(ma, i, value);
              su_json_dot_set_string(json, key, value);
              g_free(key);
        }
    }

}

static void multiload_panel_configuration_changed(Plugin *p)
{
  PLUGIN_PRIV_TYPE * multiload = PRIV(p);

  /* Determine orientation and size */
  GtkOrientation orientation =
      (plugin_get_orientation(p) == GTK_ORIENTATION_VERTICAL) ?
      GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;
/*  int size = (orientation == GTK_ORIENTATION_VERTICAL) ?
      p->panel->width : p->panel->height;
*/
  int size = panel_get_oriented_height_pixels(plugin_panel(p));
  if ( orientation == GTK_ORIENTATION_HORIZONTAL )
    gtk_widget_set_size_request (plugin_widget(p), -1, size);
  else
    gtk_widget_set_size_request (plugin_widget(p), size, -1);

  /* Refresh the panel applet */
  multiload_refresh(&(multiload->ma), orientation);
}

static gboolean
multiload_press_event(GtkWidget *pwid, GdkEventButton *event, Plugin *p)
{
  /* Standard right-click handling. */
  if (plugin_button_press_event(pwid, event, p))
    return TRUE;

  if (event->button == 1)    /* left button */
    {
      /* Launch system monitor */
    }
  return TRUE;
}

static int
multiload_constructor(Plugin *p)
{
  /* allocate our private structure instance */
  PLUGIN_PRIV_TYPE * multiload = g_new0(PLUGIN_PRIV_TYPE, 1);
  plugin_set_priv(p, multiload);

  /* Initialize multiload */
  multiload_init ();
  multiload->dlg = NULL;

  /* read the user settings */
  multiload_apply_defaults(p);
  multiload_read_configuration (p);

  /* create a container widget */
  plugin_set_widget(p, gtk_event_box_new());
  gtk_widget_show (plugin_widget(p));

  /* Initialize the applet */
  multiload->ma.container = GTK_CONTAINER(plugin_widget(p));
  /* Set size request and update orientation */
  multiload_panel_configuration_changed(p);

  g_signal_connect(plugin_widget(p), "button-press-event",
                   G_CALLBACK(multiload_press_event), p);
  /* FIXME: No way to add system monitor item to menu? */

  return 1;
}

static void
multiload_destructor(Plugin * p)
{
  /* find our private structure instance */
  PLUGIN_PRIV_TYPE * multiload = PRIV(p);

  /* Destroy dialog */
  if ( multiload->dlg )
    {
      gtk_widget_destroy (multiload->dlg);
      multiload->dlg = NULL;
    }

  /* free private data. Panel will free pwid for us. */
  g_free(multiload);
}

static void
multiload_configure_response (GtkWidget             * dialog,
                              gint                    response,
                              PLUGIN_PRIV_TYPE      * multiload)
{
  gboolean result;

  if (response == GTK_RESPONSE_HELP)
    {
      /* show help */
      /* FIXME: Not all common versions of xdg-open support lxde -2012-06-25 */
      result = g_spawn_command_line_async ("xdg-open --launch WebBrowser "
                                           PLUGIN_WEBSITE, NULL);

      if (G_UNLIKELY (result == FALSE))
        g_warning (_("Unable to open the following url: %s"), PLUGIN_WEBSITE);
    }
  else
    {
      /* destroy the properties dialog */
      gtk_widget_destroy (multiload->dlg);
      multiload->dlg = NULL;
    }
}

/* Lookup the MultiloadPlugin object from the preferences dialog. */
/* Called from multiload/properties.c */
MultiloadPlugin *
multiload_configure_get_plugin (GtkWidget *widget)
{
  GtkWidget *toplevel = gtk_widget_get_toplevel (widget);
  MultiloadPlugin *ma = NULL;
  if ( G_LIKELY (gtk_widget_is_toplevel (toplevel)) )
    ma = g_object_get_data(G_OBJECT(toplevel), "MultiloadPlugin");
  else
    g_assert_not_reached ();
  g_assert( ma != NULL);
  return ma;
}

static void multiload_configure(Plugin * p, GtkWindow * parent)
{
  GtkWidget *dialog;
  PLUGIN_PRIV_TYPE * multiload = PRIV(p);
  if ( multiload->dlg != NULL )
    {
      gtk_widget_show_all (multiload->dlg);
      gtk_window_present (GTK_WINDOW (multiload->dlg));
      return;
    }

  /* create the dialog */
  multiload->dlg = gtk_dialog_new_with_buttons
      (_("Multiload"),
       parent,
       GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
       GTK_STOCK_HELP, GTK_RESPONSE_HELP,
       GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
       NULL);
  dialog = multiload->dlg;

  /* center dialog on the screen */
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

  /* set dialog icon */
  gtk_window_set_icon_name (GTK_WINDOW (dialog),
                            "utilities-system-monitor");

  /* link the dialog to the plugin, so we can destroy it when the plugin
   * is closed, but the dialog is still open */
  g_object_set_data (G_OBJECT (dialog),
                     "MultiloadPlugin", &multiload->ma);

  /* Initialize dialog widgets */
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  multiload_init_preferences(dialog, &multiload->ma);

  /* connect the reponse signal to the dialog */
  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK(multiload_configure_response), multiload);

  /* Magic incantation from lxpanel/src/plugins/launchbar.c */
  /* Establish a callback when the dialog completes. */
  g_object_weak_ref(G_OBJECT(dialog), (GWeakNotify) plugin_save_configuration, p);

  /* show the entire dialog */
  gtk_widget_show_all (dialog);
}

/* Plugin descriptor. */
PluginClass multiload_plugin_class = {

   // this is a #define taking care of the size/version variables
   PLUGINCLASS_VERSIONING,

   // type of this plugin
   type : "multiload",
   name : N_("Multiload"),
   version: PACKAGE_VERSION,
   description : N_("A system load monitor that graphs processor, memory, "
                   "and swap space use, plus network and disk activity."),
   category: PLUGIN_CATEGORY_HW_INDICATOR,

   // we can have many running at the same time
   one_per_system : FALSE,

   // can't expand this plugin
   expand_available : FALSE,

   // assigning our functions to provided pointers.
   constructor : multiload_constructor,
   destructor  : multiload_destructor,
   show_properties : multiload_configure,
   save_configuration : multiload_save_configuration,
   panel_configuration_changed : multiload_panel_configuration_changed
};

