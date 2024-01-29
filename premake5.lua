include "C:\\GameDev\\SmokLibraries\\Engine\\SmokGraphics"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "SmokMesh"
kind "StaticLib"
language "C++"

targetdir ("bin/" .. outputdir .. "/%{prj.name}")
objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

files 
{
    "includes/**.h",
    "src/**.c",
    "includes/**.hpp",
    "src/**.cpp",
}

includedirs
{
    "includes",

    "C:\\GameDev\\Libraries\\yaml-cpp\\include",
    "C:\\GameDev\\Libraries\\glm",
    
    "C:\\VulkanSDK\\1.3.239.0\\Include",
    "C:\\GameDev\\Libraries\\VulkanMemoryAllocator\\include",

    "C:\\GameDev\\BTDSTD/includes",
    "C:\\GameDev\\BTDSTD_C/includes",

    
    "C:\\GameDev\\SmokLibraries\\Engine\\SmokGraphics\\includes"
}

links
{
    "SmokGraphics"--,
   -- "SmokAsset"
}
                
defines
{
    "GLM_FORCE_RADIANS",
    "GLM_FORCE_DEPTH_ZERO_TO_ONE",
    "GLM_ENABLE_EXPERIMENTAL"
}
                
flags
{
    "NoRuntimeChecks",
    "MultiProcessorCompile"
}

--platforms
filter "system:windows"
cppdialect "C++17"
staticruntime "On"
systemversion "latest"

defines
{
    "Window_Build",
    "Desktop_Build"
}

--configs
filter "configurations:Debug"
defines "DEBUG"
symbols "On"

links
{

}

filter "configurations:Release"
defines "RELEASE"
optimize "On"

flags
{
   -- "LinkTimeOptimization"
}

filter "configurations:Dist"
defines "DIST"
optimize "On"

defines
{
    "NDEBUG"
}

flags
{
    -- "LinkTimeOptimization"
}

links
{
   
}