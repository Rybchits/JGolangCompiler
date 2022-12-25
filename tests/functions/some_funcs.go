package functions

func sum(a int, b int, c int) int {
	return a + b + c
}

func onlyTypes(int, float32, []string, []int, int) int {
	return 1
}

func divby10(num int) int {
	res := num / 10
	return res
}

func main() {

	print(sum(1, 2, 3))
}
