desktop = iptux.desktop
logo_64 = hicolor/64x64/apps/ip-tux.png hicolor/64x64/apps/i-tux.png
logo_48 = hicolor/48x48/apps/ip-tux.png hicolor/48x48/apps/i-tux.png
logo_32 = hicolor/32x32/apps/ip-tux.png hicolor/32x32/apps/i-tux.png
logo_24 = hicolor/24x24/apps/ip-tux.png hicolor/24x24/apps/i-tux.png
logo_22 = hicolor/22x22/apps/ip-tux.png hicolor/22x22/apps/i-tux.png
logo_16 = hicolor/16x16/apps/ip-tux.png hicolor/16x16/apps/i-tux.png
logo = $(logo_64) $(logo_48) $(logo_32) $(logo_24) $(logo_22) $(logo_16)
EXTRA_DIST = $(desktop) $(logo)

desktopdir = @desktopdir@
desktop_DATA = $(desktop)
logodir = @logodir@
nobase_dist_logo_DATA = $(logo)


gtk_update_icon_cache = `$(PKG_CONFIG) gtk+-2.0 --variable=exec_prefix`/bin/gtk-update-icon-cache
index_theme = $(logodir)/hicolor/index.theme

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test ! -f "$(index_theme)"; then \
		touch $(index_theme); \
	fi; \
	if test -x "$(gtk_update_icon_cache)"; then \
		$(gtk_update_icon_cache) -f $(logodir)/hicolor; \
	else \
		gtk-update-icon-cache -f $(logodir)/hicolor; \
	fi
