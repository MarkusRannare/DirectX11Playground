# Setup the includes needed for a module to be able to find anything in it's own source directory
function( AddDefaultIncludes )
    include_directories("Public")
    include_directories("Private")
endfunction( AddDefaultIncludes )

function( BeginModule ModuleName ModuleType )
	# @todo: Verify that no other project exists already
	set( MOGET_CURRENT_MODULE ${ModuleName} CACHE STRING "Name of current module" FORCE )
	set( MOGET_CURRENT_CONTENT_PATH "${CMAKE_SOURCE_DIR}/../${ModuleName}" CACHE STRING "Path where we put content to current project" FORCE )
	set( MOGET_BINARIES_PATH "${CMAKE_SOURCE_DIR}../../Binaries/${MOGET_PLATFORM}/" CACHE STRING "The directory where we place our binaries" FORCE  )

	string( TOUPPER ${ModuleName} UpperModule )
	add_definitions(-D${UpperModule}_EXPORTS )
	AddDefaultIncludes()

	# @todo: Dump source list into a cache variable 
	GatherSourceFiles( ${CMAKE_CURRENT_SOURCE_DIR} SourceList )
	AppendShaderFiles( ${CMAKE_CURRENT_SOURCE_DIR} ShaderList )
	# Append the shaders to the source list
	set( SourceList ${SourceList} ${ShaderList} )
	SetupIDEFilters( "${SourceList}" )

	if( ${ModuleType} MATCHES "LIBRARY" )
		add_library(${ModuleName} SHARED ${SourceList})
	elseif( ${ModuleType} MATCHES "EXECUTABLE" )
		add_executable(${ModuleName} WIN32 ${SourceList})
	else()
		message( FATAL_ERROR "Could not find ModuleType '${ModuleType}', valid values are LIBRARY and EXECUTABLE")
	endif()
endfunction( BeginModule )

function( EndModule )
	SetOutputPaths()
	set_target_properties( ${MOGET_CURRENT_MODULE} PROPERTIES 
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../Binaries/${MOGET_PLATFORM}/")

	unset( MOGET_CURRENT_MODULE CACHE )
	# @todo: Verify that there exists a current project
endfunction( EndModule )

function( AppendShaderFiles SourceDirectory DestinationVariable )
	# @todo: Add sources from additional directories from cache variables
    file(
        GLOB_RECURSE HlslFiles
        LIST_DIRECTORIES false
        RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        "${SourceDirectory}/Shaders/*.hlsl"
    )
	set( ${DestinationVariable} ${HlslFiles} PARENT_SCOPE )
	# @todo: this will be removed when we get a in engine shader compilation mechanism
	foreach( HlslFile ${HlslFiles} )
		get_filename_component( FileName ${HlslFile} NAME_WE )
		STRING( FIND ${HlslFile} "Shaders/Pixel/" PixelIndex REVERSE )
		STRING( FIND ${HlslFile} "Shaders/Vertex/" VertexIndex REVERSE )

		# Some filetypes shouldn't be compiled
		set( OverrideCompiler "FXCompile" )

		if( PixelIndex STRGREATER -1 )
			set( ShaderType "Pixel" )
		elseif( VertexIndex STRGREATER -1 )
			set( ShaderType "Vertex" )
		else()
			set( OverrideCompiler "None" )
		endif()


		set_source_files_properties( ${HlslFile} PROPERTIES
			VS_TOOL_OVERRIDE "${OverrideCompiler}"
			VS_SHADER_MODEL "4.1"
			VS_SHADER_TYPE "${ShaderType}"
			VS_SHADER_OBJECT_FILE_NAME "${MOGET_CURRENT_CONTENT_PATH}/ShaderBinaries/${FileName}.cso" )
	endforeach()
endfunction( AppendShaderFiles )

# Will set the variable SourceList with all the source files to build
function( GatherSourceFiles SourceDirectory SourceList )
	# @todo: Add sources from additional directories from cache variables
    file(
        GLOB_RECURSE SourceFiles
        LIST_DIRECTORIES false
        RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        "${SourceDirectory}/Public/*.c*"
        "${SourceDirectory}/Private/*.c*"
        "${SourceDirectory}/Public/*.h*"
        "${SourceDirectory}/Private/*.h*"
    )
    set( ${SourceList} ${SourceFiles} PARENT_SCOPE )
endfunction( GatherSourceFiles )

function( SetupIDEFilters FileList )
	# Add files to filters according to how they are places in the folder structure
    foreach(Source IN ITEMS ${FileList})
        get_filename_component(_source_path "${Source}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${Source}")
    endforeach()
endfunction( SetupIDEFilters )

function( SetOutputPaths )		
	# These three should work, but I don't know what they do...
	#ARCHIVE_OUTPUT_DIRECTORY_${UpperConfig} ${LibrariesPath}
	#LIBRARY_OUTPUT_DIRECTORY_${UpperConfig} ${LibrariesPath}
	#VS_INTERMEDIATE_DIRECTORY_${UpperConfig} ${IntermediatePath}

	foreach (Config ${CMAKE_CONFIGURATION_TYPES})
		string (TOUPPER ${Config} UpperConfig)

		set_target_properties(${MOGET_CURRENT_MODULE} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_${UpperConfig} ${MOGET_BINARIES_PATH}
			PDB_OUTPUT_DIRECTORY_${UpperConfig} ${MOGET_BINARIES_PATH}
			OUTPUT_NAME_${UpperConfig} ${MOGET_CURRENT_MODULE}-${Config}
		)
    endforeach ()
endfunction( SetOutputPaths )

function( DependsOn ModuleName )
	# Make sure other project is compiled first
	add_dependencies( ${MOGET_CURRENT_MODULE} ${ModuleName} )
	# Link against other library, magically finds configuration type o.O
	target_link_libraries(${MOGET_CURRENT_MODULE} ${ModuleName} )
	# Let us include files from other projects Public directory
	include_directories("${CMAKE_SOURCE_DIR}/${ModuleName}/Public")
endfunction( DependsOn )