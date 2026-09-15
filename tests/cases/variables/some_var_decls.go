// Don't save with linter
package main

var (
	i int8
	k string
	l, a int64 = 10*100, 42
)

var i1 int
var (i2, i3 float32)
var (i4 float32 = 3.2; i5 float64)
var (i6 float32; i7 float64;)

func main() {
	i = 1
	k = "k"
	l *= 2
}
