SUBDIRS = sbc googleapis/base src
ACLOCAL_AMFLAGS = -I m4
if SYSTEMD
systemdunitdir = @SYSTEMD_UNITDIR@
systemdunit_DATA = a2dp.service a2dp.target
endif
dist_pkgdata_DATA = \
   sounds/Incorrect.ogg                    \
   sounds/Correct.ogg                      \
   sounds/Disabled.ogg                     \
   sounds/Restarting.ogg                   \
   sounds/Sonar.ogg                        \
   sounds/Enter_code_522.ogg               \
   sounds/Unable_to_connect_to_bluetooth_adapter.ogg \
   sounds/Update_completed.ogg             \
   sounds/No_phone_connected.ogg           \
   sounds/Update_complete.ogg              \
   sounds/Please_dont_turh_the_power.ogg   \
   sounds/Update_is_available.ogg          \
   sounds/Preparing_the_update.ogg         \
   sounds/Updating2.ogg                    \
   sounds/Ready_to_pair1.ogg

EXTRA_DIST = a2dp.service a2dp.target
