package main

func return1() int {
	return 1
}

func main() {
	m1 := [3][3]int{}
	for i := 0; i < 3; i++ {
		for j := 0; j < 3; j++ {
			m1[i][j] = i * j * return1()
		}
	}

	var a = 1
	a++
	println(a)
}
