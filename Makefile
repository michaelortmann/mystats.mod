srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../mystats.o

modules: ../../../mystats.$(MOD_EXT)

../mystats.o:
	$(CC) $(CFLAGS) $(shell mysql_config --cflags) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/mystats.c
	@rm -f ../mystats.o
	mv mystats.o ../

../../../mystats.$(MOD_EXT): ../mystats.o
	$(LD) $(shell mysql_config --libs) -o ../../../mystats.$(MOD_EXT) ../mystats.o
	$(STRIP) ../../../mystats.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/mystats.c > .depend

clean:
	@rm -f .depend config.* *.o *.$(MOD_EXT) *~
distclean: clean

#safety hash
mystats.o: .././mystats.mod/mystats.c .././mystats.mod/../module.h \
 ../../../src/main.h ../../../config.h ../../../eggint.h ../../../lush.h \
 ../../../src/lang.h ../../../src/eggdrop.h ../../../src/compat/in6.h \
 ../../../src/flags.h ../../../src/cmdt.h ../../../src/tclegg.h \
 ../../../src/tclhash.h ../../../src/chan.h ../../../src/users.h \
 ../../../src/compat/compat.h ../../../src/compat/base64.h \
 ../../../src/compat/inet_aton.h ../../../src/compat/snprintf.h \
 ../../../src/compat/gethostbyname2.h \
 ../../../src/compat/explicit_bzero.h ../../../src/compat/strlcpy.h \
 .././mystats.mod/../modvals.h ../../../src/tandem.h \
 .././mystats.mod/../irc.mod/irc.h \
 .././mystats.mod/../channels.mod/channels.h \
 .././mystats.mod/../server.mod/server.h .././mystats.mod/mystats.h \
 .././mystats.mod/language.h .././mystats.mod/settings.c \
 .././mystats.mod/common.c .././mystats.mod/users.c \
 .././mystats.mod/chans.c .././mystats.mod/sensors.c \
 .././mystats.mod/triggers.c .././mystats.mod/public.c
