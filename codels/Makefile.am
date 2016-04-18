lib_LTLIBRARIES = libobjectdetection_codels.la

libobjectdetection_codels_la_SOURCES=	objectdetection_c_types.h
libobjectdetection_codels_la_SOURCES+=	objectdetection_codels.cc
libobjectdetection_codels_la_SOURCES+=	objectdetection_FindObjects_codels.cc

libobjectdetection_codels_la_SOURCES+= misc.h
libobjectdetection_codels_la_SOURCES+= misc.cpp

libobjectdetection_codels_la_CPPFLAGS=	$(requires_CFLAGS)
libobjectdetection_codels_la_LDFLAGS=	$(requires_LIBS)
libobjectdetection_codels_la_LDFLAGS+=	-release $(PACKAGE_VERSION)


# idl  mappings
BUILT_SOURCES=	objectdetection_c_types.h
CLEANFILES=	${BUILT_SOURCES}
objectdetection_c_types.h: ${top_srcdir}/objectdetection.gen
	${GENOM3}  mappings \
	  -MD -MF .deps/$@.d -MT $@ --signature -l c $< >$@

-include .deps/objectdetection_c_types.h.d
