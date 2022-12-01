package functions

func sum(a int, b int, c int) int {
	return a + b + c
}

func onlyTypes(int, float32, []string, []int, int) (a int, b int) {
	return 1, 2
}

func divby10(num int) (res int) {
	res = num / 10
	return res
}
