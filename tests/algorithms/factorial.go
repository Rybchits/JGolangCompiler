package main

func factorial(n int) int {

	if n == 0 {
		return 1
	}
	return n * factorial(n-1)
}
func main() {

	printInt(factorial(4)) // 24
	printInt(factorial(5)) // 120
	printInt(factorial(6)) // 720
}
