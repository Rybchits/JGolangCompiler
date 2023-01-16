package main

func main() {
	sum := 0
	i := 1
	for {
		if i == 5 {
			i++
			continue
		}

		sum = sum + i
		i++
		if i == 10 {
			break
		}
	}
	println(sum)
}
