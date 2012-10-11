"""deb
"""

#
# Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 The SCons Foundation
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

# This code was written by Trevor Clarke <tclarke@ball.com> based on
# existing SCons code. This file has been submitted back for inclusion
# in SCons but is not yet in a distribution

import SCons.Builder
import SCons.Node.FS
import os

from SCons.Tool.packaging import stripinstallbuilder, putintopackageroot

def package(env, target, source, PACKAGEROOT, NAME, VERSION, DESCRIPTION,
            SUMMARY, X_DPKG_PRIORITY, X_DPKG_SECTION, SOURCE_URL,
            X_DPKG_MAINTAINER, **kw):
    """ this function prepares the packageroot directory for packaging with the
    dpkg builder.
    """
    env.Tool('dpkg')

    # setup the Dpkg builder
    bld = env['BUILDERS']['Dpkg']
    target, source = stripinstallbuilder(target, source, env)
    target, source = putintopackageroot(target, source, env, PACKAGEROOT)

    # This should be overridable from the construction environment,
    # which it is by using ARCHITECTURE=.
    # Guessing based on what os.uname() returns at least allows it
    # to work for both i386 and x86_64 Linux systems.
    archmap = {
        'i686'  : 'i386',
        'i586'  : 'i386',
        'i486'  : 'i386',
    }

    buildarchitecture = os.uname()[4]
    buildarchitecture = archmap.get(buildarchitecture, buildarchitecture)

    if kw.has_key('ARCHITECTURE'):
        buildarchitecture = kw['ARCHITECTURE']

    # setup the kw to contain the mandatory arguments to this fucntion.
    # do this before calling any builder or setup function
    loc=locals()
    del loc['kw']
    kw.update(loc)
    del kw['source'], kw['target'], kw['env']

    # generate the specfile
    specfile = gen_dpkg_dir(PACKAGEROOT, source, env, kw)

    # override the default target.
    if str(target[0])=="%s-%s"%(NAME, VERSION):
        target=[ "%s_%s_%s.deb"%(NAME, VERSION, buildarchitecture) ]

    # now apply the Dpkg builder
    return apply(bld, [env, target, specfile], kw)

def gen_dpkg_dir(proot, source, env, kw):
    # make sure the packageroot is a Dir object.
    if SCons.Util.is_String(proot): proot=env.Dir(proot)

    #  create the specfile builder
    s_bld=SCons.Builder.Builder(
        action  = build_specfiles,
        )

    # create the specfile targets
    spec_target=[]
    control=proot.Dir('DEBIAN')
    spec_target.append(control.File('control'))

    # apply the builder to the specfile targets
    apply(s_bld, [env, spec_target, source], kw)

    # the packageroot directory does now contain the specfiles.
    return proot

def build_specfiles(source, target, env):
    """ filter the targets for the needed files and use the variables in env
    to create the specfile.
    """
    #
    # At first we care for the DEBIAN/control file, which is the main file for dpkg.
    #
    # For this we need to open multiple files in random order, so we store into
    # a dict so they can be easily accessed.
    #
    #
    opened_files={}
    def open_file(needle, haystack):
        try:
            return opened_files[needle]
        except KeyError:
            file=filter(lambda x: x.get_path().rfind(needle)!=-1, haystack)[0]
            opened_files[needle]=open(file.abspath, 'w')
            return opened_files[needle]

    control_file=open_file('control', target)

    if not env.has_key('X_DPKG_DESCRIPTION'):
        env['X_DPKG_DESCRIPTION']="%s\n %s"%(env['SUMMARY'],
                                            env['DESCRIPTION'].replace('\n', '\n '))


    content = """
Package: $NAME
Version: $VERSION
Priority: $X_DPKG_PRIORITY
Section: $X_DPKG_SECTION
Source: $SOURCE_URL
Homepage: $X_DPKG_HOMEPAGE
Architecture: $ARCHITECTURE
Maintainer: $X_DPKG_MAINTAINER
Depends: $X_DPKG_DEPENDS
Description: $X_DPKG_DESCRIPTION
"""

    control_file.write(env.subst(content) + '\n')

    #
    # close all opened files
    for f in opened_files.values():
        f.close()

    # call a user specified function
    if env.has_key('CHANGE_SPECFILE'):
        content += env['CHANGE_SPECFILE'](target)

    return 0
