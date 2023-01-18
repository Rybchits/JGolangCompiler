package main

func main() {
	switch 4 {
	default:
		println("default")

	case 1:
		fallthrough

	case 2:

	case 4:
		fallthrough

	case 3:
		println("333")
	}
}
