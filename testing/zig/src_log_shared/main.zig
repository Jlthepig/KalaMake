const std = @import("std");

export fn shared_print(str: [*:0]const u8) void
{
    std.debug.print("[SHARED] {s}\n", .{str});
}