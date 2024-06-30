#!/usr/bin/env python

import os
from binding_generator import scons_generate_bindings, scons_emit_files

EnsureSConsVersion(4, 0)

try:
    Import("env")
except:
    # Default tools with no platform defaults to gnu toolchain.
    # We apply platform specific toolchains via our custom tools.
    env = Environment(tools=["default"], PLATFORM="")

env.PrependENVPath("PATH", os.getenv("PATH"))

scons_cache_path = os.environ.get("SCONS_CACHE")
if scons_cache_path is not None:
    CacheDir(scons_cache_path)
    Decider("MD5")

sources = [ "dataproto.cpp" ]
library = env.Library(target="dataproto", source=sources)
Return('env')
