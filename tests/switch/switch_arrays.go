package main

func main() {
	switch [2]int{1, 2} {
	default:
		println("default")

	case [2]int{2, 2}:
		println("111")

	case [2]int{1, 2}:
		println("222")

	case [2]int{3, 2}:
		println("333")
	}
}
