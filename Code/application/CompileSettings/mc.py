"""SCons.Tool.mc

Tool-specific initialization for mc (Microsoft MC compiler).

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.

"""

#
# Copyright (c) 2006 The SCons Foundation
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

__revision__ = "/home/scons/scons/branch.0/baseline/src/engine/SCons/Tool/mc.py 0.96.1.D001 2004/08/23 09:55:29 knight"
# This file is not from the Scons trunk or Subversion repository, it is from the scons developer mailing list.
# In addition, this tool is not in the Scons distribution as of version 1.3.0

import SCons.Defaults
import SCons.Util
import SCons.Tool.msvs

def mc_emitter(target, source, env):
    """Produces a list of outputs from the MC compiler"""
    base, ext = SCons.Util.splitext(str(target[0]))
    incl = base + '.h'
    resource = base + '.rc'

    t = [incl, resource]

    return (t,source)

mc_builder = SCons.Builder.Builder(action=SCons.Action.Action('$MCCOM'),
                                     src_suffix = '.mc',
                                     suffix='.h',
                                     emitter = mc_emitter)

def generate(env):
    """Add Builders and construction variables for mc to an Environment."""
    env['MC']          = 'MC.EXE'
    env['MCFLAGS']     = '-cU'
    env['MCCOM']       = '$MC $MCFLAGS -h ${TARGETS[0].dir} -r ${TARGETS[0].dir} $SOURCE'
    env['BUILDERS']['MessageCatalog'] = mc_builder

def exists(env):
    if SCons.Tool.msvs.is_msvs_installed():
        # there's at least one version of MSVS installed, which comes with mc:
        return 1
    else:
        return env.Detect('mc')

