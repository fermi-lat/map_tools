###### Library ######
add_library(
  map_tools STATIC
  src/DiffuseFunction.cxx
  src/Exposure.cxx
  src/MapParameters.cxx
  src/Parameters.cxx
  src/SkyImage.cxx
)
add_library(Fermitools::map_tools ALIAS map_tools)

target_include_directories(
  map_tools PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:>
)
target_link_libraries(map_tools PUBLIC healpix astro hoops tip st_app st_stream irfLoader)

###### Executables ######
add_executable(gtdispcube src/cube_display/cube_display.cxx)
add_executable(exposure_cube src/exposure_cube/exposure_cube.cxx)
target_link_libraries(gtdispcube PRIVATE map_tools)
target_link_libraries(exposure_cube PRIVATE map_tools)

###### Tests ######
add_executable(test_map_tools src/test/test_main.cxx)
target_link_libraries(test_map_tools PRIVATE map_tools)

###############################################################
# Installation
###############################################################
install(DIRECTORY map_tools DESTINATION ${FERMI_INSTALL_INCLUDEDIR})
install(DIRECTORY pfiles/ DESTINATION ${FERMI_INSTALL_PFILESDIR})

install(
  TARGETS map_tools gtdispcube exposure_cube test_map_tools
  EXPORT fermiTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
