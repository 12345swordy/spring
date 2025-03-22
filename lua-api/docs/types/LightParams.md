---
layout: default
title: LightParams
parent: Lua API
permalink: lua-api/types/LightParams
---

# class LightParams





Parameters for lighting

[<a href="https://github.com/12345swordy/spring/blob/375462d15b00dd1fe92e7b93c1196adfd4926da8/rts/Lua/LuaUnsyncedCtrl.cpp#L1567-L1593" target="_blank">source</a>]





## fields


### LightParams.position

```lua
LightParams.position : { pz: number,px: number,py: number }
```




### LightParams.direction

```lua
LightParams.direction : { dy: number,dx: number,dz: number }
```




### LightParams.ambientColor

```lua
LightParams.ambientColor : { red: number,green: number,blue: number }
```




### LightParams.diffuseColor

```lua
LightParams.diffuseColor : { blue: number,green: number,red: number }
```




### LightParams.specularColor

```lua
LightParams.specularColor : { red: number,green: number,blue: number }
```




### LightParams.intensityWeight

```lua
LightParams.intensityWeight : { ambientWeight: number,specularWeight: number,diffuseWeight: number }
```




### LightParams.ambientDecayRate

```lua
LightParams.ambientDecayRate : { ambientGreenDecay: number,ambientBlueDecay: number,ambientRedDecay: number }
```



Per-frame decay of `ambientColor` (spread over TTL frames)


### LightParams.diffuseDecayRate

```lua
LightParams.diffuseDecayRate : { diffuseRedDecay: number,diffuseBlueDecay: number,diffuseGreenDecay: number }
```



Per-frame decay of `diffuseColor` (spread over TTL frames)


### LightParams.specularDecayRate

```lua
LightParams.specularDecayRate : { specularRedDecay: number,specularBlueDecay: number,specularGreenDecay: number }
```



Per-frame decay of `specularColor` (spread over TTL frames)


### LightParams.decayFunctionType

```lua
LightParams.decayFunctionType : { diffuseDecayType: number,specularDecayType: number,ambientDecayType: number }
```



If value is `0.0` then the `*DecayRate` values will be interpreted as linear, otherwise exponential.


### LightParams.radius

```lua
LightParams.radius : number
```




### LightParams.fov

```lua
LightParams.fov : number
```




### LightParams.ttl

```lua
LightParams.ttl : number
```




### LightParams.priority

```lua
LightParams.priority : number
```




### LightParams.ignoreLOS

```lua
LightParams.ignoreLOS : boolean
```




