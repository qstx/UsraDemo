cmake_minimum_required(VERSION 3.20)
project(UrsaDemo VERSION 0.1.0 LANGUAGES CXX)

if(MSVC)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/EHsc> $<$<COMPILE_LANGUAGE:CXX>:/utf-8>)
endif()

set(URSA_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Engine" CACHE PATH "Path to the UrsaEngine repository")
if(NOT EXISTS "${URSA_ENGINE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "UrsaEngine was not found at '${URSA_ENGINE_DIR}'. Set URSA_ENGINE_DIR to the engine repository path.")
endif()

enable_testing()
add_subdirectory("${URSA_ENGINE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/Engine")
include_directories("${URSA_ENGINE_DIR}/Tests/Fixtures")

if(WIN32)
  add_custom_target(UrsaDemoFormatCheck
    COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass
      -File "${CMAKE_CURRENT_SOURCE_DIR}/Tools/Format.ps1" -Check
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM)
  add_custom_target(UrsaFormatCheck DEPENDS UrsaEngineFormatCheck UrsaDemoFormatCheck)
  set_target_properties(UrsaDemoFormatCheck UrsaFormatCheck PROPERTIES FOLDER "Tools")
endif()

add_executable(UrsaEngineHeaderSmoke Tests/Integration/EngineHeaderSmoke.cpp)
target_link_libraries(UrsaEngineHeaderSmoke PRIVATE Ursa::Engine)
add_test(NAME UrsaEngineHeaderSmoke COMMAND UrsaEngineHeaderSmoke)

file(GLOB_RECURSE URSA_SNAKE_LOGIC_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Snake/Game/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Snake/Game/*.hpp")
add_library(UrsaSnakeLogic STATIC ${URSA_SNAKE_LOGIC_SOURCES})
target_include_directories(UrsaSnakeLogic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Snake/Game)
target_link_libraries(UrsaSnakeLogic PUBLIC Ursa::Runtime)
target_compile_features(UrsaSnakeLogic PUBLIC cxx_std_17)

add_executable(UrsaSnakeTests Tests/Snake/Game/SnakeTests.cpp)
target_link_libraries(UrsaSnakeTests PRIVATE UrsaSnakeLogic)
add_test(NAME UrsaSnakeTests COMMAND UrsaSnakeTests)

file(GLOB_RECURSE URSA_SNAKE_SCENE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Snake/Scene/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Snake/Scene/*.hpp")
add_library(UrsaSnakeScene STATIC ${URSA_SNAKE_SCENE_SOURCES})
target_link_libraries(UrsaSnakeScene PUBLIC Ursa::Engine UrsaSnakeLogic)
target_include_directories(UrsaSnakeScene PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Snake/Scene)

add_executable(UrsaSnakeAssetBake Tools/BakeSnakeAssets.cpp)
target_link_libraries(UrsaSnakeAssetBake PRIVATE UrsaSnakeScene)

add_custom_target(UrsaDemoPackage
  COMMAND UrsaPak --engine "${URSA_ENGINE_DIR}/Assets"
    --engine-generated "${URSA_ASSET_MIRROR_DIR}" --project "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
    --output "${CMAKE_CURRENT_BINARY_DIR}/package/Content.pak"
  DEPENDS UrsaPak UrsaEngine VERBATIM)
if(TARGET UrsaEditorCore)
  add_dependencies(UrsaDemoPackage UrsaEditorCore)
endif()
add_test(NAME UrsaDemoPackageTests COMMAND UrsaPak
  --engine "${URSA_ENGINE_DIR}/Assets" --engine-generated "${URSA_ASSET_MIRROR_DIR}"
  --project "${CMAKE_CURRENT_SOURCE_DIR}/Assets" --output "${CMAKE_CURRENT_BINARY_DIR}/package/Content.pak")
set_tests_properties(UrsaDemoPackageTests PROPERTIES FIXTURES_SETUP UrsaDemoContentPak)
add_test(NAME UrsaSnakePakWindowSmoke COMMAND UrsaSnake
  --pak "${CMAKE_CURRENT_BINARY_DIR}/package/Content.pak" --hidden --frames 8)
set_tests_properties(UrsaSnakePakWindowSmoke PROPERTIES FIXTURES_REQUIRED UrsaDemoContentPak)

file(GLOB URSA_SNAKE_APP_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Snake/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Snake/*.hpp")
add_executable(UrsaSnake ${URSA_SNAKE_APP_SOURCES})
target_link_libraries(UrsaSnake PRIVATE UrsaSnakeScene)
set_target_properties(UrsaSnake PROPERTIES
  VS_DEBUGGER_COMMAND_ARGUMENTS ""
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME UrsaSnakeWindowSmoke COMMAND UrsaSnake --hidden --frames 20 --stress-window --smoke-gameplay)
add_test(NAME UrsaSnakeSerializedWindowSmoke COMMAND UrsaSnake --hidden --frames 20 --stress-window --smoke-gameplay --serialized)
add_test(NAME UrsaSnakeAssetWindowSmoke COMMAND UrsaSnake --hidden --frames 3)
set_tests_properties(UrsaSnakeWindowSmoke UrsaSnakeSerializedWindowSmoke UrsaSnakeAssetWindowSmoke PROPERTIES
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

if(TARGET Ursa::Editor)
  add_executable(UrsaDemoEditor Editor/Main.cpp Editor/SnakeEditors.cpp Editor/SnakeEditors.hpp)
  target_link_libraries(UrsaDemoEditor PRIVATE Ursa::Editor UrsaSnakeScene)
  set_target_properties(UrsaDemoEditor PROPERTIES
    VS_DEBUGGER_COMMAND_ARGUMENTS "--project \"${CMAKE_CURRENT_SOURCE_DIR}\" --scene Snake/Snake.ursascene"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  add_test(NAME UrsaDemoEditorWindowSmoke COMMAND UrsaDemoEditor --project "${CMAKE_CURRENT_SOURCE_DIR}" --scene Snake/Snake.ursascene --hidden --frames 3)
  add_test(NAME UrsaDemoEditorPakWindowSmoke COMMAND UrsaDemoEditor
    --project "${CMAKE_CURRENT_SOURCE_DIR}" --pak "${CMAKE_CURRENT_BINARY_DIR}/package/Content.pak"
    --scene Snake/Snake.ursascene --hidden --frames 8)
  set_tests_properties(UrsaDemoEditorPakWindowSmoke PROPERTIES FIXTURES_REQUIRED UrsaDemoContentPak)

  add_executable(UrsaSnakeEditorTests Tests/Snake/Scene/SnakeEditorTests.cpp Editor/SnakeEditors.cpp)
  target_link_libraries(UrsaSnakeEditorTests PRIVATE Ursa::Editor UrsaSnakeScene)
  target_include_directories(UrsaSnakeEditorTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Editor")
  add_test(NAME UrsaSnakeEditorTests COMMAND UrsaSnakeEditorTests)
endif()

file(GLOB URSA_MATERIAL_SHOWCASE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Material/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Material/*.hpp")
add_executable(UrsaMaterialShowcase ${URSA_MATERIAL_SHOWCASE_SOURCES})
target_link_libraries(UrsaMaterialShowcase PRIVATE Ursa::Engine)
add_test(NAME UrsaMaterialWindowSmoke COMMAND UrsaMaterialShowcase --hidden --frames 20)

add_executable(UrsaSnakeSceneTests Tests/Snake/Scene/SnakeSceneTests.cpp)
target_link_libraries(UrsaSnakeSceneTests PRIVATE UrsaSnakeScene)
add_test(NAME UrsaSnakeSceneTests COMMAND UrsaSnakeSceneTests)

add_executable(UrsaSnakeAssetTests Tests/Snake/Scene/SnakeAssetTests.cpp)
target_link_libraries(UrsaSnakeAssetTests PRIVATE UrsaSnakeScene)
add_test(NAME UrsaSnakeAssetTests COMMAND UrsaSnakeAssetTests "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(UrsaSnakeSerializationTests Tests/Snake/Scene/SnakeSerializationTests.cpp)
target_link_libraries(UrsaSnakeSerializationTests PRIVATE UrsaSnakeScene)
add_test(NAME UrsaSnakeSerializationTests COMMAND UrsaSnakeSerializationTests)

add_executable(UrsaSnakeRenderTests Tests/Snake/Scene/SnakeRenderTests.cpp)
target_link_libraries(UrsaSnakeRenderTests PRIVATE UrsaSnakeScene)
add_test(NAME UrsaSnakeRenderTests COMMAND UrsaSnakeRenderTests)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
  ${URSA_SNAKE_APP_SOURCES}
  ${URSA_SNAKE_LOGIC_SOURCES}
  ${URSA_SNAKE_SCENE_SOURCES}
  ${URSA_MATERIAL_SHOWCASE_SOURCES}
  Tools/BakeSnakeAssets.cpp
  Tests/Integration/EngineHeaderSmoke.cpp
  Tests/Snake/Game/SnakeTests.cpp
  Tests/Snake/Scene/SnakeSceneTests.cpp
  Tests/Snake/Scene/SnakeAssetTests.cpp
  Tests/Snake/Scene/SnakeSerializationTests.cpp
  Tests/Snake/Scene/SnakeRenderTests.cpp)

# Group Demo applications and their tests into solution folders.
get_property(URSA_DEMO_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
foreach(URSA_DEMO_TARGET IN LISTS URSA_DEMO_TARGETS)
  if(URSA_DEMO_TARGET MATCHES "Tests$" OR URSA_DEMO_TARGET MATCHES "Smoke$")
    set_target_properties(${URSA_DEMO_TARGET} PROPERTIES FOLDER "Tests/Demo")
  else()
    set_target_properties(${URSA_DEMO_TARGET} PROPERTIES FOLDER "Demo")
  endif()
endforeach()
