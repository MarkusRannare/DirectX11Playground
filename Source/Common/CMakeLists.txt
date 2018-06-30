cmake_minimum_required (VERSION 3.11.4)
include( "../MoGET.cmake" )

BeginModule( Common LIBRARY )

# Link direct x libraries
target_link_libraries( Common d3d11 dxgi )

EndModule()