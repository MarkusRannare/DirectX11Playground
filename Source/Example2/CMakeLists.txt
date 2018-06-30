cmake_minimum_required (VERSION 3.11.4)
include( "../MoGET.cmake" )

BeginModule( Example2 EXECUTABLE )
DependsOn( Common )

target_link_libraries( Example2 d3d11 dxgi )
EndModule()
