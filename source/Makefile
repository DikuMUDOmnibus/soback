INCLUDE	= -I../header
WFLAGS	= -Wall
OFLAGS	= -O

CC	= gcc
LFLAGS	= 
DFLAGS	= -DIMO=41 -ggdb -D__BSD__ -D__BSD_DEBUG__ 
CFLAGS	= $(INCLUDE) $(WFLAGS) $(OFLAGS) $(DFLAGS) -pipe 

OBJS	= act.cast.o act.comm.o act.eat.o act.get.o act.group.o \
	act.help.o act.info.o act.look.o act.movement.o \
	act.offensive.o act.other.o act.rent.o act.round_skill.o \
	act.second_skill.o act.skills.o act.social.o act.use.o \
	act.wear.o act.who.o act.wizard.o affect.o allocate.o \
	apartment.o bank.o board.o character.o comm.o constants.o \
	edit.o errhandler.o fight.o find.o hash.o iedit.o \
	insurance.o interpreter.o limits.o locker.o magic.affect.o \
	magic.mobile.o magic.object.o magic.offen.o magic.other.o \
	magic.point.o magic.room.o magic.weapon.o magic.world.o \
	mail.o main.o medit.o mobile.action.o mobile.o modify.o \
	nanny.o object.o page.o pawn.o quest.o queue.o redit.o \
	shop.o sockets.o spec.assign.o spec.items.o spec.mids.o \
	spec.mobiles.o spec.portal.o spec.reboot.o spec.rooms.o \
	spec.school.o spec.unit.o stash.o strings.o sundhaven.o \
	transfer.o update.o utility.o variables.o weather.o \
	world.o zedit.o zone.o


dms : $(OBJS)
	$(CC) -o dms.tmp $(CFLAGS) $(LFLAGS) $(OBJS)

.c.o:
	$(CC) -c $(CFLAGS) $<

clean :
	/bin/rm -f *.o
