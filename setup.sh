# this setup-script was made for ubuntu 24.04 on a laptop using an integrated graphics chip
# for more details see the following tutorial: https://oneuptime.com/blog/post/2026-03-02-how-to-install-and-configure-vulkan-on-ubuntu/view


# this repo contains the vulkan header files. The 'vulkan-sdk' package provided by this repo depends on an outdated version of 'libyaml-cpp' (as of writing this 2026-03-30)
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc 
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-noble.list http://packages.lunarg.com/vulkan/lunarg-vulkan-noble.list

sudo apt-get install -y libglm-dev libglm-doc  vulkan-headers vulkan-tools vulkan-validationlayers libvulkan-dev libulkan1  spirv-tools  glslang-dev