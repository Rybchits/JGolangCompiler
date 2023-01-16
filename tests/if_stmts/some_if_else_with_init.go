package main //package name

func main() {
	b := 2             /* Not error and multiline comment */
	c := 3             // Comment
	if a := 2; a > b { // Comment
		if a > c { // Comment
			println("Biggest is a") // Comment
		} else if b > c {
			println("Biggest is b")
		}
	} else if a, b = 7, 8; b > c*2 || false { /* Not error
		and multiline comment */
		println("Biggest is b 2")
	} else {
		println("Biggest is c")
	}
}
