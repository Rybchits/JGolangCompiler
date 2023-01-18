package main

func main() {
	switch -1 + 3*3*3/9 + 1 {
	default:
		println("default")

	case 1000 - 999:
		println("111")

	case 2 * 2 / 2:
		println("222")

	case 3000*3 - 9000/1 + 3:
		println("333")
	}
}
