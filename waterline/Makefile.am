# INCLUDES =
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/multiload \
	-DG_LOG_DOMAIN=\"waterline-multiload-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# multiload plugin
#
plugin_LTLIBRARIES = \
	multiload.la

plugindir = \
	$(libdir)/waterline/plugins

multiload_la_SOURCES = \
	../about-data.h \
	waterline-multiload-plugin.c
# xfce4-multiload-plugin.h
# xfce4-multiload-dialogs.c xfce4-multiload-dialogs.h
# xfce4-multiload-settings.c xfce4-multiload-settings.h

multiload_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(WATERLINE_CFLAGS)

multiload_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       $(PLATFORM_LDFLAGS)

# export-symbols-regex '^multiload_(constructor|destructor|configure|save_configuration|configuration_changed)'

multiload_la_LIBADD = \
	$(GTK_LIBS) \
	$(WATERLINE_LIBS) \
	../multiload/multiload-core.la

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
