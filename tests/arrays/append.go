package main

func createBool() bool {
	i := 12
	if true {
		return true
	} else {
		return false
	}
	i = (12*i)/2 + 0
}

func main() {
	dwarfs := []string{"Церера"}

	dwarfs = append([]string{"a"}, "Оркус", "b")
	println(len(dwarfs))
}
