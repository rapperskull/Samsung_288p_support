-include ../toolchain.conf
-include ../install.conf

LIB_NAME=288pSupport
C_SUPPORT=1

OUTDIR?=${PWD}/out-${ARCH}

TARGETS=lib${LIB_NAME}.so
TARGETS:=$(TARGETS:%=${OUTDIR}/%)

CFLAGS += -fPIC -O2 -std=gnu99 -I../include
CFLAGS += -ldl -DLIB_NAME='"${LIB_NAME}"'

C_FILES=288p_support.c hook.c

ifeq (${C_SUPPORT}, 1)
C_FILES+= C_support.c
CFLAGS+= -DC_SUPPORT=1
endif

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

all: ${OUTDIR} ${TARGETS}

#${OUTDIR}/lib${LIB_NAME}.so: rectitle.c hook.c C_support.c $(wildcard *.h) $(wildcard ../include/*.h)
${OUTDIR}/lib${LIB_NAME}.so: ${C_FILES} $(wildcard *.h) $(wildcard ../include/*.h)
	$(CC) $(filter %.c %.cpp,$^) ${CFLAGS} -shared -Wl,-soname,$@ -o $@

${OUTDIR}:
	@mkdir -p ${OUTDIR}

clean:
	rm -f ${TARGETS}

ifeq (${TARGET_IP}, )
endif

install: ${TARGETS}
	ping -c1 -W1 -w1 ${TARGET_IP} >/dev/null && \
        lftp -v -c "open ${TARGET_IP};cd ${TARGET_DEST_DIR};mput $^;"

.PHONY: clean
.PHONY: install
