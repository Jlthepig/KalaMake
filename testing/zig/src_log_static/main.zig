const std = @import("std");

export fn static_print(str: [*:0]const u8) void
{
    std.debug.print("[STATIC] {s}\n", .{str});
}