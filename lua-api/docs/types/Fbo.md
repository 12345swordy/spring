---
layout: default
title: Fbo
parent: Lua API
permalink: lua-api/types/Fbo
---

# class Fbo





User Data FBO

[<a href="https://github.com/12345swordy/spring/blob/375462d15b00dd1fe92e7b93c1196adfd4926da8/rts/Lua/LuaFBOs.cpp#L421-L433" target="_blank">source</a>]





## fields


### Fbo.depth

```lua
Fbo.depth : attachment
```




### Fbo.stencil

```lua
Fbo.stencil : attachment
```




### Fbo.color0

```lua
Fbo.color0 : attachment
```




### Fbo.color1

```lua
Fbo.color1 : attachment
```




### Fbo.color2

```lua
Fbo.color2 : attachment
```




### Fbo.colorn

```lua
Fbo.colorn : attachment
```




### Fbo.color15

```lua
Fbo.color15 : attachment
```




### Fbo.drawbuffers

```lua
Fbo.drawbuffers : table
```



`{ GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT3_EXT, ..}`


### Fbo.readbuffer

```lua
Fbo.readbuffer : table
```



`GL_COLOR_ATTACHMENT0_EXT`


