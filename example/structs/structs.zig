export executable "structs";

use "std.zig";

pub fn main(argc : isize, argv : &&u8, env : &&u8) -> i32 {
    var foo : Foo;

    foo.a = foo.a + 1;

    foo.b = foo.a == 1;

    test_foo(foo);

    modify_foo(&foo);

    if foo.c != 100 {
        print_str("BAD\n");
    }

    test_point_to_self();

    test_byval_assign();

    test_initializer();

    print_str("OK\n");
    return 0;
}

struct Foo {
    a : i32,
    b : bool,
    c : f32,
}

struct Node {
    val: Val,
    next: &Node,
}

struct Val {
    x: i32,
}

fn test_foo(foo : Foo) {
    if !foo.b {
        print_str("BAD\n");
    }
}

fn modify_foo(foo : &Foo) {
    foo.c = 100;
}

fn test_point_to_self() {
    var root : Node;
    root.val.x = 1;

    var node : Node;
    node.next = &root;
    node.val.x = 2;

    root.next = &node;

    if node.next.next.next.val.x != 1 {
        print_str("BAD\n");
    }
}

fn test_byval_assign() {
    var foo1 : Foo;
    var foo2 : Foo;

    foo1.a = 1234;

    if foo2.a != 0 { print_str("BAD\n"); }

    foo2 = foo1;

    if foo2.a != 1234 { print_str("BAD - byval assignment failed\n"); }

}

fn test_initializer() {
    const val = Val { .x = 42 };
    if val.x != 42 { print_str("BAD\n"); }
}