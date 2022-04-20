solution "lemontree"
  
  editorintegration "On"
  platforms { "x64" }

  configurations { "Debug", "Release" }

  dofile "liblt/project.lua"
  dofile "ltsend/project.lua"
  dofile "ltshow/project.lua"
  dofile "ltsrv/project.lua"
  dofile "ltrcv/project.lua"
  dofile "lttrace/project.lua"
  dofile "ltanalyze/project.lua"
  dofile "example/project.lua"
