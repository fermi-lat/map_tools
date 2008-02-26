#$Id: map_toolsLib.py,v 1.2 2008/02/20 17:08:46 golpa Exp $
def generate(env, **kw):
    if not kw.get('depsOnly',0):
        env.Tool('addLibrary', library = ['map_tools'])
    env.Tool('healpixLib')
    env.Tool('astroLib')
    env.Tool('hoopsLib')
    env.Tool('tipLib')
    env.Tool('st_appLib')
    env.Tool('st_streamLib')
    env.Tool('irfLoaderLib')
    env.Tool('addLibrary', library = env['cfitsioLibs'])

def exists(env):
    return 1
