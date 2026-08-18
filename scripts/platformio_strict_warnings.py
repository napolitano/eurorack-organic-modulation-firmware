# Purpose: Apply project-only strict warnings to FMD production and test sources.
# SPDX-License-Identifier: GPL-3.0-or-later
Import("env")
STRICT_FLAGS=["-Werror","-Wconversion","-Wsign-conversion","-Wshadow","-Wpedantic"]
PROJECT_DIR=env.subst("$PROJECT_DIR").replace("\\","/").rstrip("/").lower()
ROOTS=(PROJECT_DIR+"/lib/fmd/src/",PROJECT_DIR+"/test/")
def owned(node): return any(node.get_abspath().replace("\\","/").lower().startswith(r) for r in ROOTS)
def middleware(build_env,node):
    if not owned(node): return node
    flags=list(build_env.get("CCFLAGS",[]))
    for f in STRICT_FLAGS:
        if f not in flags: flags.append(f)
    return build_env.Object(node,CCFLAGS=flags)
env.AddBuildMiddleware(middleware)
