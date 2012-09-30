bin_PROGRAMS = ksr_chat_server ksr_chat_client

AM_CFLAGS = \
    $(GLIB_CFLAGS)        \
	$(GIO_CFLAGS)

ksr_chat_server_SOURCES =       \
    $(dbus_built_sources) 		\
    main-server.c

ksr_chat_client_SOURCES =       \
    $(dbus_built_sources) 		\
    main-client.c

ksr_chat_client_LDADD =         \
	$(GLIB_LIBS) 	      		\
	$(GIO_LIBS)
	
ksr_chat_server_LDADD =         \
	$(GLIB_LIBS) 	      		\
	$(GIO_LIBS)
	
dbus_built_sources = \
      chat-generated.h \
      chat-generated.c

$(dbus_built_sources) : $(top_srcdir)/data/org.ksr.chat.xml
	gdbus-codegen							                \
		--interface-prefix org.ksr                          \
		--c-namespace ksr    						        \
		--generate-c-code chat-generated                    \
		$(top_srcdir)/data/org.ksr.chat.xml	

clean-local :
	rm -f $(dbus_built_sources) \
	rm -f *~
	