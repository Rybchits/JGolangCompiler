package main

func main() {
	switch 4 {
	case 1:
		println("a")
		fallthrough
	default:
		println("default")
		fallthrough
	case 2:
		println("b")
	case 3:
		println("c")
	}
	println("")
}
