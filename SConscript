#$Id: SConscript,v 1.3 2008/02/22 01:19:12 golpa Exp $

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

progEnv.Tool('registerObjects', package = 'map_tools', libraries = [map_toolsLib], binaries = [gtexpcube, gtdispcube, exposure_cube], includes = listFiles(['map_tools/*.h']),
             testApps = [test_map_tools], pfiles = listFiles(['pfiles/*.par']))