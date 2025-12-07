/*
wsimplecal: a simple GTK calendar
Copyright (C) 2025 Fuzzy Dunlop

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include <time.h>
#include <gtk/gtk.h>
#include <gtk4-layer-shell/gtk4-layer-shell.h>

const char *WDAYS[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char *MONTHS[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static GtkWidget *window = NULL;
static GtkWidget *calendar = NULL;

static gboolean no_layer_shell = FALSE;
static gboolean start_hidden = FALSE;
static GtkLayerShellLayer default_layer = GTK_LAYER_SHELL_LAYER_TOP;
static gboolean default_anchors[] = {FALSE, TRUE, FALSE, TRUE};

static gboolean update_date(gpointer user_data) {
	GtkWidget *label = user_data;
	time_t s = time(NULL);
	struct tm *t = localtime(&s);
	char date[32];
	sprintf(date, "%s %d %s, %d:%02d:%02d", WDAYS[t->tm_wday], t->tm_mday, MONTHS[t->tm_mon], t->tm_hour, t->tm_min, t->tm_sec);
	gtk_label_set_text(GTK_LABEL(label), date);
	return G_SOURCE_CONTINUE;
}

static gboolean show_today(GtkWidget *widget) {
	GDateTime *now = g_date_time_new_now_local();
	gtk_calendar_set_date(GTK_CALENDAR(widget), now);
	return G_SOURCE_CONTINUE;
}

static gboolean toggle_visible(GtkWidget *widget) {
	gtk_widget_set_visible(widget, !gtk_widget_get_visible(widget));
	return G_SOURCE_CONTINUE;
}

static void activate(GtkApplication *app, gpointer user_data) {
	if (window == NULL) {
		window = gtk_application_window_new(app);

		if (!no_layer_shell) {
			gtk_layer_init_for_window(GTK_WINDOW(window));
			gtk_layer_set_layer(GTK_WINDOW(window), default_layer);
			for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++)
				gtk_layer_set_anchor(GTK_WINDOW(window), i, default_anchors[i]);
		}

		GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
		gtk_widget_add_css_class(window, "frame");
		gtk_widget_set_margin_top(box, 5);
		gtk_widget_set_margin_bottom(box, 5);
		gtk_widget_set_margin_start(box, 5);
		gtk_widget_set_margin_end(box, 5);
		gtk_window_set_child(GTK_WINDOW(window), box);

		GtkWidget *date = gtk_label_new(NULL);
		PangoAttrList *attrlist = pango_attr_list_new();
		pango_attr_list_insert(attrlist, pango_attr_font_features_new("tnum=1"));
		pango_attr_list_insert(attrlist, pango_attr_size_new(15 * PANGO_SCALE));
		gtk_label_set_attributes(GTK_LABEL(date), attrlist);
		gtk_box_append(GTK_BOX(box), date);
		g_timeout_add(100, update_date, date);

		calendar = gtk_calendar_new();
		gtk_box_append(GTK_BOX(box), calendar);

		gtk_window_present(GTK_WINDOW(window));
	} else if (no_layer_shell) {
		show_today(calendar);
		gtk_window_present(GTK_WINDOW(window));
	} else {
		show_today(calendar);
		toggle_visible(window);
	}
}

gboolean anchor_option_callback(const gchar *_option_name, const gchar *value, void *_data, GError **error) {
	for (int i = 0; i < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; i++)
		default_anchors[i] = FALSE;

	for (const char *c = value; *c; c++) {
		if (*c == 'l') {
			default_anchors[GTK_LAYER_SHELL_EDGE_LEFT] = TRUE;
		} else if (*c == 'r') {
			default_anchors[GTK_LAYER_SHELL_EDGE_RIGHT] = TRUE;
		} else if (*c == 't') {
			default_anchors[GTK_LAYER_SHELL_EDGE_TOP] = TRUE;
		} else if (*c == 'b') {
			default_anchors[GTK_LAYER_SHELL_EDGE_BOTTOM] = TRUE;
		}
	}

	return TRUE;
}

static const GOptionEntry options[] = {
	{"anchor", 'a', G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_CALLBACK, (void*)&anchor_option_callback, "A sequence of 'l', 'r', 't' and 'b' to anchor to those edges", NULL},
	{"no-layer-shell", 'n', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &no_layer_shell, "Disable gtk4-layer-shell, create a normal shell surface instead", NULL},
	G_OPTION_ENTRY_NULL
};

int main(int argc, char **argv)
{
	GError *error = NULL;
	GOptionContext *context = g_option_context_new("");
	g_option_context_add_main_entries(context, options, NULL);
	g_option_context_parse(context, &argc, &argv, &error);
	GtkApplication *app = gtk_application_new("org.wsimplecal", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	g_option_context_free(context);
	
	return status;
}
