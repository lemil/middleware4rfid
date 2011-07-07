ACLOCAL_AMFLAGS=-I m4

SUBDIRS=test

lib_LTLIBRARIES=libtdt.la

libtdt_la_SOURCES= \
	soapC.cpp \
	table.cpp \
	tdt_nsmap.cpp \
	tdt.cpp

libtdt_la_LDFLAGS= \
	-lpthread \
	-lgmpxx

libtdt_la_LIBADD= \
	$(objfiles)
	

tdt_nsmap.cpp: tdt.nsmap
	cp $< $@

soapC.cpp tdt.nsmap: EpcTagDataTranslation.h
	soapcpp2 $^ -I/usr/local/share/gsoap/import

EpcTagDataTranslation.h: CompactionMethodList.xsd   InputFormatList.xsd  ModeList.xsd EpcTagDataTranslation.xsd  LevelTypeList.xsd    PadDirectionList.xsd
	wsdl2h EpcTagDataTranslation.xsd

files= \
	SGLN-195.xml \
	GSRN-96.xml \
	SGLN-96.xml \
	SGTIN-198.xml \
	USDOD-64.xml \
	GIAI-96.xml \
	GRAI-64.xml \
	SGLN-64.xml \
	GID-96.xml \
	SGTIN-96.xml \
	GIAI-64.xml \
	SSCC-64.xml \
	SGTIN-64.xml \
	GIAI-202.xml \
	SSCC-96.xml \
	USDOD-96.xml \
	GRAI-96.xml \
	GDTI-96.xml \
	GRAI-170.xml \
	GDTI-113.xml

objfiles=$(files:.xml=.o)
names=$(files:.xml=)

declares=$(foreach name,$(names),"extern char _binary_$(name)_xml_start[]; extern char _binary_$(name)_xml_end[]; ")

inits=$(foreach name,$(names), "{\"$(name)\", _binary_`echo $(name)|sed 's/\./_/;s/-/_/g'`_xml_start, _binary_`echo $(name)|sed 's/\./_/g;s/-/_/g'`_xml_end}, ")

table.cpp: table.h
	echo '#include "table.h"' > $@
	echo $(declares) |sed 's/-/_/g;s/\.xml/_xml/g' >> $@
	echo 'XMLFile xmlfiles[] = {' >>$@
	echo $(inits) >> $@
	echo '{0, 0, 0}' >> $@
	echo '};' >>$@

table.h:
	echo '#ifndef __TABLE_H__' > $@
	echo '#define __TABLE_H__' >> $@
	echo 'typedef struct {' >> $@
	echo '	const char *name;' >> $@
	echo '	char *start;' >> $@
	echo '	char *end;' >> $@
	echo '} XMLFile;' >> $@
	echo 'extern XMLFile xmlfiles[];' >> $@
	echo '#endif //__TABLE_H__' >> $@
	
clean-local:
	rm -f *.nsmap table.h table.c tdt_nsmap.c

OBJCOPY             ?= objcopy
OBJDUMP             ?= objdump
objcopyflags         = case "$(host_cpu)" in                                  \
                         i[0-9]86)echo '-O elf32-i386 -B i386';;              \
                         x86_64)  echo '-O elf64-x86-64 -B i386:x86-64';;     \
                         *)       trap 'rm -f /tmp/probe$$$$.o' EXIT;         \
                                  $(CC) $(AM_CFLAGS) $(CFLAGS) -c -xc         \
                                        -o /tmp/probe$$$$.o /dev/null &&      \
                                  $(OBJDUMP) -f /tmp/probe$$$$.o |            \
                                  sed -e 's/.*file format */-O /;t;           \
                                       s/architecture: *\([^,]*\).*/-B \1/;t; \
                                       d';;                                   \
                       esac
.xml.o:
	$(OBJCOPY) -I binary `$(objcopyflags)` \
	  $< $@


