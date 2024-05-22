from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.env import VirtualBuildEnv

required_conan_version = ">=2.0.14"


class noexcept_application(ConanFile):
    settings = "compiler", "build_type", "os", "arch", "libc"

    def build_requirements(self):
        self.tool_requires("cmake/3.27.1")
        self.tool_requires("libhal-cmake-util/[^4.0.0]")

    def requirements(self):
        self.requires("prebuilt-picolibc/12.3")

    def generate(self):
        virt = VirtualBuildEnv(self)
        virt.generate()
        cmake = CMakeDeps(self)
        cmake.generate()
        tc = CMakeToolchain(self)
        tc.cache_variables["CONAN_LIBC"] = str(self.settings.libc)
        tc.generate()

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
