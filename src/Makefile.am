bin_PROGRAMS = bt_a2dp
bt_a2dp_SOURCES =               \
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
    main.cpp		        \
    ObjectBase.cpp              \
    ../include/ObjectBase.h            \
    BluezAdapter.cpp            \
    ../include/BluezAdapter.h          \
    BluezAgent.cpp              \
    ../include/BluezAgent.h            \
    sha256.cpp              \
    ../include/sha256.h            \
    FirmwareUpdater.cpp              \
    ../include/FirmwareUpdater.h            \
    MediaEndpoint.cpp	        \
    ../include/MediaEndpoint.h         \
    PlaybackThread.cpp          \
    ../include/PlaybackThread.h        \
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


bt_a2dp_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/sbc \
    -I$(top_srcdir) \
    $(dbus_CFLAGS) \
    $(libglog_CFLAGS) 

bt_a2dp_CXXFLAGS = --std=c++11 

bt_a2dp_LDADD =  $(libglog_LIBS) $(dbus_LIBS) \
    $(top_builddir)/sbc/libsbc.la \
    $(top_builddir)/googleapis/base/libgoogleapis.la \
    -lgflags -lasound -lpthread
