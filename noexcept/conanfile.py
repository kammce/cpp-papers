from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

required_conan_version = ">=2.0.14"


class noexcept_application(ConanFile):
    settings = "compiler", "build_type", "os", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualBuildEnv"

    def build_requirements(self):
        self.tool_requires("cmake/3.27.1")
        self.tool_requires("libhal-cmake-util/[^4.0.0]")
        self.tool_requires("arm-gnu-toolchain/12.3")

    def requirements(self):
        # self.requires("prebuilt-picolibc/12.2")
        # self.requires("tl-expected/20190710")
        pass

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
