package main


func test(array [4]int, slice []float32, i int, f float32, s string) {
	array[0]++
	slice[0]++
	i++
	f++
	s = s + "!"

	println("From function ")
	println(array)
	println(slice)
	println(i)
	println(f)
	println(s)
}

func main() {

	var a = 1

	var b float32 = 2.541
	var s string = "Hello"

	var array = [4]int{1, 2, 3}
	var slice = []float32{1, 2, 3}

	test(array, slice, a, b, s)
	println("From main ")
	println(array)
	println(slice)
	println(a)
	println(b)
	println(s)
}
