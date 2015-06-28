bin_PROGRAMS = bt_a2dp mkupdate
noinst_PROGRAMS = serial_screen

lib_LIBRARIES = liba2dp.a
liba2dp_a_SOURCES = \
    AacMediaEndpoint.cpp        \
    ../include/AacMediaEndpoint.h      \
    InterfaceImplementation.cpp \
    ../include/InterfaceImplementation.h   \
    MpegMediaEndpoint.cpp       \
    ../include/MpegMediaEndpoint.h     \
    AudioSource.cpp             \
    ../include/AudioSource.h           \
    AudioTargetControl.cpp      \
    ../include/AudioTargetControl.h    \
    ObjectBase.cpp              \
    ../include/ObjectBase.h            \
    BluezAdapter.cpp            \
    ../include/BluezAdapter.h          \
    BluezDevice.cpp            \
    ../include/BluezDevice.h          \
    BluezAgent.cpp              \
    ../include/BluezAgent.h            \
    DelayedProcessing.cpp              \
    ../include/DelayedProcessing.h            \
    FirmwareContainer.cpp              \
    ../include/FirmwareContainer.h            \
    FirmwareUpdater.cpp              \
    ../include/FirmwareUpdater.h            \
    MediaEndpoint.cpp	        \
    ../include/MediaEndpoint.h         \
    PlaybackThread.cpp          \
    ../include/PlaybackThread.h        \
    AudioMixer.cpp          \
    ../include/AudioMixer.h        \
    SoundFragment.cpp          \
    ../include/SoundFragment.h        \
    SoundQueue.cpp          \
    ../include/SoundQueue.h        \
    SoundManager.cpp          \
    ../include/SoundManager.h        \
    BluezManager.cpp            \
    ../include/BluezManager.h          \
    MediaTransport.cpp	        \
    ../include/MediaTransport.h        \
    RemoteMethod.cpp            \
    ../include/RemoteMethod.h          \
    BluezMedia.cpp	        \
    ../include/BluezMedia.h  	       \
    DictionaryHelper.cpp        \
    ../include/DictionaryHelper.h      \
    MediaTransportProperties.cpp \
    ../include/MediaTransportProperties.h  \
    Serial.cpp             \
    ../include/Serial.h           \
    TextScreen.cpp             \
    ../include/TextScreen.h           \
    SbcDecodeThread.cpp         \
    ../include/SbcDecodeThread.h       \
    BluezNames.cpp	        \
    ../include/BluezNames.h  	       \
    MessageArgumentIterator.cpp \
    ../include/MessageArgumentIterator.h   \
    SbcMediaEndpoint.cpp        \
    ../include/SbcMediaEndpoint.h      \
    Connection.cpp	        \
    ../include/Connection.h  	       \
    Message.cpp		        \
    ../include/Message.h  	       \
    CommandParser.cpp           \
    ../include/CommandParser.h         \
    time_util.cpp               \
    ../include/time_util.h             \
    ../include/a2dp-codecs.h	       \
    ../include/StringWithHash.h        \
    ../include/ObjectPath.h            \
    ../include/util.h

liba2dp_a_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/sbc \
    -I$(top_srcdir) \
    -DDATADIR='"$(datadir)/@PACKAGE@"' \
    $(dbus_CFLAGS) \
    $(libglog_CFLAGS) 

liba2dp_a_CXXFLAGS = --std=c++11 

bt_a2dp_SOURCES = \
    main.cpp

bt_a2dp_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/sbc \
    -I$(top_srcdir) \
    -DDATADIR='"$(datadir)/@PACKAGE@"' \
    $(dbus_CFLAGS) \
    $(libglog_CFLAGS) 

bt_a2dp_CXXFLAGS = --std=c++11 

bt_a2dp_LDADD =  $(libglog_LIBS) $(dbus_LIBS) \
    liba2dp.a \
    $(top_builddir)/sbc/libsbc.la \
    $(top_builddir)/googleapis/base/libgoogleapis.la \
    -lgflags -lasound -lpthread -lgcrypt -llzo2 -lvorbisfile

serial_screen_SOURCES = \
    serial_main.cpp

serial_screen_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir) \
    $(dbus_CFLAGS) \
    $(libglog_CFLAGS) 

serial_screen_CXXFLAGS = --std=c++11 

serial_screen_LDADD =  $(libglog_LIBS) $(dbus_LIBS) \
    liba2dp.a \
    $(top_builddir)/googleapis/base/libgoogleapis.la \
    -lgflags -lpthread

mkupdate_SOURCES = \
    mkupdate.cpp   \
    FirmwareContainer.cpp \
    ../include/FirmwareContainer.h

mkupdate_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir) \
    $(libglog_CFLAGS) 

mkupdate_CXXFLAGS = --std=c++11 

mkupdate_LDADD =  $(libglog_LIBS) $(dbus_LIBS) \
    $(top_builddir)/googleapis/base/libgoogleapis.la \
    -lgflags -lgcrypt -llzo2

     