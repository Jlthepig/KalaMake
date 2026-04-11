//our own logger
const logger = @import("logger.zig");
//static lib print
extern fn static_print(str: [*:0]const u8) void;
//shared lib print
extern fn shared_print(str: [*:0]const u8) void;

pub fn main() void
{
    logger.print("hello from logger!");
    static_print("hello from external logger!");
    shared_print("hello from external logger!");
}