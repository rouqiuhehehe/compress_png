add_library(imagequant SHARED IMPORTED GLOBAL)

set_target_properties(imagequant PROPERTIES
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/imagequant.dll.a"
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/imagequant.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
)