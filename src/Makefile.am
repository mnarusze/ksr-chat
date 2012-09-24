bin_PROGRAMS = ksr_chat

AM_CFLAGS = \
    $(GLIB_CFLAGS)        \
	$(GIO_CFLAGS)

ksr_chat_SOURCES =        \
    $(dbus_built_sources) \
    main.c

ksr_chat_LDADD =          \
	$(GLIB_LIBS) 	      \
	$(GIO_LIBS)
	
dbus_built_sources = \
      chat-generated.h \
      chat-generated.c

$(dbus_built_sources) : $(top_srcdir)/data/org.ksr.chat.xml
	gdbus-codegen							                \
		--interface-prefix org.ksr.chat.                    \
		--c-namespace chat							        \
		--generate-c-code chat-generated                    \
		$(top_srcdir)/data/org.ksr.chat.xml	

clean-local :
	rm -f $(dbus_built_sources) \
	rm -f *~
	