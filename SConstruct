#!/usr/bin/env python

import os
from SCons.Script import *

EnsureSConsVersion(4, 0)

try:
    Import("env", "customs")
except:
    # Default tools with no platform defaults to gnu toolchain.
    # We apply platform specific toolchains via our custom tools.
    env = Environment(tools=["default"], PLATFORM="")

env.PrependENVPath("PATH", os.getenv("PATH"))

scons_cache_path = os.environ.get("SCONS_CACHE")
if scons_cache_path is not None:
    CacheDir(scons_cache_path)
    Decider("MD5")

sources = [ "#dataproto-cpp/src/dataproto.cpp" ]
env.Append(CPPPATH=[ "#dataproto-cpp/include/dataproto_cpp" ])

buildtype = ARGUMENTS.get("buildtype", "shared")

if buildtype == "library":
    target = "dataproto"
    target_obj = env.Library(target=target, source=sources)
elif buildtype == "shared":
    target = "dataproto"
    target_obj = env.SharedLibrary(target=target, source=sources)
elif buildtype == "objects":
    target = "dataproto"
    target_obj = env.Object(target=target, source=sources)
else:
    raise ValueError(f"Unsupported buildtype '{buildtype}'")

Return("env", "target_obj")
