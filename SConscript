# -*- python -*-
# $Id: SConscript,v 1.18 2009/07/16 00:30:39 glastrm Exp $
# Authors: T. Burnett <tburnett@u.washington.edu>
# Version: map_tools-07-04-02
Import('baseEnv')
Import('listFiles')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

map_toolsLib = libEnv.StaticLibrary('map_tools', listFiles(['src/*.cxx']))

progEnv.Tool('map_toolsLib')
progEnv.Tool('dataSubselectorLib')
gtexpcube = progEnv.Program('gtexpcube', listFiles(['src/exposure_map/*.cxx']))
gtdispcube = progEnv.Program('gtdispcube', listFiles(['src/cube_display/*.cxx']))
exposure_cube = progEnv.Program('exposure_cube', listFiles(['src/exposure_cube/*.cxx']))
test_map_tools = progEnv.Program('test_map_tools', listFiles(['src/test/*.cxx']))

progEnv.Tool('registerTargets', package = 'map_tools',
             staticLibraryCxts = [[map_toolsLib, libEnv]],
             binaryCxts = [[gtexpcube,progEnv], [gtdispcube,progEnv], [exposure_cube,progEnv]],
             includes = listFiles(['map_tools/*.h']),
             testAppCxts = [[test_map_tools,progEnv]], pfiles = listFiles(['pfiles/*.par']))
