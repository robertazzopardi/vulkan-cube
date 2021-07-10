# Compile shader.vert and shader.frag to .spv files
# In the shaders folder in root of project

cd shaders

# Acess vulkan sdk path and point to the glslc shader compiler
GLSLC=$VULKAN_SDK/macOS/bin/glslc

# Remove old compiled shaders, to make sure they are rebuilt
rm *.spv

# Compile the shaders to .spv files
GLSLC shader.vert -o vert.spv
GLSLC shader.frag -o frag.spv

cd ..

# sh compileShaders.sh
