# INCLUDES =
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/multiload \
	-DG_LOG_DOMAIN=\"xfce4-multiload-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
plugin_LTLIBRARIES = \
	libmultiload.la

plugindir = \
	$(libdir)/xfce4/panel/plugins

libmultiload_la_SOURCES = \
	../about-data.h \
	xfce4-multiload-plugin.c xfce4-multiload-plugin.h \
	xfce4-multiload-dialogs.c xfce4-multiload-dialogs.h \
	xfce4-multiload-settings.c xfce4-multiload-settings.h

libmultiload_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(XFCE4_CFLAGS) $(XFCE4UI_CFLAGS) $(XFCEGUI4_CFLAGS)

libmultiload_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       -export-symbols-regex '^xfce_panel_(module_(preinit|init|construct)|plugin_get_(check|construct))' \
       $(PLATFORM_LDFLAGS)

libmultiload_la_LIBADD = \
	$(GTK_LIBS) \
	$(XFCE4_LIBS) $(XFCE4UI_LIBS) $(XFCEGUI4_LIBS) \
	../multiload/multiload-core.la

#
# Desktop file
#
if XFCE_IS_4_6
desktopdir = \
	$(datadir)/xfce4/panel-plugins
else
desktopdir = \
	$(datadir)/xfce4/panel/plugins
endif

desktop_DATA = \
	multiload.desktop

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	multiload.desktop.in.in

CLEANFILES = \
	$(desktop_DATA)

DISTCLEANFILES = \
	multiload.desktop.in

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
