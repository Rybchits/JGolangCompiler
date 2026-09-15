package main

func main() {
	printBoolean(0 < 1)
	printBoolean(1 < 1)
	printBoolean(2 < 1)

	printBoolean(1.24 < 1.255)
	printBoolean(1.255 < 1.255)
	printBoolean(1.26 < 1.255)

	printBoolean(false < true)
	printBoolean(true < true)
	printBoolean(true < false)

	printBoolean("ab" < "abc")
	printBoolean("abc" < "abc")
	printBoolean("abs" < "abc")

	var c int = 1
	var d int = 1
	printBoolean(c < d)

	d = 2
	printBoolean(c < d)

	var e float32 = 1.255
	var f float32 = 1.255
	printBoolean(e < f)

	f = 2.0
	printBoolean(e < f)
}
