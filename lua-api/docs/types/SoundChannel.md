---
layout: default
title: SoundChannel
parent: Lua API
permalink: lua-api/types/SoundChannel
---

# alias SoundChannel
---



```lua
(alias) SoundChannel = ("general"|"battle"|"sfx"|"unitreply"|"voice"|"userinterface"|"ui"|0|1|2...)
    | "general" -- 0
    | "battle" -- Same as `"sfx" | 1`
    | "sfx" -- Same as `"battle" | 1`
    | "unitreply" -- Same as `"voice" | 2`
    | "voice" -- Same as `"unitreply" | 2`
    | "userinterface" -- Same as "ui" | 3`
    | "ui" -- Same as "userinterface" | 3`
    | 0 -- General
    | 1 -- SFX
    | 2 -- Voice
    | 3 -- User interface

```




[<a href="https://github.com/12345swordy/spring/blob/375462d15b00dd1fe92e7b93c1196adfd4926da8/rts/Lua/LuaUnsyncedCtrl.cpp#L698-L711" target="_blank">source</a>]

