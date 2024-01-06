const std = @import("std");
const assert = std.debug.assert;

pub const ApiVersion = enum {
    sdl2,
    sdl3,
};

pub const Options = struct {
    api_version: ApiVersion = .sdl2,
    enable_ttf: bool = false,
};

pub const Package = struct {
    options: Options,
    zsdl: *std.Build.Module,
    zsdl_options: *std.Build.Module,
    install: *std.Build.Step,

    pub fn link(pkg: Package, exe: *std.Build.Step.Compile) void {
        exe.linkLibC();

        exe.addModule("zsdl", pkg.zsdl);

        exe.step.dependOn(pkg.install);

        const target = (std.zig.system.NativeTargetInfo.detect(exe.target) catch unreachable).target;

        switch (target.os.tag) {
            .windows => {
                assert(target.cpu.arch.isX86());

                exe.addLibraryPath(.{ .path = thisDir() ++ "/libs/x86_64-windows-gnu/lib" });

                switch (pkg.options.api_version) {
                    .sdl2 => {
                        exe.linkSystemLibraryName("SDL2");
                        exe.linkSystemLibraryName("SDL2main");
                        if (pkg.options.enable_ttf) {
                            exe.linkSystemLibraryName("SDL2_ttf");
                        }
                    },
                    .sdl3 => {
                        // TODO: link SDL3 (windows)
                    },
                }
            },
            .linux => {
                assert(target.cpu.arch.isX86());

                exe.addRPath(.{ .path = "$ORIGIN" });

                exe.addLibraryPath(.{ .path = thisDir() ++ "/libs/x86_64-linux-gnu/lib" });

                switch (pkg.options.api_version) {
                    .sdl2 => {
                        exe.linkSystemLibraryName("SDL2-2.0");
                        if (pkg.options.enable_ttf) {
                            exe.linkSystemLibraryName("SDL2_ttf-2.0");
                        }
                    },
                    .sdl3 => {
                        // TODO: link SDL3 (linux)
                    },
                }
            },
            .macos => {
                exe.addRPath(.{ .path = "@executable_path/Frameworks" });

                exe.addFrameworkPath(.{ .path = thisDir() ++ "/libs/macos/Frameworks" });

                switch (pkg.options.api_version) {
                    .sdl2 => {
                        exe.linkFramework("SDL2");
                        if (pkg.options.enable_ttf) {
                            exe.linkFramework("SDL2_ttf");
                        }
                    },
                    .sdl3 => {
                        // TODO: bundle SDL3.framework instead of this hack
                        // exe.linkFramework("SDL3");
                        exe.addLibraryPath(.{ .path = "/usr/local/lib" });
                        exe.linkSystemLibraryName("SDL3");

                        if (pkg.options.enable_ttf) {
                            exe.linkFramework("SDL2_ttf");
                        }
                    },
                }
            },
            else => unreachable,
        }
    }
};

pub fn package(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    _: std.builtin.Mode,
    args: struct {
        options: Options = .{},
    },
) Package {
    const options_step = b.addOptions();
    inline for (std.meta.fields(Options)) |option_field| {
        const option_val = @field(args.options, option_field.name);
        options_step.addOption(@TypeOf(option_val), option_field.name, option_val);
    }

    const options = options_step.createModule();

    const zsdl = b.createModule(.{
        .root_source_file = .{ .path = thisDir() ++ "/src/zsdl.zig" },
        .imports = &.{
            .{ .name = "zsdl_options", .module = options },
        },
    });

    const install_step = b.allocator.create(std.Build.Step) catch @panic("OOM");
    install_step.* = std.Build.Step.init(.{ .id = .custom, .name = "zsdl-install", .owner = b });

    if (target.isWindows()) {
        switch (args.options.api_version) {
            .sdl2 => {
                install_step.dependOn(
                    &b.addInstallFile(
                        .{ .path = thisDir() ++ "/libs/x86_64-windows-gnu/bin/SDL2.dll" },
                        "bin/SDL2.dll",
                    ).step,
                );
                if (args.options.enable_ttf) {
                    install_step.dependOn(
                        &b.addInstallFile(
                            .{ .path = thisDir() ++ "/libs/x86_64-windows-gnu/bin/SDL2_ttf.dll" },
                            "bin/SDL2_ttf.dll",
                        ).step,
                    );
                }
            },
            .sdl3 => {},
        }
    } else if (target.isLinux()) {
        switch (args.options.api_version) {
            .sdl2 => {
                install_step.dependOn(
                    &b.addInstallFile(
                        .{ .path = thisDir() ++ "/libs/x86_64-linux-gnu/lib/libSDL2-2.0.so" },
                        "bin/libSDL2-2.0.so.0",
                    ).step,
                );
                if (args.options.enable_ttf) {
                    install_step.dependOn(
                        &b.addInstallFile(
                            .{ .path = thisDir() ++ "/libs/x86_64-linux-gnu/lib/libSDL2_ttf-2.0.so" },
                            "bin/libSDL2_ttf-2.0.so.0",
                        ).step,
                    );
                }
            },
            .sdl3 => {},
        }
    } else if (target.isDarwin()) {
        switch (args.options.api_version) {
            .sdl2 => {
                install_step.dependOn(
                    &b.addInstallDirectory(.{
                        .source_dir = .{ .path = thisDir() ++ "/libs/macos/Frameworks/SDL2.framework" },
                        .install_dir = .{ .custom = "" },
                        .install_subdir = "bin/Frameworks/SDL2.framework",
                    }).step,
                );
                if (args.options.enable_ttf) {
                    install_step.dependOn(
                        &b.addInstallDirectory(.{
                            .source_dir = .{ .path = thisDir() ++ "/libs/macos/Frameworks/SDL2_ttf.framework" },
                            .install_dir = .{ .custom = "" },
                            .install_subdir = "bin/Frameworks/SDL2_ttf.framework",
                        }).step,
                    );
                }
            },
            .sdl3 => {},
        }
    } else unreachable;

    return .{
        .options = args.options,
        .zsdl = zsdl,
        .zsdl_options = options,
        .install = install_step,
    };
}

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{});
    const target = b.standardTargetOptions(.{});

    const test_step = b.step("test", "Run zsdl tests");
    test_step.dependOn(runTests(b, optimize, target, .sdl2));
    test_step.dependOn(runTests(b, optimize, target, .sdl3));
}

pub fn runTests(
    b: *std.Build,
    optimize: std.builtin.Mode,
    target: std.Build.ResolvedTarget,
    api_version: ApiVersion,
) *std.Build.Step {
    const tests = b.addTest(.{
        .name = "zsdl-tests",
        .root_source_file = .{ .path = thisDir() ++ "/src/zsdl.zig" },
        .target = target,
        .optimize = optimize,
    });
    const zsdl_pkg = package(b, target, optimize, .{
        .options = .{
            .api_version = api_version,
            .enable_ttf = true,
        },
    });
    tests.addModule("zsdl_options", zsdl_pkg.zsdl_options);
    zsdl_pkg.link(tests);
    return &b.addRunArtifact(tests).step;
}

inline fn thisDir() []const u8 {
    return comptime std.fs.path.dirname(@src().file) orelse ".";
}
