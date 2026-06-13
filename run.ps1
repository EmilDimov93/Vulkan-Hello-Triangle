New-Item -ItemType Directory -Force -Path "build/Debug/shaders" | Out-Null

$glslang = "$env:VULKAN_SDK\Bin\glslangValidator.exe"

& $glslang -V "shader.vert" -o "build/Debug/shaders/vert.spv"
if ($LASTEXITCODE -ne 0) { exit 1 }

& $glslang -V "shader.frag" -o "build/Debug/shaders/frag.spv"
if ($LASTEXITCODE -ne 0) { exit 1 }

cmake -B build
cmake --build build

cd build/Debug

./HelloTriangle.exe

cd ../..