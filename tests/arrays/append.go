package main

func main() {
	dwarfs := []string{"Церера"}
	multi := [][]int{}

	for j := 0; j < 5; j++ {
		newarray := []int{}
		for i := 0; i < 5; i++ {
			newarray = append(newarray, i)
		}
		multi = append(multi, newarray)
	}

	dwarfs = append(dwarfs, "Оркус")
	println(dwarfs)
	println(multi)
}
