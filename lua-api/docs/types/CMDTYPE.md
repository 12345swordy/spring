---
layout: default
title: CMDTYPE
parent: Lua API
permalink: lua-api/types/CMDTYPE
---

# enum CMDTYPE
---



Note, the CMDTYPE[] table is bidirectional. That means: CMDTYPE[CMDTYPE.ICON] := "CMDTYPE_ICON"

[<a href="https://github.com/12345swordy/spring/blob/6799fb470d62f3b37ad098f86f7a80f6a6b21747/rts/Lua/LuaConstCMDTYPE.cpp#L17-L36" target="_blank">source</a>]


```cpp
enum CMDTYPE {
    COMBO_BOX = number, 
    CUSTOM = number, 
    ICON = number, 
    ICON_AREA = number, 
    ICON_BUILDING = number, 
    ICON_FRONT = number, 
    ICON_MAP = number, 
    ICON_MODE = number, 
    ICON_UNIT = number, 
    ICON_UNIT_FEATURE_OR_AREA = number, 
    ICON_UNIT_OR_AREA = number, 
    ICON_UNIT_OR_MAP = number, 
    ICON_UNIT_OR_RECTANGLE = number, 
    NEXT = number, 
    NUMBER = number, 
    PREV = number, 
    
}
```
