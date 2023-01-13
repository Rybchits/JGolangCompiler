package main

func main() {
	printBoolean(0 > 1)
	printBoolean(1 > 1)
	printBoolean(2 > 1)

	printBoolean(1.24 > 1.255)
	printBoolean(1.255 > 1.255)
	printBoolean(1.26 > 1.255)

	printBoolean(false > true)
	printBoolean(true > true) //TODO семантическая проверка. Нельзя сравнивать булини
	printBoolean(true > false)

	printBoolean("ab" > "abc")
	printBoolean("abc" > "abc")
	printBoolean("abs" > "abc")

	var a int = 1
	var b int = 1
	printBoolean(a > b)

	a = 2
	printBoolean(a > b)

	var c float32 = 1.55
	var d float32 = 1.55
	printBoolean(c > d)

	c = 2.0
	printBoolean(c > d)
}
