// Thin wrapper: compile the Dear ImGui SDL2 backend as part of the project.
// The actual source lives in the `imgui/` directory which is fetched separately
// (see README.md – "Fetch Dear ImGui" build step / CI workflow).
#include "../../imgui/backends/imgui_impl_sdl2.cpp"
