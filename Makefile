# Makefile 27/08/04
# pwatch-0.2 beta

SHELL = /bin/sh
DEBUG_FLAGS = -DPW_DEBUG # leaves std fd's open  
CC = gcc
OBJECTS = pwatch_filter.o tcp_init.o conf.o pwatch.o utils.o getipv4.o logger.o parseconf.o
GCC_USER_FLAGS = -Wall -pedantic -ansi -g3 -O2
APP_NAME = pwatchd
REL != uname -r
MACHINE != uname -m
INSTALL_DIR = /usr/local/pwatch/
CFG_DIR = /etc/pwatch
CFG_FILE = pwatch.conf
DIR_FLAGS = -p -m 0700
VER = 0.4

.MAIN = pwatch

.if ${MACHINE} != "i386" 

pwatch! 
	@echo "Sorry, but pwatch-${VER} has not been built with the '${MACHINE}' architecture in mind."
	@echo "You could (at your own risk!) use 'make anyway' which will attempt to build regardless."

.elif ${REL} != "3.3" 

pwatch!
	@echo "-------------------( DISCLAIMER )--------------------------------------------"
	@echo 
	@echo "pwatch-${VER} has been written and tested using the 3.3 release of OpenBSD."
	@echo 
	@echo "You are using the ${REL} release of OpenBSD."
	@echo 
	@echo "Due to changes in later releases, the 'pf' ioctl's used in this app"
	@echo "may not be compatible with releases after 3.3."
	@echo 
	@echo "I haven't had the opportunity to test a build on -current," 
	@echo "but from what I've seen in the '3.5' man page for 'pf',"
	@echo "the ioctl's for 'pf' used in this app haven't changed" 
	@echo 
	@echo "to continue build use 'make anyway'"
	@echo 
	@echo "-----------------------------------------------------------------------------"

.elif ${REL} == "3.3"

pwatch: ${OBJECTS}  
	${CC} -o ${APP_NAME} ${OBJECTS} ${GCC_USER_FLAGS}

.endif


# build regardless of release or architecture
anyway: ${OBJECTS}
	${CC} -o ${APP_NAME} ${OBJECTS} ${GCC_USER_FLAGS}

debug:  ${OBJECTS}
	${CC} -o ${APP_NAME} ${OBJECTS} ${GCC_USER_FLAGS} ${DEBUG_FLAGS}

install:
	@echo 
	@echo "=> creating directories"
	@mkdir ${DIR_FLAGS} ${INSTALL_DIR}
	@mkdir ${DIR_FLAGS} ${CFG_DIR}
	@echo "=> installing files"
	@cp pwatchd ${INSTALL_DIR}
	@echo "=> setting permissions"
	@chmod 700 ${CFG_FILE}
	@chmod 700 ${INSTALL_DIR}${APP_NAME}
	@cp ${CFG_FILE} ${CFG_DIR}
	@echo 
	@echo "done."


.PHONY: clean

clean: 
	@echo "=> cleaning up"
	/bin/rm -rf ${OBJECTS}
	/bin/rm -rf *~


