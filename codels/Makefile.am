#
# Copyright (c) 2015-2017 LAAS/CNRS
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
lib_LTLIBRARIES = libobjectdetection_codels.la

libobjectdetection_codels_la_SOURCES=	objectdetection_c_types.h
libobjectdetection_codels_la_SOURCES+=	objectdetection_codels.cc
libobjectdetection_codels_la_SOURCES+=	objectdetection_FindObjects_codels.cc

libobjectdetection_codels_la_SOURCES+= misc.h
libobjectdetection_codels_la_SOURCES+= misc.cpp

libobjectdetection_codels_la_SOURCES+= find_object_2d.h
libobjectdetection_codels_la_SOURCES+= find_object_2d.cpp

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
