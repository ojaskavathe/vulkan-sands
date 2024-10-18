{
  description = "Vulkan Sands";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          vulkan-headers
          vulkan-loader
          vulkan-validation-layers
          vulkan-tools # vulkaninfo
          shaderc # GLSL to SPIRV compiler - glslc
          vulkan-tools-lunarg # vkconfig
        ];

        # glfw is already provided as a submodule, but we need libX, libGL etc
        inputsFrom = with pkgs; [
          glfw
        ];

        packages = with pkgs; [
          cmake

          # lsp
          clang-tools
          clang
          cmake-language-server
        ];
      };

    };
}
