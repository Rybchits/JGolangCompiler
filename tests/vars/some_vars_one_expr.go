package variable

func sub_add(a int, b int) (int, int) {
	return a + b, a - b
}

var re, im = sub_add(8, 8)
