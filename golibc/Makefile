TARGET     = libgolibc.a
OBJS       = abs.o atof.o atoi.o errno.o frexp.o ldexp.o \
	memchr.o memcmp.o memmove.o memset.o qsort.o rand.o \
	sprintf.o strcat.o strcmp.o strcpy.o strcspn.o strdup.o \
	strlen.o strncat.o strncmp.o strncpy.o strpbrk.o strrchr.o \
	strspn.o strstr.o strtol.o strtoul.o strtoul0.o vsprintf.o

GCC = i386-elf-gcc
CFLAGS = -I. -fno-common -Wall -O2 -c \
         -finhibit-size-directive -fno-ident \
         -fomit-frame-pointer -fcall-used-ebx
AR = i386-elf-ar
RANLIB = i386-elf-ranlib

ALL : ${OBJS}
	${AR} rv ${TARGET} ${OBJS}
	${RANLIB} ${TARGET}
	make clean_objs

%.o : %.c Makefile
	${GCC} ${CFLAGS} $<

clean_objs:
	rm -f *.o

clean :
	rm -f *.o ${TARGET}
