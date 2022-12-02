// Don't save with linter
package main

const Pi float64 = 3.14159265358979323846
const zero = 0.0 // untyped floating-point constant
const (
	size int64 = 1024
	eof        = -1 // untyped integer constant
)
const a, b, c = 3, 4, "foo" // a = 3, b = 4, c = "foo", untyped integer and string constants
const u, v float32 = 0, 3   // u = 0.0, v = 3.0
const (i2 float32 = 42.0)
const (i3 float32 = 42.3; i4 = 2.1)