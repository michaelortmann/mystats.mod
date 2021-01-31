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
../mystats.o: .././mystats.mod/mystats.c ../../../src/mod/module.h \
 ../../../src/main.h ../../../src/lang.h ../../../src/eggdrop.h \
 ../../../src/flags.h ../../../src/proto.h ../../../lush.h \
 ../../../src/misc_file.h ../../../src/cmdt.h ../../../src/tclegg.h \
 ../../../src/tclhash.h ../../../src/chan.h ../../../src/users.h \
 ../../../src/compat/compat.h ../../../src/compat/inet_aton.h \
 ../../../src/compat/snprintf.h ../../../src/compat/memset.h \
 ../../../src/compat/memcpy.h ../../../src/compat/strcasecmp.h \
 ../../../src/mod/modvals.h ../../../src/tandem.h
