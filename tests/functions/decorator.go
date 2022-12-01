package functions

func decorator(inner func(int, int) int, x, y int) (int, []int) {
	return inner(x, y), []int{}
}
