package main

// printBoolean([3]int{1, 2, 3} == [3]int{1, 2, 3})
// printBoolean([3]int{1, 2, 3} == [3]int{1, 2, 4})

func main() {
	printBoolean(1 == 1)
	printBoolean(1 == 2)
	printBoolean(1.255 == 1.255)
	printBoolean(1.255 == 1.25)
	printBoolean(true == true)
	printBoolean(true == false)
	printBoolean("abc" == "abc")
	printBoolean("abs" == "abc")

	var a string = "abc"
	var b string = "abs"
	printBoolean(a == b)

	b = "abc"
	printBoolean(a == b)

	var c int = 1
	var d int = 1
	printBoolean(c == d)

	d = 2
	printBoolean(c == d)

	var e float32 = 1.255
	var f float32 = 1.255
	printBoolean(e == f)

	f = 2.2
	printBoolean(e == f)

}
