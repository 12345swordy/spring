---
layout: default
title: LogLevel
parent: Lua API
permalink: lua-api/types/LogLevel
---

# alias LogLevel
---



```lua
(alias) LogLevel = (integer|"debug"|"info"|"notice"|"warning"|"deprecated"|"error"|"fatal")
    | integer
    | "debug" -- LOG.DEBUG
    | "info" -- LOG.INFO
    | "notice" -- LOG.NOTICE (engine default)
    | "warning" -- LOG.WARNING
    | "deprecated" -- LOG.DEPRECATED
    | "error" -- LOG.ERROR
    | "fatal" -- LOG.FATAL

```




[<a href="https://github.com/12345swordy/spring/blob/6799fb470d62f3b37ad098f86f7a80f6a6b21747/rts/Lua/LuaUnsyncedCtrl.cpp#L488-L498" target="_blank">source</a>]

