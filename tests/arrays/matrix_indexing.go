package main

func return1() int {
	return 1
}

func main() {
	m1 := [3][3]int{}
	for i := 0; i < 3; i++ {
		if false {
			continue
		}

		for j := 0; j < 3; j++ {
			m1[1.2][1] = i * j * return1()
			continue
		}

		continue
	}
}
