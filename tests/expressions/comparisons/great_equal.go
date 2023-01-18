package main

func main() {
	println(0 >= 1)
	println(1 >= 1)
	println(2 >= 1)

	println(1.24 >= 1.255)
	println(1.255 >= 1.255)
	println(1.26 >= 1.255)

	println(false >= true)
	println(true >= true)
	println(true >= false)

	println("ab" >= "abc")
	println("abc" >= "abc")
	println("abs" >= "abc")

	var c int = 1
	var d int = 1
	println(c >= d)

	d = 2
	println(c >= d)

	var e float32 = 1.255
	var f float32 = 1.255
	println(e >= f)

	f = 2.0
	println(e >= f)
}
