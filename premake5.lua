-- premake5.lua
workspace "ray-tracing"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "ray-tracing"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "RayTracing"