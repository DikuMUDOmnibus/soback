#
#  Makefile
#
#
#

OBJS = main.o \
	act.comm.o act.info.o act.movement.o act.get.o act.eat.o \
	act.wear.o act.offensive.o act.other.o act.social.o \
	act.wizard.o act.cast.o act.group.o act.who.o act.look.o \
	act.use.o act.help.o act.rent.o act.round_skill.o \
	act.second_skill.o act.skills.o \
	magic.mobile.o magic.other.o magic.offen.o magic.weapon.o \
	magic.point.o magic.object.o magic.room.o magic.world.o \
	magic.affect.o \
	shop.o \
	interpreter.o nanny.o \
	limits.o find.o affect.o locker.o transfer.o \
	modify.o weather.o \
	fight.o \
	mail.o board.o pawn.o bank.o \
	insurance.o \
	constants.o variables.o \
	quest.o quest2.o \
	errhandler.o utility.o strings.o \
	stash.o character.o \
	comm.o sockets.o \
	spec.rooms.o spec.items.o spec.mids.o spec.assign.o \
	spec.mobiles.o spec.school.o spec.unit.o spec.reboot.o \
	mobile.action.o \
	update.o \
	hash.o queue.o \
	allocate.o mobile.o world.o object.o zone.o \
	edit.o iedit.o medit.o redit.o zedit.o \
	sundhaven.o

INCLUDE = -I../header
WFLAGS  = -Wall
OFLAGS  = -O

.if defined( sun )
CC=gcc.sun
LFLAGS  = -lc -lnsl -lsocket -lintl
DFLAGS  = -DIMO=41 -ggdb -D__STDC__=0
CFLAGS  = $(INCLUDE) $(WFLAGS) $(OFLAGS) $(DFLAGS) -pipe 
.elseif
CC=gcc
LFLAGS  = -lcrypt
DFLAGS  = -DIMO=41 -ggdb -D__BSD__ -D__BSD_DEBUG__ -D__USE_TERMIOS__
CFLAGS  = $(INCLUDE) $(WFLAGS) $(OFLAGS) $(DFLAGS) -pipe 
.endif

.SRCDIR = $(.CURDIR)/source

CFILES=$(OBJS:S/.o$/.c/g:S/^/$(.SRCDIR)\//g)

default : dms

all : clean depend dms

dep : depend
depend :
.if defined( sun )
	mkdep -nostdinc -I/usr/local/sparc-sun-solaris2.4/include $(CFLAGS) $(CFILES) 
.elseif
	mkdep $(CFLAGS) $(CFILES) 
.endif

.if !defined( sun )
dms : $(OBJS)
	$(CC) -o dms.tmp $(CFLAGS) $(LFLAGS) $(OBJS)
	/bin/mv dms.tmp $(.CURDIR)/dms.bsd
.else
dms : $(OBJS)
	$(CC) -o dms.tmp $(CFLAGS) $(LFLAGS) $(OBJS) \
 	/usr/local/sparc-sun-solaris2.4/lib/libsocket.a \
 	/usr/local/sparc-sun-solaris2.4/lib/libm.a \
  	/usr/local/sparc-sun-solaris2.4/lib/libnsl.a \
 	/usr/local/sparc-sun-solaris2.4/lib/libintl.a
	/bin/mv dms.tmp $(.CURDIR)/dms.sun
.endif

clean :
	/bin/rm -f $(.OBJDIR)/*.o
