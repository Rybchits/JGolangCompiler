package main

func main() {
	switch num := 6; num%2 == 0 {
	case true:
		println("even value")
		for i := 1; i < 5; i++ {
		    println(i)
		}
		fallthrough
	case false:
		println("odd value")
	}
}
