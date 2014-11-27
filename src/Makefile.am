bin_PROGRAMS = bt_a2dp
bt_a2dp_SOURCES =               \
    AacMediaEndpoint.cpp        \
    InterfaceImplementation.cpp \
    MpegMediaEndpoint.cpp       \
    AudioSource.cpp             \
    main.cpp		        \
    ObjectBase.cpp              \
    BluezAdapter.cpp            \
    MediaEndpoint.cpp	        \
    PlaybackThread.cpp          \
    BluezManager.cpp            \
    MediaTransport.cpp	        \
    RemoteMethod.cpp            \
    BluezMedia.cpp	        \
    MediaTransportProperties.cpp \
    SbcDecodeThread.cpp         \
    BluezNames.cpp	        \
    MessageArgumentIterator.cpp \
    SbcMediaEndpoint.cpp        \
    Connection.cpp	        \
    Message.cpp		        \
    time_util.cpp

bt_a2dp_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/sbc \
    -I$(top_srcdir) \
    $(dbus_CFLAGS) \
    $(libglog_CFLAGS) 

bt_a2dp_CXXFLAGS = --std=c++11 

bt_a2dp_LDADD =  $(libglog_LIBS) $(dbus_LIBS) \
    $(top_builddir)/sbc/libsbc.la \
    -lgflags -lasound -lpthread
