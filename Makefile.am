# -*- Mode : makefile; indent-tabs-mode: t -*-
#
#    AUTHOR: Christophe Prud'homme
#       ORG: EPFL
#    E-MAIL: christophe.prudhomme@epfl.ch
#
# DESCRIPTION:
# ============
#  Distributed under the GPL(GNU Public License):
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
# DESCRIP-END.
#

AUTOMAKE_OPTIONS = foreign 1.7 dist-bzip2

EXTRA_DIST = configure.in.in configure.in.bot  configure.in.mid subdirs configure.files\
             Mainpage.dox BUGS Makefile.headers

MAINTAINERCLEANFILES = configure.files subdirs

DOX_FILES   = Doxyfile.am

include $(top_srcdir)/Doxyfile.am
include $(top_srcdir)/admin/deps.am

include $(top_srcdir)/Makefile.headers

dist-hook: changelog
# make sure that we have an up-to-date ChangeLog
	if test -w . -a $(top_srcdir) != $(top_builddir) -a ! -f ChangeLog; then\
	  cp $(srcdir)/ChangeLog .;\
	fi

changelog:
	if test -x /usr/bin/cvs2cl -a -d $(srcdir)/CVS; then\
	 (cd $(srcdir) && cvs2cl -b -t  -S --utc -P);\
    else \
     if test -w $(srcdir)/ChangeLog; then\
       touch $(srcdir)/ChangeLog;\
     fi;\
    fi

apidox-all:
	@echo "generate API documentation with doxygen"
	make apidox

all-local:
	@echo ""
	@echo "************************************************************"
	@echo "* LifeV compilation has ended."
	@echo "*"
	@echo "* To generate LifeV API documentation type make apidox"
	@echo "* To install LifeV API documentation type make install-apidox"
	@echo "*"
	@echo "* Enjoy!"
	@echo "*  The LifeV Developing Team"
	@echo "************************************************************"
	@echo ""

upload-api:
	-cd doc/ && rsync -avz api/ `whoami`@cmcsforge.epfl.ch:/var/lib/gforge/chroot/home/groups/lifev/htdocs/api-$(LIFEV_MAJOR_VERSION).$(LIFEV_MINOR_VERSION).$(LIFEV_MICRO_VERSION)

MOSTLYCLEANFILES	= ChangeLog life.pc.in life-uninstalled.sh life.pc life-uninstalled.pc

$(top_srcdir)/acinclude.m4:
	@echo 'm4_include([admin/libtool.m4])' > acinclude.m4
	@echo 'm4_include([admin/ltversion.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ltoptions.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ltsugar.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ac_check_y12m.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ac_check_hdf5.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ac_check_trilinos.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ac_check_boost.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ac_check_itk.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ac_check_lapack.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ac_check_pvm.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ac_check_mpi.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ac_check_parmetis.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ac_check_petsc.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ac_check_umfpack.m4])' >> acinclude.m4
	@echo 'm4_include([admin/ac_find_file.m4])' >> acinclude.m4
	@echo 'm4_include([admin/lifev_check_compilers.m4])'>>acinclude.m4
	@echo 'm4_include([admin/lifev_default_paths.m4])'>>acinclude.m4
	@echo 'm4_include([admin/ax_create_pkgconfig_info.m4])'>>acinclude.m4
	@echo 'm4_include([admin/lifev_create_subdirlist.m4])'>>acinclude.m4


$(top_srcdir)/Makefile.headers:
	cd $(top_srcdir) && $(SHELL) tools/update-headers

report:
	@if test -x tools/inspect/inspect; then\
        thedate=`date +%Y%m%d`;\
		thepwd=`pwd`;\
		cd $(srcdir) && $$thepwd/tools/inspect/inspect > $$thepwd/report-$$thedate.html;\
    else\
       echo "tools/inspect/inspect has not been built";\
       echo "consider typing: cd tools/inspect && make";\
	fi
#SUBDIRS = admin doc life testsuite tools
SUBDIRS = admin tools $(TOPSUBDIRS)
