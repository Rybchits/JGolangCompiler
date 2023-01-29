package main

func main() {
	switch 1 {
	default:
		println("default")

	case 1:
		println("111")
		break
		println("111")
		fallthrough

	case 2:
		println("222")

	case 3:
		println("333")
	}
}
