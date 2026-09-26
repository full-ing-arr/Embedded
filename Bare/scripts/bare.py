Import("env")

env["LINKFLAGS"].remove("-u")
env["LINKFLAGS"].remove("call_user_start_cpu0")
env["LINKFLAGS"].remove("-Wl,--undefined=uxTopUsedPriority")
