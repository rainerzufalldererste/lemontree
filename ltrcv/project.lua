ProjectName = "ltrcv"
project(ProjectName)

  --Settings
  kind "ConsoleApp"
  language "C#"
  framework "4.7.2"

  objdir "intermediate/obj"

  files { "src/**.cs" }
  files { "project.lua" }

  configuration {}
  
  configuration { }
  
  targetname(ProjectName)
  targetdir "../builds/bin/server"
  debugdir "../builds/bin/server"
  
filter {}
configuration {}

links { "System" }
links { "System.Data" }
links { "3rdParty/monocypher/monocypher.dll" }
links { "3rdParty/monocypher/System.Buffers.dll" }
links { "3rdParty/monocypher/System.Memory.dll" }
links { "3rdParty/monocypher/System.Numerics.Vectors.dll" }
links { "3rdParty/monocypher/System.Runtime.CompilerServices.Unsafe.dll" }

filter { "configurations:Debug*" }
  defines { "_DEBUG", "DEBUG" }

filter { "configurations:Release" }
  defines { "NDEBUG", "RELEASE" }

filter {}

warnings "Extra"
