package main

func main() {
	a := 1  // Error
	b := 2  /* Not error
	 and multiline comment */
	c := 3
	if a := 2; a > b {
		if a > c {
			println("Biggest is a")
		} else if b > c {
			println("Biggest is b")
		}
	} else if a = 7; b > c {
		println("Biggest is b")
	} else {
		println("Biggest is c")
	}
}