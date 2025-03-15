---
layout: default
title: SetWMCaption
parent: Lua API
permalink: lua-api/globals/SetWMCaption
---

# global SetWMCaption

---

```lua
function SetWMCaption(
  title: string,
  titleShort: string?
) ->  nil
```
@param `titleShort` - (Default: title)







Sets the window title for the process (default: "Spring <version>").

The shortTitle is displayed in the OS task-bar (default: "Spring <version>").

NOTE: shortTitle is only ever possibly used under X11 (Linux & OS X), but not with QT (KDE) and never under Windows.

[<a href="https://github.com/12345swordy/spring/blob/6799fb470d62f3b37ad098f86f7a80f6a6b21747/rts/Lua/LuaUnsyncedCtrl.cpp#L5231-L5242" target="_blank">source</a>]

