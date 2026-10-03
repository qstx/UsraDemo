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

add_executable(UrsaEngineHeaderSmoke tests/integration/EngineHeaderSmoke.cpp)
target_link_libraries(UrsaEngineHeaderSmoke PRIVATE Ursa::Engine)
add_test(NAME UrsaEngineHeaderSmoke COMMAND UrsaEngineHeaderSmoke)

file(GLOB_RECURSE URSA_SNAKE_LOGIC_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/snake/game/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/snake/game/*.hpp")
add_library(UrsaSnakeLogic STATIC ${URSA_SNAKE_LOGIC_SOURCES})
target_include_directories(UrsaSnakeLogic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/snake/game)
target_link_libraries(UrsaSnakeLogic PUBLIC Ursa::Runtime)
target_compile_features(UrsaSnakeLogic PUBLIC cxx_std_17)

add_executable(UrsaSnakeTests tests/snake/game/SnakeTests.cpp)
target_link_libraries(UrsaSnakeTests PRIVATE UrsaSnakeLogic)
add_test(NAME UrsaSnakeTests COMMAND UrsaSnakeTests)

file(GLOB_RECURSE URSA_SNAKE_SCENE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/snake/scene/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/snake/scene/*.hpp")
add_library(UrsaSnakeScene STATIC ${URSA_SNAKE_SCENE_SOURCES})
target_link_libraries(UrsaSnakeScene PUBLIC Ursa::Engine UrsaSnakeLogic)
target_include_directories(UrsaSnakeScene PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/snake/scene)

add_executable(UrsaSnakeAssetBake tools/BakeSnakeAssets.cpp)
target_link_libraries(UrsaSnakeAssetBake PRIVATE UrsaSnakeScene)

file(GLOB URSA_SNAKE_APP_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/snake/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/snake/*.hpp")
add_executable(UrsaSnake ${URSA_SNAKE_APP_SOURCES})
target_link_libraries(UrsaSnake PRIVATE UrsaSnakeScene)
add_test(NAME UrsaSnakeWindowSmoke COMMAND UrsaSnake --project "${CMAKE_CURRENT_SOURCE_DIR}" --hidden --frames 20 --stress-window --smoke-gameplay)
add_test(NAME UrsaSnakeSerializedWindowSmoke COMMAND UrsaSnake --project "${CMAKE_CURRENT_SOURCE_DIR}" --hidden --frames 20 --stress-window --smoke-gameplay --serialized)
add_test(NAME UrsaSnakeAssetWindowSmoke COMMAND UrsaSnake --project "${CMAKE_CURRENT_SOURCE_DIR}" --hidden --frames 3)

if(TARGET Ursa::Editor)
  add_executable(UrsaDemoEditor editor/Main.cpp editor/SnakeEditors.cpp editor/SnakeEditors.hpp)
  target_link_libraries(UrsaDemoEditor PRIVATE Ursa::Editor UrsaSnakeScene)
  add_test(NAME UrsaDemoEditorWindowSmoke COMMAND UrsaDemoEditor --project "${CMAKE_CURRENT_SOURCE_DIR}" --scene snake/snake.ursascene --hidden --frames 3)

  add_executable(UrsaSnakeEditorTests tests/snake/scene/SnakeEditorTests.cpp editor/SnakeEditors.cpp)
  target_link_libraries(UrsaSnakeEditorTests PRIVATE Ursa::Editor UrsaSnakeScene)
  target_include_directories(UrsaSnakeEditorTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/editor")
  add_test(NAME UrsaSnakeEditorTests COMMAND UrsaSnakeEditorTests)
endif()

file(GLOB URSA_MATERIAL_SHOWCASE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/material/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/material/*.hpp")
add_executable(UrsaMaterialShowcase ${URSA_MATERIAL_SHOWCASE_SOURCES})
target_link_libraries(UrsaMaterialShowcase PRIVATE Ursa::Engine)
add_test(NAME UrsaMaterialWindowSmoke COMMAND UrsaMaterialShowcase --hidden --frames 20)

add_executable(UrsaSnakeSceneTests tests/snake/scene/SnakeSceneTests.cpp)
target_link_libraries(UrsaSnakeSceneTests PRIVATE UrsaSnakeScene)
add_test(NAME UrsaSnakeSceneTests COMMAND UrsaSnakeSceneTests)

add_executable(UrsaSnakeAssetTests tests/snake/scene/SnakeAssetTests.cpp)
target_link_libraries(UrsaSnakeAssetTests PRIVATE UrsaSnakeScene)
add_test(NAME UrsaSnakeAssetTests COMMAND UrsaSnakeAssetTests "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(UrsaSnakeSerializationTests tests/snake/scene/SnakeSerializationTests.cpp)
target_link_libraries(UrsaSnakeSerializationTests PRIVATE UrsaSnakeScene)
add_test(NAME UrsaSnakeSerializationTests COMMAND UrsaSnakeSerializationTests)

add_executable(UrsaSnakeRenderTests tests/snake/scene/SnakeRenderTests.cpp)
target_link_libraries(UrsaSnakeRenderTests PRIVATE UrsaSnakeScene)
add_test(NAME UrsaSnakeRenderTests COMMAND UrsaSnakeRenderTests)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
  ${URSA_SNAKE_APP_SOURCES}
  ${URSA_SNAKE_LOGIC_SOURCES}
  ${URSA_SNAKE_SCENE_SOURCES}
  ${URSA_MATERIAL_SHOWCASE_SOURCES}
  tools/BakeSnakeAssets.cpp
  tests/integration/EngineHeaderSmoke.cpp
  tests/snake/game/SnakeTests.cpp
  tests/snake/scene/SnakeSceneTests.cpp
  tests/snake/scene/SnakeAssetTests.cpp
  tests/snake/scene/SnakeSerializationTests.cpp
  tests/snake/scene/SnakeRenderTests.cpp)

# Group Demo applications and their tests into solution folders.
get_property(URSA_DEMO_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
foreach(URSA_DEMO_TARGET IN LISTS URSA_DEMO_TARGETS)
  if(URSA_DEMO_TARGET MATCHES "Tests$" OR URSA_DEMO_TARGET MATCHES "Smoke$")
    set_target_properties(${URSA_DEMO_TARGET} PROPERTIES FOLDER "Tests/Demo")
  else()
    set_target_properties(${URSA_DEMO_TARGET} PROPERTIES FOLDER "Demo")
  endif()
endforeach()
