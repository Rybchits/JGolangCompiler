package main

func main() {
	switch 1 {
	default:
		println("default")
		fallthrough

	case 1:
		println("111")
		fallthrough

	case 2:
		println("222")
		fallthrough

	case 3:
		println("333")
	}
}
