package main

func main() {
	println(1 == 1)
	println(1 == 2)
	println(1.255 == 1.255)
	println(1.255 == 1.25)
	println(true == true)
	println(true == false)
	println("abc" == "abc")
	println("abs" == "abc")

	var a string = "abc"
	var b string = "abs"
	println(a == b)

	b = "abc"
	println(a == b)

	var c int = 1
	var d int = 1
	println(c == d)

	d = 2
	println(c == d)

	var e float32 = 1.255
	var f float32 = 1.255
	println(e == f)

	f = 2.2
	println(e == f)

}
