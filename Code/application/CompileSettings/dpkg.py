"""dpkg

Tool-specific initialization for dpkg.

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.

The dpkg tool calls the dpkg-deb. Its only argument should be the 
packages fake_root.
"""

#
# __COPYRIGHT__
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

__revision__ = "__FILE__ __REVISION__ __DATE__ __DEVELOPER__"

# This code was written by Trevor Clarke <tclarke@ball.com> based on
# existing SCons code. This file has been submitted back for inclusion
# in SCons but is not yet in a distribution

import os
import string

import SCons.Builder

def generate(env):
    """Add Builders and construction variables for dpkg to an Environment."""
    try:
        bld = env['BUILDERS']['Dpkg']
    except KeyError:
        bld = SCons.Builder.Builder( action  = '$DPKGCOM',
                                     suffix  = '$DPKGSUFFIX',
                                     source_scanner = None,
                                     target_scanner = None)
        env['BUILDERS']['Dpkg'] = bld

    env['DPKG']       = 'fakeroot dpkg-deb'
    env['DPKGCOM']    = '$DPKG $DPKGFLAGS --build ${SOURCE} ${PACKAGEROOT}/${TARGET}'
    env['IPKGSUFFIX'] = '.deb'

def exists(env):
    return env.Detect('dpkg')

# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
