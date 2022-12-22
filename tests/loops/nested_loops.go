package main

func main() {
	for i, s := range []int{1, 2, 3} {
		println(i, s)

		if false {
			continue
		}

		for i := 1; i < 5; i++ {
			print(i)

			for i < 10 {
				print(s)
				continue
			}
			continue
		}
		continue
		print(s)
	}
}
