package main

func factorial(n int) int {

	if n == 0 {
		return 1
	}
	return n * factorial(n-1)
}
func main() {

	println(factorial(4)) // 24
	println(factorial(5)) // 120
	println(factorial(6)) // 720
}
