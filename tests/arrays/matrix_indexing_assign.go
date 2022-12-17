package main

func main() {
	m1 := []int{1, 2, 3, 4}
	var a = 2
	m1[0], a, m1[1+2] = 2, 3, 7
	print(a)
}
