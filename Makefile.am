SUBDIRS = sbc googleapis/base src
ACLOCAL_AMFLAGS = -I m4
if SYSTEMD
systemdunitdir = @SYSTEMD_UNITDIR@
systemdunit_DATA = a2dp.service a2dp.target
endif
EXTRA_DIST = a2dp.service a2dp.target
