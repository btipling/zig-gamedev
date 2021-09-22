const std = @import("std");
const Builder = std.build.Builder;
const Pkg = std.build.Pkg;

fn makeDxcCmd(
    comptime input_path: []const u8,
    comptime entry_point: []const u8,
    comptime output_filename: []const u8,
    comptime profile: []const u8,
    comptime define: []const u8,
) [9][]const u8 {
    // NOTE(mziulek): PIX reports warning about non-retail shader version. Why?
    const shader_ver = "6_6";
    const shader_dir = "content/shaders/";
    return [_][]const u8{
        "../../external/bin/dxc/dxc.exe",
        input_path,
        if (entry_point.len == 0) "" else "/E " ++ entry_point,
        "/Fo " ++ shader_dir ++ output_filename,
        "/T " ++ profile ++ "_" ++ shader_ver,
        if (define.len == 0) "" else "/D " ++ define,
        "/WX",
        "/Ges",
        "/O3",
    };
}

pub fn build(b: *std.build.Builder) void {
    const files = [_][]const u8{
        "D3D12Core.dll",
        "D3D12Core.pdb",
        "D3D12SDKLayers.dll",
        "D3D12SDKLayers.pdb",
    };
    std.fs.cwd().makePath("zig-out/bin/d3d12") catch unreachable;
    inline for (files) |file| {
        std.fs.Dir.copyFile(
            std.fs.cwd(),
            "../../external/bin/d3d12/" ++ file,
            std.fs.cwd(),
            "zig-out/bin/d3d12/" ++ file,
            .{},
        ) catch unreachable;
    }

    const dxc_step = b.step("dxc", "Build shaders");

    var dxc_command = makeDxcCmd("../../libs/common/imgui.hlsl", "vsMain", "imgui.vs.cso", "vs", "");
    dxc_step.dependOn(&b.addSystemCommand(&dxc_command).step);
    dxc_command = makeDxcCmd("../../libs/common/imgui.hlsl", "psMain", "imgui.ps.cso", "ps", "");
    dxc_step.dependOn(&b.addSystemCommand(&dxc_command).step);

    dxc_command = makeDxcCmd("../../libs/common/generate_mipmaps.hlsl", "main", "generate_mipmaps.cs.cso", "cs", "");
    dxc_step.dependOn(&b.addSystemCommand(&dxc_command).step);

    dxc_command = makeDxcCmd(
        "src/simple_raytracer.hlsl",
        "vsRastStaticMesh",
        "rast_static_mesh.vs.cso",
        "vs",
        "PSO__RAST_STATIC_MESH",
    );
    dxc_step.dependOn(&b.addSystemCommand(&dxc_command).step);
    dxc_command = makeDxcCmd(
        "src/simple_raytracer.hlsl",
        "psRastStaticMesh",
        "rast_static_mesh.ps.cso",
        "ps",
        "PSO__RAST_STATIC_MESH",
    );
    dxc_step.dependOn(&b.addSystemCommand(&dxc_command).step);

    dxc_command = makeDxcCmd(
        "src/simple_raytracer.hlsl",
        "vsZPrePass",
        "z_pre_pass.vs.cso",
        "vs",
        "PSO__Z_PRE_PASS",
    );
    dxc_step.dependOn(&b.addSystemCommand(&dxc_command).step);
    dxc_command = makeDxcCmd(
        "src/simple_raytracer.hlsl",
        "psZPrePass",
        "z_pre_pass.ps.cso",
        "ps",
        "PSO__Z_PRE_PASS",
    );
    dxc_step.dependOn(&b.addSystemCommand(&dxc_command).step);

    dxc_command = makeDxcCmd(
        "src/simple_raytracer.hlsl",
        "vsGenShadowRays",
        "gen_shadow_rays.vs.cso",
        "vs",
        "PSO__GEN_SHADOW_RAYS",
    );
    dxc_step.dependOn(&b.addSystemCommand(&dxc_command).step);
    dxc_command = makeDxcCmd(
        "src/simple_raytracer.hlsl",
        "psGenShadowRays",
        "gen_shadow_rays.ps.cso",
        "ps",
        "PSO__GEN_SHADOW_RAYS",
    );
    dxc_step.dependOn(&b.addSystemCommand(&dxc_command).step);

    dxc_command = makeDxcCmd(
        "src/simple_raytracer.hlsl",
        "",
        "trace_shadow_rays.lib.cso",
        "lib",
        "PSO__TRACE_SHADOW_RAYS",
    );
    dxc_step.dependOn(&b.addSystemCommand(&dxc_command).step);

    b.getInstallStep().dependOn(dxc_step);

    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("simple_raytracer", "src/simple_raytracer.zig");

    exe.setTarget(target);
    exe.setBuildMode(mode);

    const enable_pix = b.option(bool, "enable-pix", "Enable PIX GPU events and markers") orelse false;
    const tracy = b.option([]const u8, "tracy", "Enable Tracy integration. Supply path to Tracy source.");

    const exe_options = b.addOptions();
    exe.addOptions("build_options", exe_options);

    exe_options.addOption(bool, "enable_pix", enable_pix);
    exe_options.addOption(bool, "enable_tracy", tracy != null);
    if (tracy) |tracy_path| {
        const client_cpp = std.fs.path.join(
            b.allocator,
            &[_][]const u8{ tracy_path, "TracyClient.cpp" },
        ) catch unreachable;
        exe.addIncludeDir(tracy_path);
        exe.addCSourceFile(client_cpp, &[_][]const u8{
            "-DTRACY_ENABLE=1",
            "-fno-sanitize=undefined",
            "-D_WIN32_WINNT=0x601",
        });
        exe.linkSystemLibrary("ws2_32");
        exe.linkSystemLibrary("dbghelp");
    }

    // This is needed to export symbols from an .exe file.
    // We export D3D12SDKVersion and D3D12SDKPath symbols which
    // is required by DirectX 12 Agility SDK.
    exe.rdynamic = true;

    exe.want_lto = false;

    const pkg_win32 = Pkg{
        .name = "win32",
        .path = .{ .path = "../../libs/win32/win32.zig" },
    };
    exe.addPackage(pkg_win32);

    const pkg_common = Pkg{
        .name = "common",
        .path = .{ .path = "../../libs/common/common.zig" },
        .dependencies = &[_]Pkg{
            Pkg{
                .name = "win32",
                .path = .{ .path = "../../libs/win32/win32.zig" },
                .dependencies = null,
            },
            Pkg{
                .name = "build_options",
                .path = exe_options.getSource(),
                .dependencies = null,
            },
        },
    };
    exe.addPackage(pkg_common);

    const external = "../../external/src";
    exe.addIncludeDir(external);

    exe.linkSystemLibrary("c");
    exe.linkSystemLibrary("c++");
    exe.linkSystemLibrary("imm32");
    exe.addCSourceFile(external ++ "/cimgui/imgui/imgui.cpp", &[_][]const u8{""});
    exe.addCSourceFile(external ++ "/cimgui/imgui/imgui_widgets.cpp", &[_][]const u8{""});
    exe.addCSourceFile(external ++ "/cimgui/imgui/imgui_tables.cpp", &[_][]const u8{""});
    exe.addCSourceFile(external ++ "/cimgui/imgui/imgui_draw.cpp", &[_][]const u8{""});
    exe.addCSourceFile(external ++ "/cimgui/imgui/imgui_demo.cpp", &[_][]const u8{""});
    exe.addCSourceFile(external ++ "/cimgui/cimgui.cpp", &[_][]const u8{""});

    exe.addCSourceFile(external ++ "/cgltf.c", &[_][]const u8{"-std=c99"});

    exe.install();

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
