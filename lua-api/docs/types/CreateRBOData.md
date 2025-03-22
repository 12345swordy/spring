---
layout: default
title: CreateRBOData
parent: Lua API
permalink: lua-api/types/CreateRBOData
---

# class CreateRBOData





[<a href="https://github.com/12345swordy/spring/blob/375462d15b00dd1fe92e7b93c1196adfd4926da8/rts/Lua/LuaRBOs.cpp#L150-L155" target="_blank">source</a>]





## fields


### CreateRBOData.target

```lua
CreateRBOData.target : GL {
    cmd: integer,
    POINTS: integer,
    LINES: integer,
    LINE_LOOP: integer,
    LINE_STRIP: integer,
    TRIANGLES: integer,
    TRIANGLE_STRIP: integer,
    TRIANGLE_FAN: integer,
    QUADS: integer,
    QUAD_STRIP: integer,
    POLYGON: integer,
    LINE_STRIP_ADJACENCY: integer,
    ...
}
```




### CreateRBOData.format

```lua
CreateRBOData.format : GL {
    cmd: integer,
    POINTS: integer,
    LINES: integer,
    LINE_LOOP: integer,
    LINE_STRIP: integer,
    TRIANGLES: integer,
    TRIANGLE_STRIP: integer,
    TRIANGLE_FAN: integer,
    QUADS: integer,
    QUAD_STRIP: integer,
    POLYGON: integer,
    LINE_STRIP_ADJACENCY: integer,
    ...
}
```




### CreateRBOData.samples

```lua
CreateRBOData.samples : number?
```



any number here will result in creation of multisampled RBO


